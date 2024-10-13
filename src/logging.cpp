/**
 * @file logging.h
 * @author Shivam Patel (shivam.patel94@rutgers.edu)
 * @brief Manages logging for thrust stand system
 * @version 1.0
 * @date 2024-10-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "logging.h"

Logging::Logging()
{
    this->sdExists = false;
};

u_int8_t Logging::init()
{
    delay(1000);
    byte attempts = 1;
    u_int8_t status = 1 * this->sdExists;
    
    while (attempts <= 10 && !this->sdExists) {
        if (!this->sd.begin(SdioConfig(FIFO_SDIO))) {
            status = 1;
            Serial.print("SD Begin Failed, Attempting "); Serial.print(10 - attempts++); Serial.println(" more tries ...");
            delay(1000);
        } else {
            status = 0;
            Serial.println("\nFIFO SDIO mode.");

            // INIT DATA FILE

            uint16_t n_file = 0;
            this->dataFileName = DATA_FILE_BASE_NAME + "0000" + DATA_FILE_BASE_TYPE;
            while(sd.exists(this->dataFileName))
            {
                n_file++;
                char fIdx[10];
                sprintf(fIdx, "%04d", n_file);
                this->dataFileName = DATA_FILE_BASE_NAME + fIdx + DATA_FILE_BASE_TYPE;
            }

            this->dataFile = sd.open(this->dataFileName, FILE_WRITE);
            Serial.print("Writing data to: ");
            Serial.println(this->dataFileName);
            if (!this->dataFile) {
                status = 1;
                Serial.println("Failed opening data file.");
                break;
            }

            this->sdExists = true;
        }
    }

    this->bufferCount = 0;

    return status;
};

void Logging::writeBytes(const uint8_t* bytes, size_t length, bool force_sync=false)
{
    if (sdExists)
    {
        bufferCount++;
        this->dataFile.write(bytes, length);
        if (force_sync || bufferCount >= LOGGING_BUFFER_SIZE)
        {
            this->dataFile.sync();   
            bufferCount = 0;
        }
    }
};

void Logging::printLoadCellPacket(const LoadCellPacket &packet) {
    Serial.printf("%u,%u,%6.6f,%u,%u\n", packet.magic, packet.time_ms, packet.force, packet.status, packet.checksum);
}

// Function to print PressurePacket
void Logging::printPressurePacket(const PressurePacket &packet) {
    Serial.printf("%u,%u,%u,%u,%6.6f,%6.6f,%u\n", packet.magic, packet.status, packet.idx, packet.time_ms, packet.voltage, packet.pressure, packet.checksum);
}