/**
 * @file NuATCommands.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-21
 * @brief AT command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

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
    const char *in = incomingPacket.c_str();
    execute((const uint8_t *)in, incomingPacket.size());
}

//-----------------------------------------------------------------------------
// Printing
//-----------------------------------------------------------------------------

void NuATCommandProcessor::printATResponse(std::string message)
{
    print("\r\n");
    print(message);
    print("\r\n");
}
