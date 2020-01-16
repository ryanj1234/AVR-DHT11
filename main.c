#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include <stdio.h>

#define DHT11_DDR   DDRD
#define DHT11_PORT  PORTD
#define DHT11_PIN   PIND

#define DHT11_START_SIGNAL_TIME     20 // data sheet says at least 18ms
#define DHT11_START_WAIT_TIME       20 // data sheet says at least 20-40ms
#define DHT11_TRANSMISSION_SIZE 5 // bytes

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

#define DHT11_READ_0(bitMask) ((~DHT11_PIN) & bitMask)
#define DHT11_READ_1(bitMask) ((DHT11_PIN) & bitMask)

typedef struct {
    uint8_t bus;
} dht11_device;

dht11_device dht11_get_device(uint8_t pin)
{
    dht11_device dev = {_BV(pin)};
    return dev;
}

void dht11_signal_start(dht11_device dev)
{
    DHT11_PULL_BUS_LOW(dev.bus);
    _delay_ms(DHT11_START_SIGNAL_TIME);
    DHT11_RELEASE_BUS(dev.bus);

    // wait for sensor to begin transmission
    while(DHT11_READ_0(dev.bus)) {}
}

int main(void)
{
    uart_init(9600);

    dht11_device dht11 = dht11_get_device(PIND4);
    uint8_t data[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    // uint8_t counts[40];

    printf("Detecting device...\n");
    dht11_signal_start(dht11);
    while(DHT11_READ_1(dht11.bus)) {}
    while(DHT11_READ_0(dht11.bus)) {}
    while(DHT11_READ_1(dht11.bus)) {}
    // printf("Device detected!\n");
    for(uint8_t i = 0; i < DHT11_TRANSMISSION_SIZE; i++)
    {
        for(uint8_t j = 0; j < 8; j++)
        {
            uint8_t count = 0;
            // wait for bit
            while(DHT11_READ_0(dht11.bus)) {}
            while(DHT11_READ_1(dht11.bus))
            {
                count++;
                _delay_us(1);
            }
            if(count > 30)
            {
                data[i] |= _BV(8 - j);
            }
            // counts[i*8 + j] = count;
            // printf("%d\n", i*8 + j);
        }
    }
    
    for(uint8_t i = 0; i < DHT11_TRANSMISSION_SIZE; i++)
    {
        printf("%i: 0x%02X\n", i, data[i]);
    }

    // for(uint8_t i = 0; i < 40; i++)
    // {
    //     printf("%i: %d\n", i, counts[i]);
    // }

    uint8_t chk_sum = data[0] + data[1] + data[2] + data[3];
    if(chk_sum == data[4])
    {
        printf("Checksum matches!\n");
    }
    else
    {
        printf("No match...\n");
    }
    
    while(1) 
    {
        
    }
}