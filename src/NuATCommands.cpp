/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief AT command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <string.h>
#include <exception>
#include "NuATCommands.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NuATCommandProcessor &NuATCommands = NuATCommandProcessor::getInstance();

//-----------------------------------------------------------------------------
// Parsing macros
//-----------------------------------------------------------------------------

inline bool isATPreamble(const char *in)
{
    return ((in[0] == 'A') && (in[1] == 'T') || (in[0] == 'a') && (in[1] == 't'));
}

inline bool isCommandEndToken(const char c)
{
    return (c == '\n') || (c == '\0') || (c == ';');
}

bool isAlphaString(const char *in)
{
    while (((in[0] >= 'A') && (in[0] <= 'Z')) || ((in[0] >= 'a') && (in[0] <= 'z')))
        in++;
    return (in[0] == '\0');
}

const char *followingCommand(const char *in, NuATCommandResult_t conditional = AT_RESULT_OK)
{
    if ((conditional < 0) || (in[0] == '\0') || (in[0] == '\n'))
        return nullptr;
    else if (in[0] == ';')
        return in + 1;
    else
        return nullptr;
}

const char *findSuffix(const char *in)
{
    while ((in[0] != '\0') && (in[0] != '\n') && (in[0] != ';') && (in[0] != '?') && (in[0] != '='))
        in++;
    return in;
}

//-----------------------------------------------------------------------------
// Parsing machinery
//-----------------------------------------------------------------------------

void NuATCommandParser::parseCommandLine(const char *in)
{
    if (!pCmdCallbacks)
        // no callbacks: nothing to do here
        return;

    size_t textLength = strlen(in);
    // Detect AT preamble
    if ((textLength >= 2) && isATPreamble(in))
    {
        // skip preamble
        in = in + 2;
        textLength = textLength - 2;
        if ((textLength <= 0) || (in[0] != '\n'))
        {
            // This is a preamble with no commands at all.
            // Response is OK to signal that AT commands are accepted.
            printResultResponse(AT_RESULT_OK);
            return;
        }
    }
    else
    {
        // Not an AT command
        pCmdCallbacks->onNonATCommand(in);
        return;
    }

    // Parse all commands contained in incoming data
    do
    {
        in = parseSingleCommand(in);
    } while (in);
}

const char *NuATCommandParser::parseSingleCommand(const char *in)
{
    // Detect prefix.
    // Note: if prefix is '&', just a single letter is allowed as command name
    if ((in[0] == '&') || (in[0] == '+'))
    {
        // Prefix is valid, now detect suffix.
        // Text between a prefix and a suffix is a command name.
        // Text between a prefix and ";", "\n" or "\0" is also a command name.
        const char *suffix = findSuffix(in + 1);
        size_t cmdNameLength = suffix - in;
        if ((cmdNameLength > 0) && (cmdNameLength < bufferSize) && ((in[0] != '&') || (cmdNameLength == 1)))
        {
            // store command name in "cmdName" as a null-terminated string
            char cmdName[bufferSize];
            memcpy(cmdName, in + 1, cmdNameLength);
            cmdName[cmdNameLength] = '\0';

            if (isAlphaString(cmdName))
            {
                // check command availability
                int commandId = pCmdCallbacks->getATCommandId(cmdName);
                if (commandId >= 0)
                {
                    return parseAction(suffix, commandId);
                } // else this command is not supported

            } // else command name contains non-alphabetic characters

        } // else invalid prefix

    } // else error: no command name, buffer overflow or command name has "&" prefix but more than one letter
    printResultResponse(AT_RESULT_ERROR);
    return nullptr;
}

const char *NuATCommandParser::parseAction(const char *in, int commandId)
{
    // Note: "in" points to a suffix or an end-of-command token
    if ((in[0] == '=') && (in[1] == '?'))
    {
        // This is a TEST command
        if (isCommandEndToken(in[2]))
        {
            pCmdCallbacks->onTest(commandId);
            printResultResponse(AT_RESULT_OK);
            return followingCommand(in + 2);
        } // else syntax error
    }
    else if (in[0] == '?')
    {
        // This is a READ/QUERY command
        if (isCommandEndToken(in[2]))
        {
            NuATCommandResult_t response = pCmdCallbacks->onQuery(commandId);
            printResultResponse(response);
            return followingCommand(in + 1, response);
        } // else syntax error
    }
    else if (in[0] == '=')
    {
        // This is a SET/WRITE command
        return parseWriteParameters(in + 1, commandId);
    }
    else if (isCommandEndToken(in[0]))
    {
        // This is an EXECUTE Command
        NuATCommandResult_t response = pCmdCallbacks->onExecute(commandId);
        printResultResponse(response);
        return followingCommand(in + 1, response);
    } // else syntax error
    printResultResponse(AT_RESULT_ERROR);
    return nullptr;
}

const char *NuATCommandParser::parseWriteParameters(const char *in, int commandId)
{
    NuATCommandParameters_t paramList;
    char buffer[bufferSize];
    size_t l = 0;

    // copy parameters to buffer
    while (!isCommandEndToken(in[0]) && (l < bufferSize))
    {
        buffer[l++] = in[0];
        in++;
    }
    if (l >= bufferSize)
    {
        // buffer overflow
        printResultResponse(AT_RESULT_ERROR);
        return nullptr;
    }
    buffer[l] = '\0';

    // Tokenize
    paramList.push_back(in);
    l = 0;
    while (buffer[l] != '\0')
    {
        if (buffer[l] == ',')
        {
            paramList.push_back(buffer + l + 1);
        }
        l++;
    }

    // Invoke callback
    NuATCommandResult_t response = pCmdCallbacks->onSet(commandId, paramList);
    printResultResponse(response);
    if (response == AT_RESULT_OK)
        return followingCommand(in);
    else
        return nullptr;
}

//-----------------------------------------------------------------------------
// Buffer size
//-----------------------------------------------------------------------------

void NuATCommandParser::setBufferSize(size_t size)
{
    if (size > 5)
    {
        bufferSize = size;
    }
    else
        // absolute minimum
        bufferSize = 5;
}

//-----------------------------------------------------------------------------
// Printing
//-----------------------------------------------------------------------------

void NuATCommandParser::printResultResponse(const NuATCommandResult_t response)
{
    switch (response)
    {
    case AT_RESULT_INVALID_PARAM:
        printATResponse("INVALID INPUT PARAMETERS");
        break;
    case AT_RESULT_ERROR:
        printATResponse("ERROR");
        break;
    case AT_RESULT_OK:
        printATResponse("OK");
        break;
    case AT_RESULT_SEND_OK:
        printATResponse("SEND OK");
        break;
    case AT_RESULT_SEND_FAIL:
        printATResponse("SEND FAIL");
        break;
    }
}

void NuATCommandProcessor::printATResponse(const char message[])
{
    send("\r\n");
    send(message);
    send("\r\n");
}

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NuATCommandProcessor::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Incoming data
    NimBLEAttValue incomingPacket = pCharacteristic->getValue();
    const char *in = pCharacteristic->getValue().c_str();

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
