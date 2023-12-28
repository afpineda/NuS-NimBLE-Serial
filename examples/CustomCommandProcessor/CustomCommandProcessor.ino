/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Example of a custom command processor
 *        based on the Nordic UART Service
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuS.hpp"
#include <NimBLEDevice.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

/**
 * @brief Custom command processor class
 *
 */
class CustomCommandProcessor : public NordicUARTService
{
public:
    // Do not get confused by this method's name.
    // Data is received here.
    void onWrite(NimBLECharacteristic *pCharacteristic) override;

private:
    // Methods that execute received commands
    void onExitCommand();
    void onSumCommand(char *param1, char *param2);
} server;

//------------------------------------------------------
// Globals
//------------------------------------------------------

// Internal buffer size required for command parsing
#define BUFFER_SIZE 64

// Messages for the BLE peer
#define MSG_UNKNOWN_CMD "ERROR: Unknown command. Valid commands are (case sensitive) \"sum <int> <int>\" and \"exit\"\n"
#define MSG_TOO_LONG "ERROR: command line too long\n"
#define MSG_PARAM_ERROR "ERROR: a parameter is not a valid integer\n"
#define MSG_UNEXPECTED_ERROR "ERROR: unexpected failure. Sorry.\n"
#define MSG_GOODBYE "See you later\n"

// Command names
#define CMD_SUM "sum"
#define CMD_EXIT "exit"

//------------------------------------------------------
// CustomCommandProcessor implementation
//------------------------------------------------------

/**
 * @brief Split a string into two parts separated by blank spaces
 *
 * @param[in/out] string On entry, the string to split.
 *                       On exit, the substring at the left side
 * @return char* The substring at the right side. May be empty.
 */
char *split(char *string)
{
    char *rightSide = string;
    // find first blank space or null
    while ((rightSide[0] != ' ') && (rightSide[0] != '\0'))
        rightSide++;
    if (rightSide[0] != '\0')
    {
        rightSide[0] = '\0';
        rightSide++;
        // ignore continuous blank spaces
        while (rightSide[0] == ' ')
            rightSide++;
    }
    return rightSide;
}

/**
 * @brief Parse incoming data
 *
 */
void CustomCommandProcessor::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // convert data to a null-terminated string
    const char *data = pCharacteristic->getValue().c_str();
    Serial.printf("--Incoming text line:\n%s\n", data);

    // Preliminary check to discard wrong commands early
    // and simplify further processing
    auto dataLen = strlen(data);
    if (dataLen > BUFFER_SIZE)
    {
        send(MSG_TOO_LONG);
        return;
    }
    else if ((dataLen < 4) || (data[0] < 'a') || (data[0] > 'z'))
    {
        send(MSG_UNKNOWN_CMD);
        return;
    }

    // Since data is "const", we need a modifiable buffer
    // to parse each parameter (commandLine).
    // A null terminating character will be inserted after the
    // command name and after each parameter.

    // Copy string from "data" to "commandLine"
    char commandLine[BUFFER_SIZE + 1];
    strncpy(commandLine, data, BUFFER_SIZE);
    commandLine[BUFFER_SIZE] = '\0';

    // Substitute unwanted characters with blank spaces
    // (unwanted characters are, mostly, line feeds and carriage returns)
    for (int i = 0; (i < BUFFER_SIZE) && (commandLine[i] != '\0'); i++)
        if ((commandLine[i] < ' ') || (commandLine[i] > 'z'))
            commandLine[i] = ' ';

    // Separate command name from first parameter (if any)
    char *firstParam = split(commandLine);

    // Decode command
    if (strcmp(commandLine, CMD_EXIT) == 0)
    {
        Serial.printf("--Processing \"%s\"\n", CMD_EXIT);
        onExitCommand();
        return;
    }
    else if (strcmp(commandLine, CMD_SUM) == 0)
    {
        char *secondParam = split(firstParam);
        Serial.printf("--Processing \"%s %s %s\"\n", CMD_SUM, firstParam, secondParam);
        onSumCommand(firstParam, secondParam);
        return;
    }
    else
    {
        Serial.printf("--Command %s NOT processed\n", commandLine);
        send(MSG_UNKNOWN_CMD);
    }
}

//------------------------------------------------------
// Command execution
//------------------------------------------------------

void CustomCommandProcessor::onExitCommand()
{
    send(MSG_GOODBYE);
    disconnect();
}

void CustomCommandProcessor::onSumCommand(char *param1, char *param2)
{
    // Convert string parameters into integer values

    // "errno" is used to detect non-integer data
    // errno==0 means no error
    errno = 0;
    intmax_t n1, n2;
    n1 = strtoimax(param1, NULL, 10); // convert first parameter
    if (!errno)
        n2 = strtoimax(param2, NULL, 10); // convert second parameter
    if (errno)
    {
        send(MSG_PARAM_ERROR);
        return;
    }
    else
    {
        // Execute command and send result to the BLE peer

        auto sum = n1 + n2; // command result
        Serial.printf("(sum is %ld)\n", sum);
        // convert command result to string in a private buffer
        char output[BUFFER_SIZE];
        int t = snprintf(output, BUFFER_SIZE, "Sum is %lld\n", sum);
        if ((t >= 0) && (t < BUFFER_SIZE))
        {
            // Transmit result
            send(output);
        }
        else
        {
            // Buffer is too small (t>=BUFFER_SIZE) or encoding error (t<0)
            send(MSG_UNEXPECTED_ERROR);
            Serial.printf("ERROR at onSumCommand()-->snprintf(): return code %d. Increase buffer size >%d.\n", t, BUFFER_SIZE);
        }
    }
}

//------------------------------------------------------
// Arduino entry points
//------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("***********************************");
    Serial.println(" BLE custom command processor demo ");
    Serial.println("***********************************");
    Serial.println("--Initializing--");

    // Initialize BLE and Nordic UART service
    NimBLEDevice::init("Custom commands demo");
    server.start();

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