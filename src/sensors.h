#ifndef Sensors_H
#define Sensors_H
#include <Arduino.h>
#include <string.h>
#include <structs.h>
#include <EEPROM.h>
#include "HX711.h"
#include <string.h>
#include "logging.h"
#include "reg.h"

// #define FILE_BASE_NAME "TestStandLog_"
// const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN       34
#define LOADCELL_SCK_PIN        33


class Sensors{
    public:

        float load_scale_factor;
        float load_scale_offset;

        HX711 scale;

        Sensors(){
            this->load_scale_factor = 0.f;
            this->load_scale_offset = 0.f;
        }

        uint8_t init() {
            u_int8_t status = 0;
            // load initialization from EEPROM and begin
            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
            Serial.println("Attempting to load scale factor from EEPROM.");
            u_int8_t lcf_bytes[4] = {EEPROM.read(0), EEPROM.read(1), EEPROM.read(2), EEPROM.read(3)};
            // for(u_int8_t i = 0; i < 4; i++) 
            //     Serial.printf("%x ", lcf_bytes[i]);
            this->load_scale_factor = *(float*) &lcf_bytes;

            u_int8_t lco_bytes[4] = {EEPROM.read(4), EEPROM.read(5), EEPROM.read(6), EEPROM.read(7)};
            // for(u_int8_t i = 0; i < 4; i++) 
            //     Serial.printf("%x ", lcf_bytes[i]);
            this->load_scale_offset = *(float*) &lco_bytes;
            // if (! this->load_scale_factor) {
            //     this->load_scale_factor = 1;
            //     status = 1;
            // }
            Serial.print("Loaded scale factor: \t");
            Serial.printf("%10.10f\n", this->load_scale_factor);
            Serial.print("Loaded scale offset: \t");
            Serial.printf("%10.10f\n", this->load_scale_offset);

            scale.set_scale(this->load_scale_factor);
            scale.set_offset(this->load_scale_offset);
            
            scale.tare(100);

            analogReadResolution(12);

            return status;
        }

        void calibrate(Logging &logger)
        {   
            u_int8_t mode = 0;

            Serial.println("Enter Calibration Mode: (0 - Points | 1 - Ramp Up)");

            while (Serial.available() < 1)
            {
                delay(100);
            }

            mode = Serial.read();

            if(mode != '0' && mode != '1')
            {
                Serial.printf("%d is not a valid mode. Exiting Calibration.\n", mode);
                return;
            }

            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

            if(mode == '0')
            {
                Serial.println("POINT CALIBRATION MODE");
                Serial.println("----------");
                Serial.println("Enter number of point samples as a 3 digit number (<= 255): ");
                
                u_int8_t n_points = 0;

                while (Serial.available() < 1)
                {
                    delay(500);
                }

                n_points = Serial.readStringUntil('\n').toInt();

                float target_weights[n_points];
                float measurements[n_points];
                
                for(u_int8_t i = 0; i < n_points; i++)
                {
                    Serial.println("APPLY LOAD BEFORE ENTERING VALUE!");
                    Serial.printf("(Point %d of %d) Enter Applied force in the (lbs): ", i, n_points);

                    while (Serial.available() < 1)
                    {
                        delay(500);
                    }

                    target_weights[i] = Serial.readStringUntil('\n').toFloat();
                    
                    Serial.printf("Target value %f, Sampling Load\n", target_weights[i]);
                    
                    float cum_sample = 0;

                    for(uint8_t j = 0; j < 500; j++)
                    {
                        cum_sample += scale.get_value();
                    }

                    measurements[i] = cum_sample/500;

                }

                 Serial.println("Sampling Complete");

                Serial.printf("Target Weights: %f %f %f\n", target_weights[0], target_weights[1], target_weights[2]);
                Serial.printf("Measurements: %f %f %f\n", measurements[0], measurements[1], measurements[2]);

                logger.writeBytes((u_int8_t*) target_weights, sizeof(target_weights));
                logger.writeBytes((u_int8_t*) "\n", 1);
                logger.writeBytes((u_int8_t*) measurements, sizeof(measurements), true);
                
                Serial.println("Test Complete, Computing Values...");
                LinearRegression reg;

                this->load_scale_factor = reg.calculateSlope(measurements, target_weights, n_points);
                this->load_scale_offset = reg.calculateIntercept(measurements, target_weights, n_points);
                
                this->load_scale_factor = 1/this->load_scale_factor;

                EEPROM.put(0, this->load_scale_factor);
                EEPROM.put(4, this->load_scale_offset);


                Serial.printf("Computed Scale Factor: %f\n", this->load_scale_factor);
                Serial.printf("Computed Scale Offset: %f\n", this->load_scale_offset);
            }
            else
            {
                Serial.println("RAMP UP (INSTRON) CALIBRATION MODE");
                Serial.println("----------");
                Serial.print("Enter Ramp-Rate (lbs/s): ");
                
                while (Serial.available() < 1)
                {
                    delay(100);
                }

                float ramp_rate = Serial.parseFloat();

                Serial.printf("Max Weight Set: %f\n", ramp_rate);
                Serial.print("Enter measurement Duration (s): ");

                while (Serial.available() < 1)
                {
                    delay(100);
                }

                float duration = Serial.parseFloat();

                float sumX = 0.0f, sumY = 0.0f, sumXY = 0.0f, sumX2 = 0.0f;

                float x,y;

                Serial.println("Initializing Scale...");
                scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
                scale.set_scale(1.f);
                scale.tare(10);
                
                Serial.printf("%f second test, press ENTER to begin:");
                size_t n_samples = 0;
                u_int32_t start_time = millis();
                
                while(millis() - start_time < duration)
                {
                    x = scale.get_value();
                    y = (millis() - start_time)/1000 * ramp_rate;

                    sumX += x;
                    sumY += y;
                    sumX2 += x*x;
                    sumXY += x*y;
                }


                Serial.println("Test Complete, Computing Values...");

                float numerator = (float)n_samples * sumXY - sumX * sumY;
                float denominator = (float)n_samples * sumX2 - sumX * sumX;

                // Precompute the result for division
                if (denominator == 0.0f) 
                    this->load_scale_factor = 1.0f;
                else
                    this->load_scale_factor = numerator / denominator;

                logger.writeBytes((u_int8_t*) &duration, 4);
                logger.writeBytes((u_int8_t*) &ramp_rate, 4);                
                logger.writeBytes((u_int8_t*) "\n", 1);
                logger.writeBytes((u_int8_t*) &this->load_scale_factor, 4, true);

                EEPROM.put(0, this->load_scale_factor);

                Serial.printf("Computed Scale Factor: %f\n", this->load_scale_factor);
            }
        }

        float readForce() {
            return scale.get_units();
        }

        float voltageToPressure(float v){ //TODO Tweak this based on observed data
            // out put from 0.0-3.3v 0psi-1600psi
            return (v - 0.1333333) * 633.713;
        }

        float readVoltage(uint8_t pin){
            //Serial.printf("pin %d read value: %d \n", pin, analogRead(pin));
            return (analogRead(pin))/(4095.0) * 3.3; 
        }

};

#endif