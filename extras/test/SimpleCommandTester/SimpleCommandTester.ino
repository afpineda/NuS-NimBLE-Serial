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
#include "NuShellCmdParser.hpp"

//-----------------------------------------------------------------------------
// MOCK
//-----------------------------------------------------------------------------

class NuShellCommandTester : public NuShellCommandParser, NuShellCommandCallbacks
{
public:
    virtual void onExecute(NuShellCommand_t &commandLine) override;
    virtual void onParseError(NuShellParsingResult_t parsingResult) override;
    void parseCommandLine(const char *in)
    {
        NuShellCommandParser::parseCommandLine(in);
    };

public:
    void reset();
    NuShellCommand_t expectedCmdLine; // Must contain pointers to static strings
    bool testExecution = false;
    bool testParseCallback = false;

    NuShellCommandTester()
    {
        setShellCommandCallbacks(this);
        setBufferSize(128);
    };
} tester;

void NuShellCommandTester::reset()
{
    testExecution = false;
    testParseCallback = false;
}

void NuShellCommandTester::onExecute(NuShellCommand_t &commandLine)
{
    if (testExecution)
    {
        if (commandLine.size() == expectedCmdLine.size())
        {
            for (int index = 0; index < commandLine.size(); index++)
            {
                const char *expected = expectedCmdLine[index];
                const char *found = commandLine[index];
                bool test = (strcmp(expected, found) != 0);
                if (test)
                {
                    Serial.printf(" --Failure at string index #%d. Expected: %s Found: %s\n", index, expected, found);
                }
            }
        }
        else
        {
            Serial.printf(" --Failure at strings count. Expected: %d Found: %d\n", expectedCmdLine.size(), commandLine.size());
        }
    }
}

void NuShellCommandTester::onParseError(NuShellParsingResult_t parsingResult)
{
    if (testParseCallback)
    {
        Serial.printf("onParseError(%d)", parsingResult);
    }
}

//-----------------------------------------------------------------------------
// Test macros
//-----------------------------------------------------------------------------

void Test_parsingResult(const char *line, NuShellParsingResult_t parsingResult)
{
    tester.reset();
    tester.parseCommandLine(line);
    if (tester.lastParsingResult != parsingResult)
    {
        Serial.printf("Parsing failure at %s. Expected code: %d. Found code: %d\n", line, parsingResult, tester.lastParsingResult);
    }
}

void Test_execution(const char *line)
{
    tester.reset();
    tester.testExecution = true;
    Serial.printf("--Executing: %s\n",line);
    tester.parseCommandLine(line);
    if (tester.lastParsingResult != SIMPLE_PR_OK)
    {
        Serial.printf("Failure. Unexpected parsing result code: %d\n", tester.lastParsingResult);
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

    Test_parsingResult("", SIMPLE_PR_NO_COMMAND);
    Test_parsingResult("   \n", SIMPLE_PR_NO_COMMAND);
    Test_parsingResult("  abc de", SIMPLE_PR_OK);
    Test_parsingResult("abc de   \n", SIMPLE_PR_OK);
    Test_parsingResult("   abc    de   \n", SIMPLE_PR_OK);
    Test_parsingResult(" \"abc\" ", SIMPLE_PR_OK);
    Test_parsingResult(" abc\"abc ", SIMPLE_PR_OK);
    Test_parsingResult("\"abc\"", SIMPLE_PR_OK);
    Test_parsingResult(" \"abc \"\"def\"\" abc \" ", SIMPLE_PR_OK);
    Test_parsingResult("\"unterminated string\n", SIMPLE_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"unterminated \"\" string\n", SIMPLE_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text\"after quotes\n", SIMPLE_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text\"\"\"after quotes\n", SIMPLE_PR_ILL_FORMED_STRING);
    Test_parsingResult("\"text \"\"___\"\" \"after quotes\n", SIMPLE_PR_ILL_FORMED_STRING);

    tester.expectedCmdLine.clear();
    tester.expectedCmdLine.push_back("abc");

    Test_execution("abc");
    Test_execution("abc\n");
    Test_execution("   abc\n");
    Test_execution("   abc   ");
    Test_execution("abc \n  cde");

    tester.expectedCmdLine.push_back("cde");

    Test_execution("abc cde");
    Test_execution("abc     cde    \n");
    Test_execution("abc  \"cde\"  ");
    Test_execution("  \"abc\" \"cde\"  ");
    Test_execution("\"abc\" cde");

    tester.expectedCmdLine.push_back("123 456");

    Test_execution("abc cde \"123 456\" \n");

    tester.expectedCmdLine.push_back(".\"xyz\".");

    Test_execution("abc cde \"123 456\" \".\"\"xyz\"\".\"");

    tester.setBufferSize(5);
    Test_parsingResult("very long command line", SIMPLE_PR_BUFFER_OVERFLOW);


    Serial.println("**************************************************");
    Serial.println("END");
    Serial.println("**************************************************");
}

void loop()
{
    delay(30000);
}