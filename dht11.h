#ifndef _DHT11_H_
#define _DHT11_H_

#include <avr/io.h>

#define DHT11_USE_INTERNAL_PULLUP

#define DHT11_READ_SUCCESS  0
#define DHT11_READ_TIMEOUT  1
#define DHT11_READ_ERROR    2

typedef struct {
    uint8_t bus;
} dht11_device;

dht11_device dht11_get_device(uint8_t pin);
uint8_t dht11_read_data(dht11_device dev, uint16_t *rel_hum, uint16_t *temp);

#endif