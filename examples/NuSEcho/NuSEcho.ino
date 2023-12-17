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
            while (NuSerial.available())
            {
                int bleChar = NuSerial.read();
                if (bleChar<0) 
                    Serial.println("ERROR: NuSerial.read()<0, but NuSerial.available()>0. Should not happen.");
                else {
                    if (NuSerial.write((uint8_t)bleChar)<1)
                        Serial.println("ERROR: NuSerial.write() failed");
                else {
                    Serial.print((char)bleChar);
                }
                Serial.println("");
            }
            // int bleChar = NuSerial.read();
            // if (bleChar >= 0)
            //     NuSerial.write((uint8_t)bleChar); // Echo
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