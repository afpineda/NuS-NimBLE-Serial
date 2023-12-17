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
    void onWrite(NimBLECharacteristic *pCharacteristic) override;

private:
    void onExitCommand();
    void onSumCommand(char *param1, char *param2);
} server;

//------------------------------------------------------
// Globals
//------------------------------------------------------

#define BUFFER_SIZE 32

#define MSG_UNKNOWN_CMD "ERROR: Unknown command. Valid commands are \"sum <int> <int>\" and \"exit\"\n"
#define MSG_TOO_LONG "ERROR: command line too long\n"
#define MSG_PARAM_ERROR "ERROR: a parameter is not a valid integer\n"
#define MSG_GOODBYE "See you later\n"

#define CMD_SUM "sum"
#define CMD_EXIT "exit"

//------------------------------------------------------
// CustomCommandProcessor
//------------------------------------------------------

/**
 * @brief Split a string into two parts separated by blank spaces
 *
 * @param[in/out] string On entry, the string to split.
 *                      On exit, the substring at the left side
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
    const char *data = pCharacteristic->getValue().c_str();
    Serial.printf("--Incoming text line:\n%s\n", data);

    // Preliminary check to discard wrong commands
    auto dataLen = strlen(data);
    if (dataLen > BUFFER_SIZE)
    {
        send(MSG_TOO_LONG);
        return;
    }
    else if ((dataLen < 4) || ((data[0] < 'a') && (data[0] > 'z')))
    {
        send(MSG_UNKNOWN_CMD);
        return;
    }

    // Since data is "const", we need a modifiable buffer
    // to parse each parameter
    char commandLine[BUFFER_SIZE + 1];
    strncpy(commandLine, data, BUFFER_SIZE);
    commandLine[BUFFER_SIZE] = '\0';

    // Substitute unwanted characters with blank spaces
    for (int i = 0; (i < BUFFER_SIZE) && (commandLine[i] != '\0'); i++)
        if ((commandLine[i] < ' ') || (commandLine[i] > 'z'))
            commandLine[i] = ' ';

    // Separate command name from first parameter (if any)
    char *firstParam = split(commandLine);

    // Process each command
    if (strcmp(commandLine, CMD_EXIT) == 0)
    {
        Serial.printf("--Processing \"%s\"\n",CMD_EXIT);
        onExitCommand();
        return;
    }
    else if (strcmp(commandLine, CMD_SUM) == 0)
    {
        char *secondParam = split(firstParam);
        Serial.printf("--Processing \"%s %s %s\"\n",CMD_SUM,firstParam,secondParam);
        onSumCommand(firstParam, secondParam);
        return;
    }
    else {
        Serial.printf("--Command %s NOT processed\n",commandLine);
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
    errno = 0;
    intmax_t n1, n2;
    n1 = strtoimax(param1, NULL, 10);
    n2 = 0;
    if (!errno)
        n2 = strtoimax(param2, NULL, 10);
    if (errno)
    {
        send(MSG_PARAM_ERROR);
        return;
    }
    else
    {
        auto sum = n1 + n2;
        Serial.printf("(sum is %ld)\n",sum);
        char output[BUFFER_SIZE];
        // memset(output,BUFFER_SIZE,0);
        // FAILURE HERE
        snprintf(output, BUFFER_SIZE, "Sum is %d\n", sum);
        Serial.printf(output);
        Serial.println("");
        send(output);
    }
}

//------------------------------------------------------
// Arduino entry points
//------------------------------------------------------

void setup()
{
    Serial.begin(115200);
    Serial.println("******************************");
    Serial.println(" BLE custom command processor");
    Serial.println("******************************");
        Serial.println("--Initializing--");
    NimBLEDevice::init("Custom commands demo");
    server.start();
    Serial.println("--Ready--");
}

void loop()
{
    Serial.println("--Running (heart beat each 30 seconds)--");
    delay(30000);
}
