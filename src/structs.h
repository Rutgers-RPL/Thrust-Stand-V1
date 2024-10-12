#ifndef structs_H
#define structs_H

#include <Arduino.h>

typedef struct {
  u_int16_t magic;          // 2 bytes -   2
  float time_s;             // 4 bytes -   6
  float force;              // 4 bytes -  10
  u_int8_t status;          // 1 byte  -  11
  u_int32_t checksum;       // 4 bytes -  1
} __attribute__((packed)) LoadCellPacket;

typedef struct {
  u_int16_t magic;          // 2 bytes -   2
  float time_s;             // 4 bytes -   6
  float voltage;            // 4 bytes -  10
  float force;              // 4 bytes -  14
  u_int8_t status;          // 1 byte  -  15
  u_int32_t checksum;       // 4 bytes -  19
} __attribute__((packed)) PressurePacket;


#endif