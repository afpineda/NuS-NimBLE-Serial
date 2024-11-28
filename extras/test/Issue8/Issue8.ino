/**
 * @file Issue8.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @author cmumme (https://github.com/cmumme).
 *
 * @date 2024-11-27
 *
 * @brief Regression test for issue #8
 *
 * @see https://github.com/afpineda/NuS-NimBLE-Serial/issues/8
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuSerial.hpp"
#include <NimBLEDevice.h>

//-----------------------------------------------------------------------------
// MOCK
//-----------------------------------------------------------------------------

class ServerCallbacks : public NimBLEServerCallbacks
{
    void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *connection)
    {
        Serial.println("Connected!!");
        NuSerial.send("Hello\n");
        NuSerial.println("Ready to receive"); // <- stack overflow occurs
        NuSerial.print("Hello, world!");      // <- stack overflow occurs
        NuSerial.write(0x4A);                 // J <- stack overflow occurs
        NuSerial.write(0x0A);                 // \n  <- stack overflow occurs
        Serial.println("Handshake sent!");
    };
    void onDisconnect(NimBLEServer *pServer)
    {
        NuSerial.end();
    };
};

//-----------------------------------------------------------------------------
// Arduino entry points
//-----------------------------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("--READY--");
    NimBLEDevice::init("Issue8");

    NuSerial.setCallbacks(new ServerCallbacks());
    NuSerial.enableAutoAdvertising();
    NuSerial.start();
    Serial.println("--GO--");
}

void loop()
{
    delay(30000);
    Serial.println("--Heartbeat--");
    NuSerial.println("--I am alive--");
}