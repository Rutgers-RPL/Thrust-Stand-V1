#ifndef structs_H
#define structs_H

#include <Arduino.h>

typedef struct {
  short magic;                   // 2 bytes -   2
  float time_s;         // 4 bytes -  6
  float force;          // 4 bytes -  10
  float pressure;          // 4 bytes -  14
  unsigned int checksum;         // 4 bytes - 18
} __attribute__((packed)) data_packet;

#endif