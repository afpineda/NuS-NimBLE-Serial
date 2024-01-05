/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-27
 * @brief Automated test
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include <string.h>
#include "NuCLIParser.hpp"

//-----------------------------------------------------------------------------
// MOCK
//-----------------------------------------------------------------------------

NuCLIParser tester;
NuCLIParsingResult_t lastParsingResult;
bool testExecution = false;
bool testParseCallback = false;
NuCommandLine_t expectedCmdLine;

void initializeTester()
{
    tester
        .onUnknown(
            [](NuCommandLine_t &commandLine)
            {
                if (testExecution)
                {
                    if (commandLine.size() == expectedCmdLine.size())
                    {
                        for (int index = 0; index < commandLine.size(); index++)
                        {
                            std::string expected = expectedCmdLine[index];
                            std::string found = commandLine[index];
                            bool test = (expected == found);
                            if (test)
                            {
                                Serial.printf(" --Failure at string index #%d. Expected: %s Found: %s\n", index, expected.c_str(), found.c_str());
                            }
                        }
                    }
                    else
                    {
                        Serial.printf(" --Failure at strings count. Expected: %d Found: %d\n", expectedCmdLine.size(), commandLine.size());
                    }
                }
            })
        .onParseError(
            [&lastParsingResult](NuCLIParsingResult_t result, size_t byteIndex)
            {
                lastParsingResult = result;
                if (testParseCallback)
                {
                    Serial.printf("onParseError(%d)", result);
                }
            });
}

void reset()
{
    testExecution = false;
    testParseCallback = false;
}

//-----------------------------------------------------------------------------
// Test macros
//-----------------------------------------------------------------------------

void Test_parsingResult(std::string line, NuCLIParsingResult_t parsingResult)
{
    reset();
    tester.execute(line);
    if (lastParsingResult != parsingResult)
    {
        Serial.printf("Parsing failure at %s. Expected code: %d. Found code: %d\n", line, parsingResult, lastParsingResult);
    }
}

void Test_execution(std::string line)
{
    reset();
    testExecution = true;
    Serial.printf("--Executing: %s\n", line.c_str());
    tester.execute(line);
    if (lastParsingResult != CLI_PR_OK)
    {
        Serial.printf("Failure. Unexpected parsing result code: %d\n", lastParsingResult);
    }
}

//-----------------------------------------------------------------------------
// Arduino entry points
//-----------------------------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("**************************************************");
    Serial.println(" Automated test for simple command processor ");
    Serial.println("**************************************************");
    initializeTester();

    Test_parsingResult("", CLI_PR_NO_COMMAND);
    Test_parsingResult("   \n", CLI_PR_NO_COMMAND);
    Test_parsingResult("  abc de", CLI_PR_OK);
    Test_parsingResult("abc de   \n", CLI_PR_OK);
    Test_parsingResult("   abc    de   \n", CLI_PR_OK);
    Test_parsingResult(" \"abc\" ", CLI_PR_OK);
    Test_parsingResult(" abc\"abc ", CLI_PR_OK);
    Test_parsingResult("\"abc\"", CLI_PR_OK);
    Test_parsingResult("\"abc\"\n", CLI_PR_OK);
    Test_parsingResult(" \"abc \"\"def\"\" abc \" ", CLI_PR_OK);
    Test_parsingResult("\"unterminated string\n", CLI_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"unterminated \"\" string\n", CLI_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text\"after quotes\n", CLI_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text\"\"\"after quotes\n", CLI_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text \"\"___\"\" \"after quotes\n", CLI_PR_ILL_FORMED_STRING);

    expectedCmdLine.clear();
    expectedCmdLine.push_back("abc");

    Test_execution("abc");
    Test_execution("abc\n");
    Test_execution("   abc\n");
    Test_execution("   abc   ");
    Test_execution("abc \n  cde");

    expectedCmdLine.push_back("cde");

    Test_execution("abc cde");
    Test_execution("abc     cde    \n");
    Test_execution("abc  \"cde\"  ");
    Test_execution("  \"abc\" \"cde\"  ");
    Test_execution("\"abc\" cde");

    expectedCmdLine.push_back("123 456");

    Test_execution("abc cde \"123 456\" \n");

    expectedCmdLine.push_back(".\"xyz\".");

    Test_execution("abc cde \"123 456\" \".\"\"xyz\"\".\"");

    Serial.println("**************************************************");
    Serial.println("END");
    Serial.println("**************************************************");
}

void loop()
{
    delay(30000);
}