/**
 * @file NuCommandParser.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-30
 * @brief AT/Shell command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <string.h>
#include <stdlib.h>
#include "NuCommandParser.hpp"

/******************************************************************************
 *
 * AT COMMANDS
 *
 ******************************************************************************/

//-----------------------------------------------------------------------------
// Parsing macros
//-----------------------------------------------------------------------------

inline bool isATPreamble(const char *in, bool allowLowerCase)
{
    return ((in[0] == 'A') && (in[1] == 'T')) || (allowLowerCase && (in[0] == 'a') && (in[1] == 't'));
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
        // should not enter here
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

void NuCommandParser::parseATCommandLine(const char *in)
{
    lastATParsingResult = AT_PR_NO_CALLBACKS;
    // Detect AT preamble
    if (pATCmdCallbacks)
    {
        if (isATPreamble(in, bLowerCasePreamble))
        {
            if ((in[2] == '\n') || (in[2] == '\0'))
            {
                // This is an AT preamble with no commands at all.
                // Response is OK to signal that AT commands are accepted.
                printATResultResponse(AT_RESULT_OK);
                lastATParsingResult = AT_PR_NO_COMMANDS;
                return;
            }
            else if ((in[2] == '+') || (in[2] == '&'))
            {
                // skip preamble
                in = in + 2;
                // Parse all AT commands contained in incoming data
                int commandIndex = 0;
                do
                {
                    // Serial.printf("parseATCommandLine(): %s\n", in);
                    lastATParsingResult = AT_PR_OK; // may be changed later
                    in = parseSingleATCommand(in);
                    try
                    {
                        pATCmdCallbacks->onFinished(commandIndex++, lastATParsingResult);
                    }
                    catch (...)
                    {
                    };
                } while (in);
                return;
            }
            // Not an AT command line
            lastATParsingResult = AT_PR_NO_PREAMBLE;
        }
    }

    parseShellCommandLine(in);
    return;
}

const char *NuCommandParser::parseSingleATCommand(const char *in)
{
    // Detect prefix.
    // Note: if prefix is '&', just a single letter is allowed as command name
    // Serial.printf("parseSingleCommand(): %s\n", in);
    if ((in[0] == '&') || (in[0] == '+'))
    {
        // Prefix is valid, now detect suffix.
        // Text between a prefix and a suffix is a command name.
        // Text between a prefix and ";", "\n" or "\0" is also a command name.
        const char *suffix = findSuffix(in + 1);
        size_t cmdNameLength = suffix - (in + 1);
        if ((cmdNameLength > 0) && (cmdNameLength < bufferSize) && ((in[0] == '+') || (cmdNameLength == 1)))
        {
            // Serial.printf("parseSingleCommand(1): %s. Suffix: %s. Length: %d\n", in + 1, suffix, cmdNameLength);
            // store command name in "cmdName" as a null-terminated string
            // char cmdName[bufferSize];
            char *cmdName = (char *)malloc(cmdNameLength + 1);
            if (!cmdName)
            {
                lastATParsingResult = AT_PR_NO_HEAP;
                printATResultResponse(AT_RESULT_ERROR);
                return nullptr;
            }
            // memcpy(cmdName, in + 1, cmdNameLength);
            for (int i = 0; i < cmdNameLength; i++)
                cmdName[i] = *(in + 1 + i);
            cmdName[cmdNameLength] = '\0';
            // Serial.printf("parseSingleCommand(2): %s. Suffix: %s. Name: %s. Length: %d. pName %d. pIn+1:%d\n", in + 1, suffix, cmdName, cmdNameLength, cmdName, in + 1);

            if (isAlphaString(cmdName))
            {
                // check if command is supported
                int commandId;
                try
                {
                    commandId = pATCmdCallbacks->getATCommandId(cmdName);
                }
                catch (...)
                {
                    commandId = -1;
                }
                if (commandId >= 0)
                {
                    // continue parsing
                    free(cmdName);
                    return parseATAction(suffix, commandId);
                }
                else // this command is not supported
                    lastATParsingResult = AT_PR_UNSUPPORTED_CMD;
            }
            else // command name contains non-alphabetic characters
                lastATParsingResult = AT_PR_INVALID_CMD2;

            free(cmdName);
        }
        else // error: no command name, buffer overflow or command name has "&" prefix but more than one letter
            lastATParsingResult = AT_PR_INVALID_CMD1;

    } // invalid prefix
    else
    {
        lastATParsingResult = AT_PR_INVALID_PREFIX;
        // Serial.printf("Invalid prefix\n");
    }
    printATResultResponse(AT_RESULT_ERROR);
    return nullptr;
}

const char *NuCommandParser::parseATAction(const char *in, int commandId)
{
    // Serial.printf("parseATAction(): %s\n", in);
    //  Note: "in" points to a suffix or an end-of-command token
    if ((in[0] == '=') && (in[1] == '?'))
    {
        // This is a TEST command
        if (isCommandEndToken(in[2]))
        {
            NuATCommandResult_t result = AT_RESULT_OK;
            try
            {
                pATCmdCallbacks->onTest(commandId);
            }
            catch (...)
            {
                result = AT_RESULT_ERROR;
            }
            printATResultResponse(result);
            return followingCommand(in + 2, result);
        } // else syntax error
    }
    else if (in[0] == '?')
    {
        // This is a READ/QUERY command
        if (isCommandEndToken(in[1]))
        {
            NuATCommandResult_t response;
            try
            {
                response = pATCmdCallbacks->onQuery(commandId);
            }
            catch (...)
            {
                response = AT_RESULT_ERROR;
            }
            printATResultResponse(response);
            return followingCommand(in + 1, response);
        } // else syntax Error
    }
    else if (in[0] == '=')
    {
        // This is a SET/WRITE command
        return parseWriteParameters(in + 1, commandId);
    }
    else if (isCommandEndToken(in[0]))
    {
        // This is an EXECUTE Command
        NuATCommandResult_t response;
        try
        {
            response = pATCmdCallbacks->onExecute(commandId);
        }
        catch (...)
        {
            response = AT_RESULT_ERROR;
        }
        printATResultResponse(response);
        return followingCommand(in, response);
    } // else syntax error
    lastATParsingResult = AT_PR_END_TOKEN_EXPECTED;
    printATResultResponse(AT_RESULT_ERROR);
    return nullptr;
}

const char *NuCommandParser::parseATSetActionParameters(const char *in, int commandId)
{
    // See https://docs.espressif.com/projects/esp-at/en/release-v2.2.0.0_esp8266/AT_Command_Set/index.html
    // about parameters' syntax.

    NuATCommandParameters_t paramList;
    // char buffer[bufferSize];
    char *buffer = (char *)malloc(bufferSize);
    if (!buffer)
    {
        lastATParsingResult = AT_PR_NO_HEAP;
        printATResultResponse(AT_RESULT_ERROR);
        return nullptr;
    }
    size_t l = 0;
    bool doubleQuotes = false;
    bool syntaxError = false;
    char *currentParam = buffer;

    // Parse, tokenize and copy parameters to buffer
    while (!isCommandEndToken(in[0]) && (l < bufferSize))
    {
        if (doubleQuotes)
        {
            if ((in[0] == '\"') && ((in[1] == ',') || isCommandEndToken(in[1])))
            {
                // Closing double quotes
                doubleQuotes = false;
                in++;
                continue;
            }
            else if (in[0] == '\"')
            {
                // there is more text after the closing double quotes
                syntaxError = true;
                break;
            }
            else if ((in[0] == '\\') && (in[1] != '\0'))
            {
                // Escaped character
                in++;
                buffer[l++] = in[0];
                in++;
                continue;
            }
        }
        else
        {
            if ((in[0] == '\"') && (currentParam == (buffer + l)))
            {
                // Opening double quotes
                doubleQuotes = true;
                in++;
                continue;
            }
            else if (in[0] == '\"')
            {
                // There is some text before the opening double quotes
                syntaxError = true;
                break;
            }
        }

        // copy char to buffer and tokenize
        if (in[0] == ',')
        {
            // Serial.println("param token");
            if (doubleQuotes)
            {
                // Missing closing double quotes
                syntaxError = true;
                break;
            }
            else
            {
                // End of this parameter
                buffer[l++] = '\0';
                paramList.push_back(currentParam);
                // Serial.printf("Prev param: %s\n", currentParam);
                currentParam = (buffer + l);
                in++;
            }
        }
        else
        {
            buffer[l++] = in[0];
            in++;
        }
    } // end-while

    // check for syntax errors or missing double quotes in last parameter
    if (syntaxError || doubleQuotes)
    {
        free(buffer);
        lastATParsingResult = AT_PR_ILL_FORMED_STRING;
        printATResultResponse(AT_RESULT_ERROR);
        return nullptr;
    }

    // check for buffer overflow
    if (l >= bufferSize)
    {
        free(buffer);
        lastATParsingResult = AT_PR_SET_OVERFLOW;
        printATResultResponse(AT_RESULT_ERROR);
        return nullptr;
    }

    // Add the last parameter
    buffer[l] = '\0';
    paramList.push_back(currentParam);
    // Serial.printf("Last param: %s\n", currentParam);

    // Invoke callback
    NuATCommandResult_t response;
    try
    {
        response = pATCmdCallbacks->onSet(commandId, paramList);
    }
    catch (...)
    {
        response = AT_RESULT_ERROR;
    }
    free(buffer);
    printATResultResponse(response);
    return followingCommand(in, response);
}

//-----------------------------------------------------------------------------
// Buffer size
//-----------------------------------------------------------------------------

void NuCommandParser::setBufferSize(size_t size)
{
    if (size < 5)
        // absolute minimum
        size = 5;
    bufferSize = size;
}

//-----------------------------------------------------------------------------
// Printing
//-----------------------------------------------------------------------------

void NuCommandParser::printATResultResponse(const NuATCommandResult_t response)
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

/******************************************************************************
 *
 * SHELL COMMANDS
 *
 ******************************************************************************/

//-----------------------------------------------------------------------------
// Parsing machinery
//-----------------------------------------------------------------------------

void NuCommandParser::parseShellCommandLine(const char *in)
{

    int cmdResult = 0;
    if (pShellCmdCallbacks == nullptr)
    {
        // No callbacks: nothing to do here
        lastShellParsingResult = SHELL_PR_NO_CALLBACKS;
    }
    else
    {
        in = ignoreShellSeparator(in); // remove leading spaces (if any)
        if (in)
        {
            size_t tmpBufferSize = bufferSize; // Thread safety
            char *buffer = (char *)malloc(tmpBufferSize);
            if (buffer)
            {
                lastShellParsingResult = SHELL_PR_OK;
                size_t usedBytes = 0;
                NuShellCommand_t commandLine;
                do
                {
                    commandLine.push_back(buffer + usedBytes);
                    in = parseShellNext(buffer, in, tmpBufferSize, usedBytes);
                } while (in);
                if (lastShellParsingResult == SHELL_PR_OK)
                    try
                    {
                        pShellCmdCallbacks->onExecute(commandLine);
                    }
                    catch (...)
                    {
                    };
                free(buffer);
            }
            else
                // Not enough memory for the buffer
                lastShellParsingResult = SHELL_PR_NO_HEAP;
        }
        else
            // Command line is empty
            lastShellParsingResult = SHELL_PR_NO_COMMAND;

        if (lastShellParsingResult != SHELL_PR_OK)
            try
            {
                pShellCmdCallbacks->onParseError(lastShellParsingResult);
            }
            catch (...)
            {
            };
    }
}

const char *NuCommandParser::parseShellNext(char *dest, const char *in, size_t bufferSize, size_t &usedBytes)
{
    if (!in)
    {
        // should not enter here
        lastShellParsingResult = SHELL_PR_NO_COMMAND;
        in = nullptr;
    }
    else if (usedBytes >= bufferSize)
    {
        // Buffer overflow
        lastShellParsingResult = SHELL_PR_BUFFER_OVERFLOW;
        in = nullptr;
    }
    else // (in && usedBytes<bufferSize)
    {
        if (in[0] == '\"')
        {
            // quoted input
            in++;
            bool quotes = true;
            while ((usedBytes < bufferSize) && (in[0] >= ' '))
            {
                if (in[0] == '\"')
                {
                    in++;
                    if (in[0] != '\"')
                    {
                        // closing double quotes
                        quotes = false;
                        break;
                    }
                }
                dest[usedBytes++] = in[0];
                in++;
            }
            if (quotes || ((in[0] > ' ') && (usedBytes < bufferSize)))
            {
                // syntax error: text after closing double quotes or no closing double quotes
                lastShellParsingResult = SHELL_PR_ILL_FORMED_STRING;
                return nullptr;
            }
        }
        else
        {
            // non-quoted input
            while ((usedBytes < bufferSize) && (in[0] > ' '))
            {
                dest[usedBytes++] = in[0];
                in++;
            }
        }
        dest[usedBytes++] = '\0';
    }
    return ignoreShellSeparator(in);
}

const char *NuCommandParser::ignoreShellSeparator(const char *in)
{
    if (in)
    {
        while (in[0] == ' ')
            in++;
        if (in[0] < ' ')
            in = nullptr;
    }
    return in;
}