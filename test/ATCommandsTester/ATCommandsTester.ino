/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-19
 * @brief Automated test
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuATCommands.hpp"
#include <string.h>

//-----------------------------------------------------------------------------
// MOCK
//-----------------------------------------------------------------------------

class NuATCommandTester : public NuATCommandParser, NuATCommandCallbacks
{
public:
    NuATCommandResult_t lastResponse;
    int id = 0;
    bool bExecute = false;
    bool bWrite = false;
    bool bRead = false;
    bool bTest = false;
    bool bPrintParams = false;
    bool bPrintCmd = false;

public:
    virtual void printATResponse(const char message[]) override
    {
        Serial.print("\r\n");
        Serial.print(message);
        Serial.print("\r\n");
    };

    virtual void printResultResponse(const NuATCommandResult_t response) override
    {
        lastResponse = response;
    };

    virtual int getATCommandId(const char commandName[]) override
    {
        if (bPrintCmd)
            Serial.printf("Command: %s\n", commandName);
        return id;
    };

    virtual NuATCommandResult_t onExecute(int commandId) override
    {
        bExecute = true;
        return AT_RESULT_OK;
    };

    virtual NuATCommandResult_t onSet(int commandId, NuATCommandParameters_t &parameters) override
    {
        if (bPrintParams)
        {
            int count = 1;
            for (const char *param : parameters)
                Serial.printf("Parameter %d: %s\n", count++, param);
        }
        bWrite = true;
        return AT_RESULT_OK;
    };

    virtual NuATCommandResult_t onQuery(int commandId) override
    {
        bRead = true;
        return AT_RESULT_OK;
    };

    virtual void onTest(int commandId) override
    {
        bTest = true;
    };

public:
    void reset()
    {
        lastResponse = AT_RESULT_OK;
        id = 0;
        bExecute = false;
        bWrite = false;
        bRead = false;
        bTest = false;
        bPrintParams = false;
        bPrintCmd = false;
    };

    NuATCommandResult_t test(const char commandLine[])
    {
        parseCommandLine(commandLine);
        return lastResponse;
    };

    NuATCommandTester()
    {
        setATCallbacks(this);
    };
} tester;

//-----------------------------------------------------------------------------
// ASSERTION utilities
//-----------------------------------------------------------------------------

template <typename T>
void assert_eq(T expected, T actual, int testID, bool eqOrNot = true)
{
    bool test = (expected == actual);
    if (test != eqOrNot)
    {
        Serial.printf("  --Test #%d failure: expected %d, found %d\n", testID, expected, actual);
        Serial.printf("  --Last parsing result: %d\n", tester.lastParsingResult);
    }
}

// void assert_strEq(const char *expected, const char *actual, int testID, bool eqOrNot = true)
// {
//     bool test = (strcmp(expected, actual) == 0);
//     if (test != eqOrNot)
//     {
//         Serial.printf("--Test #%d failure: expected %d, found %d\n", testID, expected, actual);
//     }
// }

//-----------------------------------------------------------------------------
// Test macros
//-----------------------------------------------------------------------------

int testNumber = 1;

void Test_parsing(char commandLine[], NuATCommandResult_t expectedResult)
{
    tester.reset();
    Serial.printf("--Test #%d. Parsing command line: %s\n", testNumber, commandLine);
    NuATCommandResult_t actualResult = tester.test(commandLine);
    assert_eq<NuATCommandResult_t>(expectedResult, actualResult, testNumber);
    testNumber++;
}

void Test_actionFlags(char commandLine[], bool mustExecute, bool mustRead, bool mustWrite, bool mustTest)
{
    Serial.printf("--Test #%d. Callbacks for %s\n", testNumber, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = (tester.bExecute == mustExecute) && (tester.bRead == mustRead) && (tester.bWrite == mustWrite) && (tester.bTest == mustTest);
    if (!test)
    {
        Serial.printf("  --Test failed (execute,read,write,test). Expected (%d,%d,%d,%d). Found (%d,%d,%d,%d)\n",
                      mustExecute, mustRead, mustWrite, mustTest,
                      tester.bExecute, tester.bRead, tester.bWrite, tester.bTest);
        Serial.printf("  --Last parsing result: %d\n", tester.lastParsingResult);
    }
    testNumber++;
}

void Test_setActionParameters(char commandLine[])
{
    tester.reset();
    tester.bPrintParams = true;
    Serial.printf("-- Test #%d. Check parameters for %s\n", testNumber, commandLine);
    tester.test(commandLine);
    testNumber++;
}

void Test_commandName(char commandLine[])
{
    tester.reset();
    tester.bPrintCmd = true;
    Serial.printf("-- Test #%d. Check command names for %s\n", testNumber, commandLine);
    tester.test(commandLine);
    testNumber++;
}

//-----------------------------------------------------------------------------
// Arduino entry points
//-----------------------------------------------------------------------------

void setup()
{
    // Initialize serial monitor
    Serial.begin(115200);
    Serial.println("*****************************************");
    Serial.println(" Automated test for AT command processor ");
    Serial.println("*****************************************");

    // Test #1
    Test_parsing("AT\n", AT_RESULT_OK);
    Test_parsing("ATn\n", AT_RESULT_ERROR);
    Test_parsing("AT+\n", AT_RESULT_ERROR);
    Test_parsing("AT&\n", AT_RESULT_ERROR);
    Test_parsing("AT$n\n", AT_RESULT_ERROR);

    Test_parsing("AT&F\n", AT_RESULT_OK);
    Test_parsing("AT&FF\n", AT_RESULT_ERROR);
    Test_parsing("AT+F\n", AT_RESULT_OK);
    Test_parsing("AT+FFFF\n", AT_RESULT_OK);

    // Test #10
    Test_parsing("AT&+F\n", AT_RESULT_ERROR);
    Test_parsing("AT&F=\n", AT_RESULT_OK);
    Test_parsing("AT+FFFF=\"value\"\n", AT_RESULT_OK);
    Test_parsing("AT+FFFF=\"value\",1\n", AT_RESULT_OK);
    Test_parsing("AT+FFFF=\"value\",\n", AT_RESULT_OK);

    Test_parsing("AT+FFFF=,1\n", AT_RESULT_OK);
    Test_parsing("AT+FFFF=,,,\n", AT_RESULT_OK);
    Test_parsing("AT+F?\n", AT_RESULT_OK);
    Test_parsing("AT+F=?\n", AT_RESULT_OK);
    Test_parsing("AT+F/\n", AT_RESULT_ERROR);

    // Test #20
    Test_parsing("AT+F1F\n", AT_RESULT_ERROR);
    Test_parsing("AT&F;AT&F\n", AT_RESULT_ERROR);
    Test_parsing("AT&F;&G=1;&H?\n", AT_RESULT_OK);
    Test_parsing("AT&F;&G=1;&H?;\n", AT_RESULT_ERROR);
    Test_parsing("AT&F;;&H?\n", AT_RESULT_ERROR);

    Test_parsing("AT&F=\"\"\n", AT_RESULT_OK);
    Test_parsing("AT&F=error\"string\"\n", AT_RESULT_ERROR);
    Test_parsing("AT&F=\"string\"error\n", AT_RESULT_ERROR);
    Test_parsing("AT&F=\"error\n", AT_RESULT_ERROR);
    Test_parsing("AT&F=error\"\n", AT_RESULT_ERROR);

    // Test #30
    Test_parsing("AT&F=\"a \\\\ b\"\n", AT_RESULT_OK);
    Test_parsing("AT&F=\"a \\, b\"\n", AT_RESULT_OK);
    Test_parsing("AT&F=\"a \\; b\"\n", AT_RESULT_OK);
    Test_parsing("AT&F=\"a \\\" b\"\n", AT_RESULT_OK);
    Test_parsing("AT&F=\"too long too long too long too long too long too long\"\n", AT_RESULT_ERROR);

    Test_actionFlags("AT&FFF\n", false, false, false, false);
    Test_actionFlags("AT+F/\n", false, false, false, false);
    Test_actionFlags("AT&F\n", true, false, false, false);
    Test_actionFlags("AT&F?\n", false, true, false, false);
    Test_actionFlags("AT&F=99\n", false, false, true, false);

    // Test #40
    Test_actionFlags("AT&F=?\n", false, false, false, true);
    Test_actionFlags("AT&+F/;&G\n", false, false, false, false);
    Test_actionFlags("AT&G=?;&F\n", true, false, false, true);
    Test_actionFlags("AT&G;&F?\n", true, true, false, false);
    Test_actionFlags("AT&F;&G=99\n", true, false, true, false);

    Test_actionFlags("AT&F=1;&G=?\n", false, false, true, true);

    Serial.println("*****************************************");
    Serial.println(" Non-Automated test (check visually)     ");
    Serial.println("*****************************************");

    // Test #46

    Test_setActionParameters("AT&F=\"value\"\n");
    Test_setActionParameters("AT&F=1,2,3,4,5\n");
    Test_setActionParameters("AT&F=\"a \\\\ b\"\n");
    Test_setActionParameters("AT&F=\"a \\, b\"\n");

    // Test #50
    Test_setActionParameters("AT&F=\"a \\; b\"\n");
    Test_setActionParameters("AT&F=\"a \\\" b\"\n");
    Test_setActionParameters("AT&F=\"a \\\n b\"\n");
    Test_commandName("AT&F;+AB;+ABC;+ABCD\n");

    Serial.println("*****************************************");
    Serial.println("END");
    Serial.println("*****************************************");
}

void loop()
{
    delay(30000);
}