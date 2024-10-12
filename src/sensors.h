#ifndef Sensors_H
#define Sensors_H
#include <Arduino.h>
#include <string.h>
#include <structs.h>
#include <EEPROM.h>
#include <hX711.h>
#include <string.h>

// #define FILE_BASE_NAME "TestStandLog_"
// const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN       34
#define LOADCELL_SCK_PIN        33

// Pressure Transducer Wiring
#define PRESSURE_TRANSDUCER_PIN 24

#define MODE_O_PIN 00
#define MODE_Ix_PIN 00


float scale_factor = 1.0;

HX711 scale;

class Sensors{
    public:
        String fileName;

        Sensors(){
        }

        uint8_t init() {
            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
            // EEPROM.get(0, scale_factor);
            // if (scale_factor != scale_factor) {
            //     scale_factor = 1000;
            // }
            // Serial.print("Loaded scale factor: \t");
            // Serial.println(scale_factor);
            // scale.set_scale(scale_factor);
            scale.set_scale(1);
            scale.tare(10);
        }

        float readForce() {
            return scale.get_units(1);
        }

        float readPressure(){
            int raw = analogRead(PRESSURE_TRANSDUCER_PIN);

            // out put from 0.4-4.5v 0psi-1600psi
            return (raw+5 - (1024.0*0.1)) * (1600 * (4.0/1024));
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