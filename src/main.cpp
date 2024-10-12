#include <Arduino.h>
#include <EEPROM.h>
#include <FastCRC.h>


#include <structs.h>
#include <sensors.h>

data_packet packet;

Sensors sen;

FastCRC32 CRC32;

elapsedMicros printTimer;
elapsedMillis sdSaveTimer;

short magic = 0xA123;


void setup() {
  packet.magic = magic;
  Serial.begin(115200);
  Serial.println("Initializing sensors");
  sen.init(packet);
  // sen.beginSD(packet);
}

void loop() {

  if (Serial.available()) {
    sen.executeCommand();
  }

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

  Serial.printf("L: %f, P: %f \n", sen.readForce(), ((float)analogRead(24)));

}