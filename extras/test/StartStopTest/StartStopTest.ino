/**
 * @file StartStopTest.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2025-05-27
 *
 * @brief Test multiple service start / stop events
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuPacket.hpp"
#include "NimBLEDevice.h"

#define DEVICE_NAME "Start Stop Test"

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*****************************");
    Serial.println(" Start / Stop test           ");
    Serial.println("*****************************");
    Serial.println("--Ready--");
}

void loop()
{
    // Print service state
    Serial.println();
    if (NuPacket.isStarted())
        Serial.println("NuS is started");
    else
        Serial.println("NuS is stopped");
    Serial.println("Enter any character to change the service state...");

    // Wait for serial input
    while (!Serial.available())
        ;
    // Remove serial input
    while (Serial.available())
        Serial.read();

    // Change service state
    if (NuPacket.isStarted())
    {
        Serial.println("Stoping NuS...");
        NuPacket.stop();
        Serial.println("Deinitializing BLE...");
        NimBLEDevice::deinit(true);
    } else
    {
        Serial.println("Initializing BLE...");
        NimBLEDevice::init(DEVICE_NAME);
        NimBLEDevice::getAdvertising()->setName(DEVICE_NAME);
        Serial.println("Starting Nus...");
        NuPacket.start();
    }
}