/**
 * @file logging.h
 * @author Shivam Patel (shivam.patel94@rutgers.edu)
 * @brief Outlines the functions and variables necessary to manage data logging
 * @version 1.0
 * @date 2024-10-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "structs.h"
#include <SdFat.h>
#include <Arduino.h>
#include <strings.h>

#define DATA_FILE_BASE_NAME String("StandLog_")
#define DATA_FILE_BASE_TYPE String(".bin")
#define LOGGING_BUFFER_SIZE 10 // leq 255

#ifndef Logging_H
#define Logging_H

class Logging{
    public:
        bool sdExists;

        String dataFileName;

        Logging();

        u_int8_t init();

        template <typename T> void logBinaryPacket(const T* packet)
        {
            bufferCount++;
            this->dataFile.write((const uint8_t *)packet, sizeof(T));
            if (bufferCount >= LOGGING_BUFFER_SIZE)
            {
                this->dataFile.sync();
                bufferCount = 0;
            }
        };

        void writeBytes(const uint8_t* bytes, size_t length, bool force_sync=false);

        void printLoadCellPacket(const LoadCellPacket &packet);
        void printPressurePacket(const PressurePacket &packet);

    private:
        u_int8_t bufferCount;
        SdFat sd;
        FsFile dataFile;
};



#endif 