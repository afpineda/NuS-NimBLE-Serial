/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-27
 * @brief Shell command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <exception>
#include "NuShellCommands.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NuShellCommandProcessor &NuShellCommands = NuShellCommandProcessor::getInstance();

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NuShellCommandProcessor::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Incoming data
    NimBLEAttValue incomingPacket = pCharacteristic->getValue();
    const char *in = incomingPacket.c_str();
    // Serial.printf("onWrite(): %s\n");

    // Parse
    parseCommandLine(in);
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void NuShellCommandProcessor::setShellCommandCallbacks(NuShellCommandCallbacks *pCallbacks)
{
    if (!isConnected())
        NuShellCommandParser::setShellCommandCallbacks(pCallbacks);
    else
        throw std::runtime_error("Unable to set shell command callbacks while connected");
}
