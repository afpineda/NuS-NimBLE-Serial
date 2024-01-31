/**
 * @file ATCommandDemo.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-24
 * @brief Example of an AT command processor based on
 *        the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuATCommands.hpp"
#include <NimBLEDevice.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

//------------------------------------------------------
// AT Commands implementation for a simple calculator
//------------------------------------------------------

class MyATCommandCallbacks : public NuATCommandCallbacks
{
public:
    // commands are implemented by overriding the following Methods

    virtual int getATCommandId(const char commandName[]) override;
    virtual NuATCommandResult_t onExecute(int commandId) override;
    virtual NuATCommandResult_t onSet(int commandId, NuATCommandParameters_t &parameters) override;
    virtual NuATCommandResult_t onQuery(int commandId) override;
    virtual void onTest(int commandId) override;

    // The following methods are overriden for logging purposes

    virtual void onNonATCommand(const char text[]) override;
    virtual void onFinished(int index, NuATParsingResult_t parsingResult) override;

private:
    // operands for add, sub, mult and div operators

    intmax_t op1 = 0LL;
    intmax_t op2 = 0LL;

    // macro to print a number into the BLE serial port in AT format
    static void printNumberATResponse(intmax_t number);

    // macro to parse a string into an integer
    static bool strToIntMax(const char text[], intmax_t &number);

} myATCallbacks; // just one global instance is needed

// Identifiers for supported commands (arbitrary non-negative integers)

#define CMD_VERSION 0
#define CMD_ADD 1
#define CMD_SUB 2
#define CMD_MULT 3
#define CMD_DIV 4
#define CMD_OPERAND1 10
#define CMD_OPERAND2 20
#define CMD_OPERANDS 30

void MyATCommandCallbacks::printNumberATResponse(intmax_t number)
{
    char buffer[64]; // should be enough for a single integer number
    snprintf(buffer, 64, "%lld", number);
    buffer[63] = '\0';
    NuATCommands.printATResponse(buffer);
}

bool MyATCommandCallbacks::strToIntMax(const char text[], intmax_t &number)
{
    // "errno" is used to detect non-integer data
    // errno==0 means no error
    errno = 0;
    intmax_t r = strtoimax(text, NULL, 10);
    if (errno == 0)
    {
        number = r;
        return true;
    }
    else
        return false;
}

void MyATCommandCallbacks::onNonATCommand(const char text[])
{
    Serial.println("--Non-AT text received--");
    Serial.println(text);
}

int MyATCommandCallbacks::getATCommandId(const char commandName[])
{
    Serial.println("--Command identification request--");
    Serial.println(commandName);

    // Must return a non-negative integer for supported commands
    // Command aliases returns the same integer
    if ((strcmp(commandName, "V") == 0) || (strcmp(commandName, "v") == 0))
        return CMD_VERSION;
    if ((strcmp(commandName, "A") == 0) || (strcmp(commandName, "a") == 0))
        return CMD_OPERAND1;
    if ((strcmp(commandName, "B") == 0) || (strcmp(commandName, "b") == 0))
        return CMD_OPERAND2;
    if ((strcmp(commandName, "OP") == 0) || (strcmp(commandName, "op") == 0))
        return CMD_OPERANDS;
    if ((strcmp(commandName, "ADD") == 0) || (strcmp(commandName, "add") == 0))
        return CMD_ADD;
    if ((strcmp(commandName, "SUM") == 0) || (strcmp(commandName, "sum") == 0))
        return CMD_ADD;
    if ((strcmp(commandName, "SUBTRACT") == 0) || (strcmp(commandName, "subtract") == 0))
        return CMD_SUB;
    if ((strcmp(commandName, "SUB") == 0) || (strcmp(commandName, "sub") == 0))
        return CMD_SUB;
    if ((strcmp(commandName, "MULT") == 0) || (strcmp(commandName, "mult") == 0))
        return CMD_MULT;
    if ((strcmp(commandName, "DIVIDE") == 0) || (strcmp(commandName, "divide") == 0))
        return CMD_DIV;
    if ((strcmp(commandName, "DIV") == 0) || (strcmp(commandName, "div") == 0))
        return CMD_DIV;
    Serial.println("-- command not supported. Supported commands are: V A B OP ADD SUB MULT DIV --");

    //  Must return a negative integer for unsupported commands
    return -1;
}

NuATCommandResult_t MyATCommandCallbacks::onExecute(int commandId)
{
    Serial.printf("--Command execution (no parameters). ID %d--\n", commandId);
    switch (commandId)
    {
    case CMD_VERSION:
        NuATCommands.printATResponse("Version 1.0 (fictional)");
        return AT_RESULT_OK;
    case CMD_ADD:
        printNumberATResponse(op1 + op2);
        return AT_RESULT_OK;
    case CMD_SUB:
        printNumberATResponse(op1 - op2);
        return AT_RESULT_OK;
    case CMD_MULT:
        printNumberATResponse(op1 * op2);
        return AT_RESULT_OK;
    case CMD_DIV:
        if (op2 != 0LL)
        {
            printNumberATResponse(op1 / op2);
            return AT_RESULT_OK;
        }
        break;
    }
    return AT_RESULT_ERROR;
}

NuATCommandResult_t MyATCommandCallbacks::onSet(int commandId, NuATCommandParameters_t &parameters)
{
    Serial.printf("--Command execution (with parameters). ID %d--\n", commandId);
    int c = 1;
    for (const char *param : parameters)
        Serial.printf("Parameter %d: %s\n", c++, param);

    switch (commandId)
    {
    case CMD_OPERAND1:
        if ((parameters.size() == 1) && strToIntMax(parameters.at(0), op1))
            return AT_RESULT_OK;
        else
            return AT_RESULT_INVALID_PARAM;
        break;
    case CMD_OPERAND2:
        if ((parameters.size() == 1) && strToIntMax(parameters.at(0), op1))
            return AT_RESULT_OK;
        else
            return AT_RESULT_INVALID_PARAM;
        break;
    case CMD_OPERANDS:
        if ((parameters.size() == 2) && strToIntMax(parameters.at(0), op1) && strToIntMax(parameters.at(1), op2))
            return AT_RESULT_OK;
        else
            return AT_RESULT_INVALID_PARAM;
        break;
    }
    return AT_RESULT_ERROR;
}

NuATCommandResult_t MyATCommandCallbacks::onQuery(int commandId)
{
    Serial.printf("--Data request. ID %d--\n", commandId);
    switch (commandId)
    {
    case CMD_OPERAND1:
        printNumberATResponse(op1);
        return AT_RESULT_OK;
    case CMD_OPERAND2:
        printNumberATResponse(op1);
        return AT_RESULT_OK;
    case CMD_OPERANDS:
        printNumberATResponse(op1);
        printNumberATResponse(op2);
        return AT_RESULT_OK;
    }
    Serial.println("--Routing as an \"execute\" command--");
    return onExecute(commandId);
}

void MyATCommandCallbacks::onTest(int commandId)
{
    Serial.printf("--Command Syntax request. ID %d--\n", commandId);
    switch (commandId)
    {
    case CMD_OPERAND1:
        NuATCommands.printATResponse("+A: (integer)");
        return;
    case CMD_OPERAND2:
        NuATCommands.printATResponse("+B: (integer)");
        return;
    case CMD_OPERANDS:
        NuATCommands.printATResponse("+OP: (integer),(integer)");
        return;
    }
}

void MyATCommandCallbacks::onFinished(int index, NuATParsingResult_t parsingResult)
{
    Serial.printf("--Command at index %d was parsed with result code %d--\n", index, parsingResult);
}

void connectionStatusChanged(const bool status)
{
    if (status)
    {
        Serial.println("-- Client connected");
    }
    else
    {
        Serial.println("-- Client disconnected");
    }
}

class MyServerCallbacks : public NimBLEServerCallbacks
{
public:
    virtual void onConnect(NimBLEServer *pServer) override
    {
        Serial.println("-- Client connected");
    };

    virtual void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc) override
    {
        Serial.println("-- Client disconnected");
    };

} myServerCallbacks;

//------------------------------------------------------
// Arduino entry points
//------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*******************************");
    Serial.println(" BLE AT command processor demo ");
    Serial.println("*******************************");
    Serial.println("--Initializing--");

    // Initialize BLE and Nordic UART service
    NimBLEDevice::init("AT commands demo");
    NuATCommands.setBufferSize(64);
    NuATCommands.lowerCasePreamble(true);
    NuATCommands.setATCallbacks(&myATCallbacks);
    NuATCommands.setCallbacks(&myServerCallbacks);
    NuATCommands.start();

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