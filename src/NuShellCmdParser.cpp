/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-26
 * @brief Simple command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <stdlib.h>
#include "NuShellCmdParser.hpp"

//-----------------------------------------------------------------------------
// Parsing machinery
//-----------------------------------------------------------------------------

void NuShellCommandParser::parseCommandLine(const char *in)
{

    int cmdResult = 0;
    if (pCmdCallbacks == nullptr)
    {
        // No callbacks: nothing to do here
        lastParsingResult = SIMPLE_PR_NO_CALLBACKS;
    }
    else
    {
        in = ignoreSeparator(in); // remove leading spaces (if any)
        if (in)
        {
            size_t tmpBufferSize = bufferSize; // Thread safety
            char *buffer = (char *)malloc(tmpBufferSize);
            if (buffer)
            {
                lastParsingResult = SIMPLE_PR_OK;
                size_t usedBytes = 0;
                NuShellCommand_t commandLine;
                do
                {
                    commandLine.push_back(buffer + usedBytes);
                    in = parseNext(buffer, in, tmpBufferSize, usedBytes);
                } while (in);
                if (lastParsingResult == SIMPLE_PR_OK)
                    try
                    {
                        pCmdCallbacks->onExecute(commandLine);
                    }
                    catch (...)
                    {
                    };
                free(buffer);
            }
            else
                // Not enough memory for the buffer
                lastParsingResult = SIMPLE_PR_NO_HEAP;
        }
        else
            // Command line is empty
            lastParsingResult = SIMPLE_PR_NO_COMMAND;

        if (lastParsingResult != SIMPLE_PR_OK)
            try
            {
                pCmdCallbacks->onParseError(lastParsingResult);
            }
            catch (...)
            {
            };
    }
}

const char *NuShellCommandParser::parseNext(char *dest, const char *in, size_t bufferSize, size_t &usedBytes)
{
    if (!in)
    {
        // should not enter here
        lastParsingResult = SIMPLE_PR_NO_COMMAND;
        in = nullptr;
    }
    else if (usedBytes >= bufferSize)
    {
        // Buffer overflow
        lastParsingResult = SIMPLE_PR_BUFFER_OVERFLOW;
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
                    if (in[0] != '\"') {
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
                lastParsingResult = SIMPLE_PR_ILL_FORMED_STRING;
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
    return ignoreSeparator(in);
}

const char *NuShellCommandParser::ignoreSeparator(const char *in)
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
