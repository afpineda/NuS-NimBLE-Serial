/**
 * @file ReadBytesDemo.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-24
 * @brief Example of a readBytes() with no active wait
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "NuSerial.hpp"

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*********************");
    Serial.println(" BLE readBytes()demo ");
    Serial.println("*********************");
    Serial.println("--Initializing--");

    // Initialize BLE stack and Nordic UART service
    NimBLEDevice::init("ReadBytes demo");
    NuSerial.setTimeout(ULONG_MAX); // no timeout at readBytes()
    NuSerial.start();

    // Initialization complete
    Serial.println("--Ready--");
}

void loop()
{
    uint8_t buffer[4];
    size_t readBytes;

    Serial.println("--Waiting for connection--");
    // Block current task until a connection is established.
    // This is not active waiting, so the CPU is free for other tasks.
    if (NuSerial.connect())
    {
        Serial.println("--Connected--");

        // Receive data in chunks of 4 bytes.
        // Current task is blocked until data is received or connection is lost.
        // This is not active waiting.
        readBytes = NuSerial.readBytes(buffer, 4);
        while (readBytes == 4)
        {
            // Dump incoming data to the serial monitor
            Serial.printf("%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3]);

            // Receive next chunk
            readBytes = NuSerial.readBytes(buffer, 4);
        }
        // At this point the connection is lost, but some data may be still unread
        if (readBytes > 0)
        {
            // Dump remaining data
            int i = 0;
            while (readBytes > 0)
            {
                Serial.printf("%c", buffer[i++]);
                readBytes--;
            }
            Serial.println("");
        }
        Serial.println("--Disconnected--");
    }
}