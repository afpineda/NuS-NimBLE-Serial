/**
 * @file NuCLIParser.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-26
 * @brief Simple command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

// #include <stdlib.h>
#include <string>
#include "NuCLIParser.hpp"
//#include <iostream>
#include <algorithm>
#include <cctype>
// #include <cwctype>

//-----------------------------------------------------------------------------
// Set callbacks
//-----------------------------------------------------------------------------

NuCLIParser &NuCLIParser::on(const std::string commandName, NuCLICommandCallback_t callback)
{
    if (callback && (commandName.length() > 0))
    {
        vsCommandName.push_back(commandName);
        vcbCommand.push_back(callback);
    }
    return *this;
}

//-----------------------------------------------------------------------------
// Auxiliary
//-----------------------------------------------------------------------------

inline bool caseInsCharCompareN(char a, char b)
{
    return (toupper(a) == toupper(b));
}

// inline bool caseInsCharCompareW(wchar_t a, wchar_t b)
// {
//     return (towupper(a) == towupper(b));
// }

bool caseInsCompare(const std::string &s1, const std::string &s2)
{
    return ((s1.size() == s2.size()) &&
            equal(s1.begin(), s1.end(), s2.begin(), caseInsCharCompareN));
}

// bool caseInsCompare(const wstring &s1, const wstring &s2)
// {
//     return ((s1.size() == s2.size()) &&
//             equal(s1.begin(), s1.end(), s2.begin(), caseInsCharCompareW));
// }

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------

void NuCLIParser::execute(const uint8_t *commandLine, size_t size)
{
    if ((vcbCommand.size() == 0) && (!cbUnknown))
    {
        if (cbParseError)
            cbParseError(CLI_PR_NO_CALLBACKS, 0);
        return;
    }

    NuCommandLine_t parsedCommandLine;
    size_t index = 0;
    NuCLIParsingResult_t parsingResult = parse(commandLine, size, index, parsedCommandLine);
    if (parsingResult == CLI_PR_OK)
    {
        if (parsedCommandLine.size() == 0)
        {
            if (cbParseError)
                cbParseError(CLI_PR_NO_COMMAND, 0);
            return;
        }
        std::string givenCommandName = parsedCommandLine[0];
        for (size_t index = 0; index < vsCommandName.size(); index++)
        {
            std::string candidate = vsCommandName.at(index);
            bool test;
            if (bCaseSensitive)
                test = (candidate.compare(givenCommandName) == 0);
            else
                test = caseInsCompare(givenCommandName, candidate);
            if (test)
            {
                NuCLICommandCallback_t cb = vcbCommand.at(index);
                cb(parsedCommandLine);
                return;
            }
        }
        if (cbUnknown)
            cbUnknown(parsedCommandLine);
    }
    else if (cbParseError)
    {
        cbParseError(parsingResult, index);
    }
}

//-----------------------------------------------------------------------------
// Parse
//-----------------------------------------------------------------------------

NuCLIParsingResult_t NuCLIParser::parse(const uint8_t *in, size_t size, size_t &index, NuCommandLine_t &parsedCommandLine)
{
    NuCLIParsingResult_t result = CLI_PR_OK;
    while ((index < size) && (result == CLI_PR_OK))
    {
        ignoreSeparator(in, size, index);
        result = parseNext(in, size, index, parsedCommandLine);
    }
    return result;
}

NuCLIParsingResult_t NuCLIParser::parseNext(const uint8_t *in, size_t size, size_t &index, NuCommandLine_t &parsedCommandLine)
{
    if (index < size)
    {
        std::string current = "";
        if (in[index] == '\"')
        {
            // Quoted string
            index++;
            bool openString = true;
            while ((index < size) && openString)
            {
                if (in[index] == '\"')
                {
                    index++;
                    if ((index < size) && (in[index] == '\"'))
                    {
                        // Escaped double quotes
                        current += '\"';
                        index++;
                    }
                    else
                        // Closing double quotes
                        openString = false;
                }
                else
                    current += in[index++];
            }
            if (openString || !isSeparator(in, size, index))
            {
                // No closing double quotes or text after closing double quotes
                return CLI_PR_ILL_FORMED_STRING;
            }
        }
        else
        {
            // Unquoted string
            while (!isSeparator(in, size, index))
            {
                current += in[index++];
            }
        }
        parsedCommandLine.push_back(current);
    }
    return CLI_PR_OK;
}

bool NuCLIParser::isSeparator(const uint8_t *in, size_t size, size_t index)
{
    if (index < size)
        return ((in[index] == ' ') || (in[index] == '\r') || (in[index] == '\n'));
    else
        return true;
}

void NuCLIParser::ignoreSeparator(const uint8_t *in, size_t size, size_t &index)
{
    while ((index < size) && ((in[index] == ' ') || (in[index] == '\r') || (in[index] == '\n')))
        index++;
}