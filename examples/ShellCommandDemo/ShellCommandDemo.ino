/**
 * @file ShellCommandDemo.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-27
 * @brief Example of a shell command processor based on
 *        the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuShellCommands.hpp"
#include <NimBLEDevice.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

//------------------------------------------------------
// Auxiliary functions
//------------------------------------------------------

/**
 * @brief Parse a string into an integer number
 *
 * @param[in] text String containing an integer
 * @param[out] number The number contained in @p text
 * @return true On success
 * @return false If @p text does not contain a valid integer
 */
bool strToIntMax(std::string text, intmax_t &number)
{
    // "errno" is used to detect non-integer data
    // errno==0 means no error
    errno = 0;
    intmax_t r = strtoimax(text.c_str(), NULL, 10);
    if (errno == 0)
    {
        number = r;
        return true;
    }
    else
        return false;
}

/**
 * @brief Parse two strings into integer numbers and react to errors
 *
 */
bool strArgsToInt(std::string arg1, std::string arg2, intmax_t &num1, intmax_t &num2)
{
    if (strToIntMax(arg1, num1))
    {
        if (strToIntMax(arg2, num2))
            return true;
        NuShellCommands.send("ERROR: 2nd argument is not a valid integer\n");
    }
    else
        NuShellCommands.send("ERROR: 1st argument is not a valid integer\n");
    return false;
}

void printArgError()
{
    NuShellCommands.send("ERROR: expected a command and two arguments\n");
}

//------------------------------------------------------
// Shell Commands implementation for a simple calculator
//------------------------------------------------------

void onAdd(NuCommandLine_t &commandLine)
{
    if (commandLine.size() != 3)
    {
        printArgError();
        return;
    }
    intmax_t n1, n2;
    if (strArgsToInt(commandLine[1], commandLine[2], n1, n2))
        NuShellCommands.printf("%lld\n", (n1 + n2));
}

void onSubtract(NuCommandLine_t &commandLine)
{
    if (commandLine.size() != 3)
    {
        printArgError();
        return;
    }
    intmax_t n1, n2;
    if (strArgsToInt(commandLine[1], commandLine[2], n1, n2))
        NuShellCommands.printf("%lld\n", (n1 - n2));
}

void onMultiply(NuCommandLine_t &commandLine)
{
    if (commandLine.size() != 3)
    {
        printArgError();
        return;
    }
    intmax_t n1, n2;
    if (strArgsToInt(commandLine[1], commandLine[2], n1, n2))
        NuShellCommands.printf("%lld\n", (n1 * n2));
}

void onDivide(NuCommandLine_t &commandLine)
{
    if (commandLine.size() != 3)
    {
        printArgError();
        return;
    }
    intmax_t n1, n2;
    if (strArgsToInt(commandLine[1], commandLine[2], n1, n2))
    {
        if (n2 != 0)
            NuShellCommands.printf("%lld\n", (n1 / n2));
        else
            NuShellCommands.send("ERROR: divide by zero\n");
    }
}

void onParseError(NuCLIParsingResult_t parsingResult, size_t index)
{
    if (parsingResult == CLI_PR_ILL_FORMED_STRING)
        NuShellCommands.printf("SYNTAX ERROR at char index %d. Code %d.\n", index, parsingResult);
    else if (parsingResult == CLI_PR_NO_COMMAND)
    {
        NuShellCommands.send("Available commands: sum sub div mult\n");
    }
}

//------------------------------------------------------
// Arduino entry points
//------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("**********************************");
    Serial.println(" BLE shell command processor demo ");
    Serial.println("**********************************");
    Serial.println("--Initializing--");

    // Set callbacks
    NuShellCommands
        .on("add",onAdd)
        .on("sum",onAdd)
        .on("sub",onSubtract)
        .on("subtract",onSubtract)
        .on("mult",onMultiply)
        .on("multiply",onMultiply)
        .on("div",onDivide)
        .on("divide",onDivide)
        .onUnknown([](NuCommandLine_t &commandLine)
        {
            NuShellCommands.printf("ERROR: Unknown command \"%s\"\n",commandLine[0].c_str());
        })
        .onParseError(onParseError);

    // Initialize BLE and Nordic UART service
    NimBLEDevice::init("Shell commands demo");
    NuShellCommands.caseSensitive(false);
    NuShellCommands.start();

    // Initialization complete
    Serial.println("--Ready--");
}

void loop()
{
    // Incoming data is processed in another task created by the BLE stack,
    // so there is nothing to do here (in this demo)
    Serial.println("--Running (heart beat each 30 seconds)--");
    delay(30000);
}