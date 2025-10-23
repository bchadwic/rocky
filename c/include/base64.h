#ifndef BASE64_H
#define BASE64_H

#include <stdint.h>

char *encode(uint8_t x);
uint8_t *decode(char *buf);

#endif