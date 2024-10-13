#ifndef Sensors_H
#define Sensors_H
#include <Arduino.h>
#include <string.h>
#include <structs.h>
#include <EEPROM.h>
#include <hX711.h>
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

        HX711 scale;

        Sensors(){
            load_scale_factor = 0.f;
        }

        uint8_t init() {
            u_int8_t status = 0;
            // load initialization from EEPROM and begin
            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
            EEPROM.get(0, load_scale_factor);
            if (! load_scale_factor) {
                load_scale_factor = 1;
                status = 1;
            }
            Serial.print("Loaded scale factor: \t");
            Serial.println(load_scale_factor);

            scale.set_scale(load_scale_factor);
            scale.tare(10);

            analogReadResolution(16);

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

            mode = Serial.read() - 30;

            if(mode != 0 && mode != 1)
            {
                Serial.printf("%d is not a valid mode. Exiting Calibration.\n", mode);
                return;
            }

            scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

            if(mode == 0)
            {
                Serial.println("POINT CALIBRATION MODE");
                Serial.println("----------");
                Serial.println("Enter number of point samples (<= 255): ");
                
                u_int8_t n_points = 0;

                while (Serial.available() < 1)
                {
                    delay(100);
                }

                n_points = Serial.parseInt();

                float target_weights[n_points];
                float measurements[n_points];
                
                for(u_int8_t i = 0; i < n_points; i++)
                {
                    Serial.printf("(Point %d of %d) Enter Applied force (lbs): ", i, n_points);

                    while (Serial.available() < 1)
                    {
                        delay(100);
                    }

                    target_weights[i] = Serial.parseFloat();
                    
                    Serial.println("Sampling Load");
                    
                    float cum_sample = 0;

                    for(uint8_t j = 0; j < 10; j++)
                    {
                        cum_sample += scale.get_value();
                    }

                    measurements[i] = cum_sample/10;

                    Serial.println("Sampling Complete");

                    logger.writeBytes((u_int8_t*) target_weights, sizeof(target_weights));
                    logger.writeBytes((u_int8_t*) "\n", 1);
                    logger.writeBytes((u_int8_t*) measurements, sizeof(measurements), true);
                    
                    Serial.println("Test Complete, Computing Values...");
                    LinearRegression reg;

                    load_scale_factor = reg.calculateSlope(measurements, target_weights, n_points);

                    EEPROM.put(0, &load_scale_factor);

                    Serial.printf("Computed Scale Factor: %f\n", load_scale_factor);

                }
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
                scale.tare(1-0);
                
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
                    load_scale_factor = 1.0f;
                else
                    load_scale_factor = numerator / denominator;

                logger.writeBytes((u_int8_t*) &duration, 4);
                logger.writeBytes((u_int8_t*) &ramp_rate, 4);                
                logger.writeBytes((u_int8_t*) "\n", 1);
                logger.writeBytes((u_int8_t*) &load_scale_factor, 4, true);

                EEPROM.put(0, &load_scale_factor);

                Serial.printf("Computed Scale Factor: %f\n", load_scale_factor);
            }
        }

        float readForce() {
            return scale.get_units(load_scale_factor);
        }

        static float voltageToPressure(float v){ //TODO Tweak this based on observed data
            // out put from 0.0-3.3v 0psi-1600psi
            return v * (1600.0/3.3);
        }

        static float readVoltage(uint8_t pin){
            return analogRead(pin)/(65535.0) * 3.3; 
        }

};

#endif