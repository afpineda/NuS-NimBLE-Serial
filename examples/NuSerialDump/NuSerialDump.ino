/**
 * @file NuSerialDump.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 *
 * @brief Example of a blocking communications stream
 *        based on the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "NuPacket.hpp"

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*****************************");
    Serial.println(" BLE serial dump demo");
    Serial.println("*****************************");
    Serial.println("--Initializing--");

    // Initialize BLE stack and Nordic UART service
    NimBLEDevice::init("NuPacket demo");
    NuPacket.start();

    // Initialization complete
    Serial.println("--Ready--");
}

void loop()
{
    Serial.println("--Waiting for connection--");
    // Block current task until a connection is established.
    // This is not active waiting, so the CPU is free for other tasks.
    if (NuPacket.connect())
    {
        Serial.println("--Connected--");
        // "data" is a pointer to the incoming bytes
        // "size" is the count of bytes pointed by "data"
        size_t size;

        // Receive first packet:
        // current task is blocked until data is received or connection is lost.
        // This is not active waiting.
        const uint8_t *data = NuPacket.read(size);
        while (data)
        {
            // Dump incoming data to the serial monitor
            Serial.printf("--data packet of %d bytes follows--\n", size);
            Serial.write(data, size);
            Serial.println("");
            Serial.println("--end of packet--");

            // Acknowledge data reception
            NuPacket.send("Data received. Ready for more.\n");

            // Receive next packet
            data = NuPacket.read(size);
        }
        // data==nullptr here, which means that the
        // the connection is lost
        Serial.println("--Disconnected--");
    }
}