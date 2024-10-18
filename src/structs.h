#ifndef structs_H
#define structs_H

#include <Arduino.h>

typedef struct {
  u_int16_t magic;          // 2 bytes -   2
  u_int32_t time_ms;        // 4 bytes -   6
  float force;              // 4 bytes -  10
  u_int8_t status;          // 1 byte  -  11
  u_int32_t checksum;       // 4 bytes -  1
} __attribute__((packed)) LoadCellPacket;

typedef struct {
  u_int16_t magic;          // 2 bytes -   2
  u_int8_t status;          // 1 byte  -   3
  u_int8_t idx;             // 1 byte  -   4
  u_int32_t time_ms;        // 4 bytes -   8
  float voltage;            // 4 bytes -  12
  float pressure;           // 4 bytes -  16
  u_int32_t checksum;       // 4 bytes -  20
} __attribute__((packed)) PressurePacket;

#endif