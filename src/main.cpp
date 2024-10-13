#include <Arduino.h>
#include <EEPROM.h>
#include <FastCRC.h>
#include "structs.h"
#include "sensors.h"
#include "logging.h"

#define THRUST_MAGIC 0xA123
#define PRESSURE_MAGIC 0xB987

#define CAL_SIGNAL_PIN 00 // TODO: set this...

#define N_TRANSDUCERS 3

#define PRINT_FLAG true

LoadCellPacket LCPacket;
PressurePacket PPackets[3];
u_int8_t pressurePins[] = {24, 24, 24}; //TODO: Change these bases on wiring

Sensors sen;

FastCRC32 CRC32;

elapsedMillis sendTimer;

Logging logging;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  
  u_int8_t logging_status = logging.init();

  if (digitalRead(CAL_SIGNAL_PIN) == HIGH)
  {
    Serial.println("Beginning Calibration Sequence...");
    sen.calibrate(logging);
    while (1); // you shall not pass
    
  }
  else
  {
    Serial1.begin(115200);
    u_int8_t sen_status = sen.init();
    LCPacket.magic = THRUST_MAGIC; 
    LCPacket.status = sen_status | logging_status;
    for(u_int8_t i = N_TRANSDUCERS; i; --i)
    { 
      PPackets[i].magic = PRESSURE_MAGIC;
      PPackets[i].status = sen_status | logging_status;
      PPackets[i].idx = i;
    }
  }
  
}

void loop() {
  u_int32_t curr_time = millis();

  if (sen.scale.is_ready())
  {
    LCPacket.force = sen.readForce();
    LCPacket.time_ms = curr_time;
    LCPacket.checksum = CRC32.crc32((const uint8_t *)&LCPacket+sizeof(u_int16_t), sizeof(LoadCellPacket) - sizeof(u_int16_t) - sizeof(u_int32_t));
    logging.logBinaryPacket(&LCPacket);
  }

  for(u_int8_t i = N_TRANSDUCERS; i; --i)
  {
    PPackets[i].voltage = Sensors::readVoltage(pressurePins[i]);
    PPackets[i].pressure = Sensors::voltageToPressure(PPackets[i].voltage);
    PPackets[i].time_ms = curr_time;
    PPackets[i].checksum = CRC32.crc32((const uint8_t *)&PPackets[i]+sizeof(u_int16_t), sizeof(PressurePacket) - sizeof(u_int16_t) - sizeof(u_int32_t));
    logging.logBinaryPacket(&PPackets[i]);
  }

  if (sendTimer > 10)
  {
    Serial1.write((const uint8_t *)&LCPacket, sizeof(LoadCellPacket));
    for(u_int8_t i = N_TRANSDUCERS; i; --i) Serial1.write((const uint8_t *)&PPackets[i], sizeof(LoadCellPacket));

    #if PRINT_FLAG
      logging.printLoadCellPacket(LCPacket);
      for(u_int8_t i = N_TRANSDUCERS; i; --i) logging.printPressurePacket(PPackets[i]);
    #endif

  }

}