#include <Arduino.h>
#include <NimBLEDevice.h>
#include "NuStream.hpp"

void setup()
{
    Serial.begin(115200);
    Serial.println("*****************************");
    Serial.println(" BLE Serial dump demo");
    Serial.println("*****************************");
    Serial.println("--Initializing--");
    NimBLEDevice::init("NuStream demo");
    NuStream.start();
    Serial.println("--Ready--");
}

void loop()
{
    Serial.println("--Waiting for connection--");
    if (NuStream.connect())
    {
        Serial.println("--Connected--");
        size_t size;
        const uint8_t *data = NuStream.read(size);
        while (data)
        {
            Serial.printf("--data packet of %d bytes follows--\n",size);
            Serial.write(data, size);
            Serial.println("");
            Serial.println("--end of packet--");
            NuStream.send("Data received. Ready for more.\n");
            data = NuStream.read(size);
        }
        Serial.println("--Disconnected--");
    }
}