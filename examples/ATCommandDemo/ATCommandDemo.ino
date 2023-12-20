/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-19
 * @brief Example of a an AT command processor based on
 *        the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuATCommands.hpp"
#include <NimBLEDevice.h>
#include <string.h>

//------------------------------------------------------
// Command implementation
//------------------------------------------------------

class MyATCommandCallbacks : public NuATCommandCallbacks
{
public:
    virtual void onNonATCommand(const char text[]) override;
    virtual int getATCommandId(const char commandName[]) override;
    virtual NuATCommandResult_t onExecute(int commandId) override;
    virtual NuATCommandResult_t onSet(int commandId, NuATCommandParameters_t &parameters) override;
    virtual void onTest(int commandId) override;
private:
    intmax_t number[] = { 0, 0, 0, 0 };
} ATCallbacks;

#define CMD_VERSION 0
#define CMD_SUM 1
#define CMD_NUMBER 2

virtual void MyATCommandCallbacks::onNonATCommand(const char text[])
{
    Serial.println("--Non-AT command received--");
    Serial.println(text);
}

virtual int MyATCommandCallbacks::getATCommandId(const char commandName[])
{
    Serial.println("--Command identification request--");
    Serial.println(commandName);
    if ((strcmp(commandName, "V") == 0) || (strcmp(commandName, "v") == 0))
        return CMD_VERSION;
    if ((strcmp(commandName, "SUM") == 0) || (strcmp(commandName, "pair") == 0))
        return CMD_SUM;
    if ((strcmp(commandName, "NUM") == 0) || (strcmp(commandName, "num") == 0))
        return CMD_NUMBER;
    else
        return -1;
}

virtual NuATCommandResult_t MyATCommandCallbacks::onExecute(int commandId)
{
    Serial.println("--Command execution request--");
    Serial.println(commandId);
    switch (commandID)
    {
    case CMD_ECHO:
        NuATCommands.printATResponse("Version 1.0");
        return NuATCommandResult_t::
            break;

    default:
        return AT_RESULT_ERROR;
        break;
    }
}

virtual NuATCommandResult_t MyATCommandCallbacks::onWrite(int commandId, NuATCommandParameters_t &parameters)
{
    switch (commandID)
    {
    case CMD_ECHO:
        NuATCommands.printATResponse("Version 1.0");
        return NuATCommandResult_t::
            break;

    default:
        return AT_RESULT_ERROR;
        break;
    }
}

virtual void MyATCommandCallbacks::onTest(int commandId)
{
    switch (commandID)
    {
    case CMD_ECHO:
        NuATCommands.printATResponse("Version 1.0");
        return NuATCommandResult_t::
            break;

    default:
        return AT_RESULT_ERROR;
        break;
    }
}

//------------------------------------------------------
// Globals
//------------------------------------------------------
