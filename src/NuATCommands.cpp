/**
 * @file NuATCommands.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief AT command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <exception>
#include "NuATCommands.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NuATCommandProcessor &NuATCommands = NuATCommandProcessor::getInstance();

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NuATCommandProcessor::onWrite(NimBLECharacteristic *pCharacteristic)
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

void NuATCommandProcessor::setATCallbacks(NuATCommandCallbacks *pCallbacks)
{
    if (!isConnected())
        NuATCommandParser::setATCallbacks(pCallbacks);
    else
        throw std::runtime_error("Unable to set AT command callbacks while connected");
}

//-----------------------------------------------------------------------------
// Printing
//-----------------------------------------------------------------------------

void NuATCommandProcessor::printATResponse(const char message[])
{
    send("\r\n");
    send(message);
    send("\r\n");
}
