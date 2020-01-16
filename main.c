#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include <stdio.h>
#include "dht11.h"

#define DHT11_MEASUREMENT_PERIOD    5 // seconds

int main(void)
{
    uart_init(9600);

    dht11_device dht11 = dht11_get_device(PIND5);
    
    printf("Beginning program\n");
    uint16_t rel_hum = 0, temp = 0;
    while(1) 
    {
        if(dht11_read_data(dht11, &rel_hum, &temp) == DHT11_READ_SUCCESS)
        {
            uint8_t temp_i = (temp >> 8);
            uint8_t temp_d = (uint8_t)temp & 0xFF;
            float deg_f = ((float)temp_i + ((float)temp_d)/10)*(9.0/5.0) + 32.0;
            printf("Humidity: %d%%\r", rel_hum >> 8);
            printf("T: %.1f (deg F)\r\r", deg_f);
        }
        else
        {
            printf("Error reading data\n");
        }

        for(uint8_t i = 0; i < DHT11_MEASUREMENT_PERIOD; i++)
        {
            _delay_ms(1000);
        }
    }
}