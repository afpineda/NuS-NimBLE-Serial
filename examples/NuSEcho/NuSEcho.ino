#include <Arduino.h>
#include "NuSerial.hpp"
#include "NimBLEDevice.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("*****************************");
    Serial.println(" BLE echo server demo");
    Serial.println("*****************************");
    Serial.println("--Initializing--");
    NimBLEDevice::init("NuSerial Echo");
    NuSerial.begin(115200);
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
                Serial.printf("--Available %d bytes for processing\n", availableCount);
            while (NuSerial.available())
            {
                int bleChar = NuSerial.read();
                if (bleChar < 0)
                    Serial.println("ERROR: NuSerial.read()<0, but NuSerial.available()>0. Should not happen.");
                else
                {
                    if (NuSerial.write((uint8_t)bleChar) < 1)
                        Serial.println("ERROR: NuSerial.write() failed");
                    // Serial.print((char)bleChar);
                    processedCount++;
                }
            }
            if (processedCount)
                Serial.printf("--Stream of %d bytes processed\n", processedCount);
            if (processedCount != availableCount)
                Serial.printf("ERROR: %d bytes were available, but %d bytes were processed.\n", availableCount, processedCount);
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