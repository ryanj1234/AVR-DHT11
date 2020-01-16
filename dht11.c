#include <avr/io.h>
#include <util/delay.h>
#include "dht11.h"

#define DHT11_DDR   DDRD
#define DHT11_PORT  PORTD
#define DHT11_PIN   PIND

#define DHT11_START_SIGNAL_TIME     20 // data sheet says at least 18ms
#define DHT11_TIMEOUT_COUNTS        25000 // TODO: make this depend on F_CPU
#define DHT11_TRANSMISSION_SIZE     5 // bytes
#define DHT11_ZERO_PULSE_THRESH     30 // micro seconds (roughly)

#define DHT11_PULL_BUS_LOW(bitMask) \
            DHT11_DDR |= bitMask; \
            DHT11_PORT &= ~bitMask;

#ifdef DHT11_USE_INTERNAL_PULLUP            
#define DHT11_RELEASE_BUS(bitMask) \
            DHT11_DDR &= ~bitMask; \
            DHT11_PORT |= bitMask; 
#else
#define DHT11_RELEASE_BUS(bitMask) \
            DHT11_DDR &= ~bitMask; \
            DHT11_PORT &= ~bitMask;
#endif

#define DHT11_BIT_IS_0(bitMask) ((~DHT11_PIN) & bitMask)
#define DHT11_BIT_IS_1(bitMask) ((DHT11_PIN) & bitMask)

dht11_device dht11_get_device(uint8_t pin)
{
    dht11_device dev = {_BV(pin)};
    return dev;
}

static uint8_t dht11_signal_start(dht11_device dev)
{
    DHT11_PULL_BUS_LOW(dev.bus);
    _delay_ms(DHT11_START_SIGNAL_TIME);
    DHT11_RELEASE_BUS(dev.bus);
    _delay_us(1);

    // wait for sensor to begin transmission
    uint16_t cnt = 0;
    while(DHT11_BIT_IS_1(dev.bus))
    {
        if(cnt++ > DHT11_TIMEOUT_COUNTS) return DHT11_READ_TIMEOUT;
    }

    return DHT11_READ_SUCCESS;
}

static inline uint8_t dht11_verify_chk_sum(uint8_t *data)
{
    return data[4] == data[0] + data[1] + data[2] + data[3];
}

uint8_t dht11_read_data(dht11_device dev, uint16_t *rel_hum, uint16_t *temp)
{
    if(dht11_signal_start(dev) == DHT11_READ_TIMEOUT)
    {
        return DHT11_READ_TIMEOUT;
    }

    uint16_t cnt = 0;
    // pin goes low for ~80us
    while(DHT11_BIT_IS_0(dev.bus))
    {
        if(cnt == DHT11_TIMEOUT_COUNTS) { return DHT11_READ_TIMEOUT; }
        cnt++;
    }

    cnt = 0;
    // pin goes high for ~80us
    while(DHT11_BIT_IS_1(dev.bus))
    {
        if(cnt == DHT11_TIMEOUT_COUNTS) { return DHT11_READ_TIMEOUT; }
        cnt++;
    }

    uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};   
    for(uint8_t i = 0; i < DHT11_TRANSMISSION_SIZE; i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            // wait for bit
            while(DHT11_BIT_IS_0(dev.bus))
            {
                if(cnt == DHT11_TIMEOUT_COUNTS) { return DHT11_READ_TIMEOUT; }
                cnt++;
            }

            cnt = 0;
            while(DHT11_BIT_IS_1(dev.bus))
            {
                cnt++;
                _delay_us(1);
            }
            if(cnt > DHT11_ZERO_PULSE_THRESH)
            {
                data[i] |= _BV(7 - j);
            }
        }
    }

    if(dht11_verify_chk_sum(data))
    {
        *rel_hum = (data[0]<<8) | (data[1]);
        *temp = (data[2]<<8) | (data[3]);
        return DHT11_READ_SUCCESS;
    }
    else
    {
        return DHT11_READ_ERROR;
    }
}