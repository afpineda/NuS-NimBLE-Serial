/**
 * @file HandshakeTest.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 *
 * @date 2024-11-27
 *
 * @brief Proof of concept. Send a handshake message on connection.
 *
 * @see https://github.com/afpineda/NuS-NimBLE-Serial/issues/8
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuSerial.hpp"
#include <NimBLEDevice.h>

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");

    NimBLEDevice::init("Handshake");
    NuSerial.enableAutoAdvertising();
    NuSerial.begin(115200); // Note: Parameter is ignored.

    Serial.println("--GO--");
}

// unsigned long int count = 0L;
bool lastConnectionStatus = false;

void loop()
{
    if (!lastConnectionStatus && NuSerial.isConnected())
    {
        Serial.println("Device connected!");
        lastConnectionStatus = true;
        // The following delay is required, but I don't know why.
        // Otherwise, NuSerial.printf() will print nothing.
        delay(1000);
        NuSerial.printf("Hello \n");
    }
    else if (lastConnectionStatus && !NuSerial.isConnected())
    {
        Serial.println("Device disconnected!");
        lastConnectionStatus = false;
    }
    // if (lastConnectionStatus)
    // {
    //     // Do stuff
    //     Serial.println("I'm busy");
    //     NuSerial.printf("This is message number %ld.\n", count++);
    //     delay(5000);
    // }
    // else
    // {
    //     // Do background stuff
    //     Serial.println("I'm bored");
    //     delay(5000);
    // }
}
