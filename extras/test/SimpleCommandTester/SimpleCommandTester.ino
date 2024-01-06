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
bool testCallbackExecuted = false;

#define TEST_COMMAND "testCmd"
#define TEST_COMMAND_IGNORE_CASE "TESTcMD"

void initializeTester()
{
    tester
        .on(TEST_COMMAND, [](NuCommandLine_t &commandLine)
            { testCallbackExecuted = true; })
        .onUnknown([](NuCommandLine_t &commandLine)
                   {
                lastParsingResult = CLI_PR_OK;
                if (testExecution)
                {
                    if (commandLine.size() == expectedCmdLine.size())
                    {
                        for (int index = 0; index < commandLine.size(); index++)
                        {
                            std::string expected = expectedCmdLine[index];
                            std::string found = commandLine[index];
                            bool test = (expected.compare(found) != 0);
                            if (test)
                            {
                                Serial.printf(" --Failure at string index #%d. Expected: [%s] Found: [%s]\n", index, expected.c_str(), found.c_str());
                            }
                        }
                    }
                    else
                    {
                        Serial.printf(" --Failure at strings count. Expected: %d Found: %d\n", expectedCmdLine.size(), commandLine.size());
                    }
                } })
        .onParseError([&lastParsingResult](NuCLIParsingResult_t result, size_t byteIndex)
                      {
                lastParsingResult = result;
                if (testParseCallback)
                {
                    Serial.printf("onParseError(%d)", result);
                } });
}

void reset()
{
    testExecution = false;
    testParseCallback = false;
    testCallbackExecuted = false;
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
        Serial.printf("Parsing failure at [%s]. Expected code: %d. Found code: %d\n", line.c_str(), parsingResult, lastParsingResult);
    }
}

void Test_execution(std::string line)
{
    reset();
    testExecution = true;
    Serial.printf("--Executing: [%s]\n", line.c_str());
    tester.execute(line);
    if (lastParsingResult != CLI_PR_OK)
    {
        Serial.printf("Failure. Unexpected parsing result code: %d\n", lastParsingResult);
    }
}

void Test_callback(std::string line, bool expected)
{
    reset();
    tester.execute(line);
    if (expected != testCallbackExecuted)
    {
        Serial.printf("Callback execution failure at [%s]. Expected: %d. Found: %d\n", line.c_str(), expected, testCallbackExecuted);
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
    expectedCmdLine.push_back(u8"áéí");
    Test_execution(u8"áéí");
    expectedCmdLine.push_back(u8"áéí");
    Test_execution(u8"áéí\náéí\n");

    expectedCmdLine.clear();
    expectedCmdLine.push_back("abc");

    Test_execution("abc");
    Test_execution("abc\n");
    Test_execution("   abc\n");
    Test_execution("   abc   ");

    expectedCmdLine.push_back("cde");

    Test_execution("abc cde");
    Test_execution("abc     cde    \n");
    Test_execution("abc  \"cde\"  ");
    Test_execution("  \"abc\" \"cde\"  ");
    Test_execution("\"abc\" cde");
    Test_execution("abc\ncde");

    expectedCmdLine.push_back("123 456");

    Test_execution("abc cde \"123 456\" \n");

    expectedCmdLine.push_back(".\"xyz\".");

    Test_execution("abc cde \"123 456\" \".\"\"xyz\"\".\"");

    tester.caseSensitive(true);
    Test_callback(TEST_COMMAND, true);
    Test_callback(TEST_COMMAND_IGNORE_CASE, false);
    tester.caseSensitive(false);
    Test_callback(TEST_COMMAND, true);
    Test_callback(TEST_COMMAND_IGNORE_CASE, true);

    Serial.println("**************************************************");
    Serial.println("END");
    Serial.println("**************************************************");
}

void loop()
{
    delay(30000);
}