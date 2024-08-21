/**
 * @file ATCommandDemo.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-21
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
#include <exception>

//------------------------------------------------------
// Auxiliary
//------------------------------------------------------

void printNumberATResponse(intmax_t number)
{
    NuATCommands.printATResponse(std::to_string(number));
}

bool strToIntMax(const std::string text, intmax_t &number)
{
    try
    {
        number = std::stoll(text);
        return true;
    }
    catch (std::exception &e)
    {
        return false;
    }
}

//------------------------------------------------------
// AT Commands implementation for a simple calculator
//------------------------------------------------------

intmax_t op1 = 0LL;
intmax_t op2 = 0LL;

NuATCommandResult_t onVersion(NuATCommandParameters_t &parameters)
{
    NuATCommands.printATResponse("Version 2.0 (fictional)");
    return NuATCommandResult_t::AT_RESULT_OK;
}

NuATCommandResult_t onAdd(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1 + op2);
    return NuATCommandResult_t::AT_RESULT_OK;
}

NuATCommandResult_t onSub(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1 - op2);
    return NuATCommandResult_t::AT_RESULT_OK;
}

NuATCommandResult_t onMult(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1 * op2);
    return NuATCommandResult_t::AT_RESULT_OK;
}

NuATCommandResult_t onDiv(NuATCommandParameters_t &parameters)
{
    if (op2 != 0LL)
    {
        printNumberATResponse(op1 / op2);
        return AT_RESULT_OK;
    }
    return NuATCommandResult_t::AT_RESULT_ERROR;
}

NuATCommandResult_t onSetOp1(NuATCommandParameters_t &parameters)
{
    if ((parameters.size() == 1) && strToIntMax(parameters.at(0), op1))
        return AT_RESULT_OK;
    else
        return AT_RESULT_INVALID_PARAM;
}

NuATCommandResult_t onSetOp2(NuATCommandParameters_t &parameters)
{
    if ((parameters.size() == 1) && strToIntMax(parameters.at(0), op2))
        return AT_RESULT_OK;
    else
        return AT_RESULT_INVALID_PARAM;
}

NuATCommandResult_t onSetOperands(NuATCommandParameters_t &parameters)
{
    if ((parameters.size() == 2) &&
        strToIntMax(parameters.at(0), op1) &&
        strToIntMax(parameters.at(1), op2))
        return AT_RESULT_OK;
    else
        return AT_RESULT_INVALID_PARAM;
}

NuATCommandResult_t onQueryOp1(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1);
    return AT_RESULT_OK;
}

NuATCommandResult_t onQueryOp2(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1);
    return AT_RESULT_OK;
}

NuATCommandResult_t onQueryOperands(NuATCommandParameters_t &parameters)
{
    printNumberATResponse(op1);
    printNumberATResponse(op2);
    return AT_RESULT_OK;
}

NuATCommandResult_t onTestOp1(NuATCommandParameters_t &parameters)
{
    NuATCommands.printATResponse("+A: (integer)");
    return AT_RESULT_OK;
}

NuATCommandResult_t onTestOp2(NuATCommandParameters_t &parameters)
{
    NuATCommands.printATResponse("+B: (integer)");
    return AT_RESULT_OK;
}

NuATCommandResult_t onTestOperands(NuATCommandParameters_t &parameters)
{
    NuATCommands.printATResponse("+OP: (integer),(integer)");
    return AT_RESULT_OK;
}

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
    NuATCommands.allowLowerCase(true);
    NuATCommands.stopOnFirstFailure(true);
    NuATCommands
        .onSet("a", onSetOp1)
        .onSet("b", onSetOp2)
        .onSet("op", onSetOperands);
    NuATCommands
        .onExecute("v", onVersion)
        .onExecute("add", onAdd)
        .onExecute("sum", onAdd)
        .onExecute("sub", onSub)
        .onExecute("subtract", onSub)
        .onExecute("mult", onMult)
        .onExecute("div", onDiv);
    NuATCommands
        .onQuery("v", onVersion)
        .onQuery("add", onAdd)
        .onQuery("sum", onAdd)
        .onQuery("sub", onSub)
        .onQuery("subtract", onSub)
        .onQuery("mult", onMult)
        .onQuery("div", onDiv)
        .onQuery("a", onQueryOp1)
        .onQuery("b", onQueryOp2)
        .onQuery("op", onQueryOperands);
    NuATCommands
        .onTest("a", onTestOp1)
        .onTest("b", onTestOp1)
        .onTest("op", onTestOperands);

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