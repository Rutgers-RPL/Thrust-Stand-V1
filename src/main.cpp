#include <Arduino.h>
#include <EEPROM.h>
#include <FastCRC.h>
#include "structs.h"
#include "sensors.h"
#include "logging.h"

#define MAGIC 0xA123
#define CAL_SIGNAL_PIN 00

LoadCellPacket LCPacket;
PressurePacket PPackets[3];

Sensors sen;

FastCRC32 CRC32;

elapsedMicros printTimer;
elapsedMillis sdSaveTimer;

Logging logging;

void setup() {
  LCPacket.magic = MAGIC;
  Serial.begin(115200);
  
  logging.init();

  if (digitalRead(CAL_SIGNAL_PIN) == HIGH)
  {
    Serial.println("Beginning Calibration Sequence...");
    sen.calibrate(logging);
  }
  
}

void loop() {



  // if (printTimer > (1000000 / 80)) {
  //   packet.force = sen.readForce();
  //   packet.pressure = sen.readPressure();
  //   packet.time_s = millis() / 1000.0;
  //   packet.checksum = CRC32.crc32((const uint8_t *)&packet+sizeof(short), sizeof(packet) - 6);
  //   Serial.write((const uint8_t *)&packet, sizeof(data_packet));
  //   sen.logPacket(packet);
  //   printTimer = 0;
  // }

  // if (sdSaveTimer > 250) {
  //   sen.f.flush();
  //   sdSaveTimer = 0;
  // }


}