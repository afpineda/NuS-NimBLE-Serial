/**
 * @file NusEcho.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 *
 * @brief Example of a non-blocking communications stream
 *        based on the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuSerial.hpp"
#include "NimBLEDevice.h"

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*****************************");
    Serial.println(" BLE echo server demo        ");
    Serial.println("*****************************");
    Serial.println("--Initializing--");

    // Initialize BLE stack and Nordic UART service
    NimBLEDevice::init("NuSerial Echo");
    NuSerial.begin(115200);

    // Initialization complete
    Serial.println("--Ready--");
}

void loop()
{
    if (NuSerial.isConnected())
    {
        int serialMonitorChar = Serial.read();
        if ((serialMonitorChar == 'E') || (serialMonitorChar == 'e'))
        {
            // Open the serial monitor in Arduino IDE
            // Type "E" or "e" and press ENTER to drop the BLE connection
            Serial.println("--Terminating connection from server side--");
            NuSerial.end();
        }
        else
        {
            int processedCount = 0;
            int availableCount = NuSerial.available();
            if (availableCount)
                Serial.printf("--Available %d bytes for processing--\n", availableCount);
            while (NuSerial.available())
            {
                int bleChar = NuSerial.read();
                if (bleChar < 0)
                    Serial.println("ERROR: NuSerial.read()<0, but NuSerial.available()>0. Should not happen.");
                else
                {
                    // Echo
                    if (NuSerial.write(bleChar) < 1)
                        Serial.println("ERROR: NuSerial.write() failed");

                    // Note: the following delay is required because we are sending data in a byte-by-byte basis.
                    // If we send bytes quicker than they are consumed by the peer,
                    // the internal buffer of NimBLE will overflow, thus losing some bytes.
                    // BLE is designed to transmit a larger chunk of bytes slowly rather than a single byte quickly.
                    // That's another reason to use NuPacket instead of NuSerial.
                    delay(30);

                    // log ASCII/ANSI codes
                    Serial.printf("%d.", bleChar);
                    processedCount++;
                }
            }

            if (processedCount != availableCount)
                Serial.printf("\nERROR: %d bytes were available, but %d bytes were processed.\n", availableCount, processedCount);
            else if (processedCount)
            {
                Serial.printf("\n--Stream of %d bytes processed--\n", processedCount);
            }
        }
    }
    else
    {
        Serial.println("--Waiting for connection--");
        while (!NuSerial.isConnected())
            delay(500);
        Serial.println("--Connected--");
    }
}