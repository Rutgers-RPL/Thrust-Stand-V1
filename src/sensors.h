#ifndef Sensors_H
#define Sensors_H
#include <Arduino.h>
#include <SdFat.h>
#include <string.h>
#include <structs.h>
#include <EEPROM.h>
#include <hX711.h>

#define FILE_BASE_NAME "TestStandLog_"
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 34;
const int LOADCELL_SCK_PIN = 33;

// Pressure Transducer Wiring
const int PRESSURE_TRANSDUCER_PIN = 38;

float scale_factor = 1.0;

HX711 scale;

class Sensors{
    public:

        SdFs sd;
        FsFile f;
        bool sdexists = false;

        char* fileName = FILE_BASE_NAME "0000.bin";

        Sensors(){
        }

        void init(data_packet packet) {
            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
            EEPROM.get(0, scale_factor);
            if (scale_factor != scale_factor) {
                scale_factor = 1000;
            }
            Serial.print("Loaded scale factor: \t");
            Serial.println(scale_factor);
            scale.set_scale(scale_factor);
            scale.tare();
        }

        float readForce() {
            return scale.get_units(1);
        }

        float readPressure(){
            int raw = analogRead(PRESSURE_TRANSDUCER_PIN);

            // out put from 0.4-4.5v 0psi-1600psi
            return (raw+5 - (1024.0*0.1)) * (1600 * (4.0/1024));
        }

        void beginSD(data_packet packet) {
            delay(1000);
            byte attempts = 1;
            while (attempts <= 10) {
                if (!sd.begin(SdioConfig(FIFO_SDIO))) {
                    Serial.print("SD Begin Failed, Attempting "); Serial.print(10 - attempts++); Serial.println(" more tries ...");
                    delay(1000);
                } else {
                    Serial.println("\nFIFO SDIO mode.");

                    // Enumerates File Name
                    while (sd.exists(fileName)) {
                        if (fileName[BASE_NAME_SIZE + 3] != '9') {
                            fileName[BASE_NAME_SIZE + 3]++;
                        } else if (fileName[BASE_NAME_SIZE + 2] != '9') {
                            fileName[BASE_NAME_SIZE + 3] = '0';
                            fileName[BASE_NAME_SIZE + 2]++;
                        } else if (fileName[BASE_NAME_SIZE + 1] != '9') {
                            fileName[BASE_NAME_SIZE + 2] = '0';
                            fileName[BASE_NAME_SIZE + 3] = '0';
                            fileName[BASE_NAME_SIZE + 1]++;
                        } else if (fileName[BASE_NAME_SIZE] != '9') {
                            fileName[BASE_NAME_SIZE + 1] = '0';
                            fileName[BASE_NAME_SIZE + 2] = '0';
                            fileName[BASE_NAME_SIZE + 3] = '0';
                            fileName[BASE_NAME_SIZE]++;
                        } else {
                            Serial.println("Can't create file name");
                        }
                    }

                    f = sd.open(fileName, FILE_WRITE);
                    Serial.print("Writing to: ");
                    Serial.println(fileName);
                    if (!f) {
                        Serial.println("Failed opening file.");
                        break;
                    }
                    sdexists = true;
                    break;
                }
            }
        }

        void logBinaryPacket(const data_packet packet) {
            f.write((const uint8_t *)&packet, sizeof(data_packet));
        }

        void logPacket(const data_packet packet) {
            f.print(packet.magic); f.print(","); 
            f.print(packet.time_s); f.print(",");
            f.print(packet.force); f.print(",");
            f.print(packet.pressure); f.print(",");
            f.print(packet.checksum); f.print(",");
            f.println();
        }

        void printPacket(const data_packet packet) {
            Serial.print(packet.magic); Serial.print("\t"); 
            Serial.print(packet.time_s); Serial.print("\t"); 
            Serial.print(packet.force); Serial.print("\t");
            Serial.print(packet.pressure); Serial.print("\t");
            Serial.print(packet.checksum); Serial.print("\t");
            Serial.println();
        }

        void executeCommand() {
            while (1) {
                String command = Serial.readString().trim();
                Serial.println(command);

                if (command.equals("cal")) {
                    Serial.println("Remove items from scale.");
                    Serial.println("Enter anything to continue:");
                    while (!Serial.available());
                    Serial.readString();

                    scale.set_scale();
                    scale.tare();

                    float weight = 0.0f;
                    Serial.println("Place item on scale and enter weight: \t\t");
                    while (1) {
                        if (Serial.available()) {
                            weight = Serial.parseFloat();
                            if (weight > 0.1f)
                                break;
                        }
                    }

                    scale_factor = scale.get_value(80) / weight;
                    Serial.print("Callibrated scale factor: \t");
                    Serial.println(scale_factor);
                    scale.set_scale(scale_factor);
                    Serial.println("Successfully updated scale factor!");
                    break;

                } else if (command.equals("tare")) {
                    Serial.println("Tared scale!");
                    scale.tare();
                    break;

                } else if (command.equals("save")) {
                    Serial.print("Saved scale factor: \t");
                    Serial.println(scale_factor);
                    EEPROM.put(0, scale_factor);
                    break;
                } else if (command.equals("reset")) {
                    Serial.print("Reset memory and scale factor! \t");
                    EEPROM.put(0, 0.f);
                    scale.set_scale();
                    break;

                }
            }
        }
};

#endif