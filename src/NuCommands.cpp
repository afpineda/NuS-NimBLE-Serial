/**
 * @file NuCommands.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-30
 * @brief AT/Shell command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <exception>
#include "NuATCommands.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NuCommandProcessor &NuCommands = NuCommandProcessor::getInstance();

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NuCommandProcessor::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Incoming data
    NimBLEAttValue incomingPacket = pCharacteristic->getValue();
    // const char *in = pCharacteristic->getValue().c_str();
    const char *in = incomingPacket.c_str();
    // Serial.printf("onWrite(): %s\n");

    // Parse
    parseCommandLine(in);
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void NuCommandProcessor::setATCommandCallbacks(NuATCommandCallbacks *pCallbacks)
{
    if (!isConnected())
        NuATCommandParser::setATCallbacks(pCallbacks);
    else
        throw std::runtime_error("Unable to set AT command callbacks while connected");
}

//-----------------------------------------------------------------------------
// Printing
//-----------------------------------------------------------------------------

void NuCommandProcessor::printATResponse(const char message[])
{
    send("\r\n");
    send(message);
    send("\r\n");
}
