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
    char cmd[8];
    int id = 0;
    bool bExecute = false;
    bool bWrite = false;
    bool bRead = false;
    bool bTest = false;

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
        return id;
    };

    virtual NuATCommandResult_t onExecute(int commandId) override
    {
        bExecute = true;
        return AT_RESULT_OK;
    };

    virtual NuATCommandResult_t onSet(int commandId, NuATCommandParameters_t &parameters) override
    {
        bWrite = true;
        return AT_RESULT_OK;
    };

    virtual NuATCommandResult_t onQuery(int commandId) override
    {
        bRead = true;
        return AT_RESULT_OK;
    };

    virtual void onTest(int commandId) { bTest = true; };

public:
    void reset()
    {
        lastResponse = AT_RESULT_OK;
        memset(cmd, 8, 0);
        id = -1;
        bExecute = false;
        bWrite = false;
        bRead = false;
        bTest = false;
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

template<typename T>
void assert_eq(T expected, T actual, int testID, bool eqOrNot = true)
{
    bool test = (expected == actual);
    if (test != eqOrNot)
    {
        Serial.printf("--Test #%d failure: expected %d, found %d\n", testID, expected, actual);
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

void Test_ActionExecute(char commandLine[])
{
    Serial.printf("--Test #%d. EXECUTE %s\n", testNumber++, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = tester.bExecute && !tester.bRead && !tester.bWrite && !tester.bTest;
    if (!test)
        Serial.println("--Test failed.");
    testNumber++;
}

void Test_ActionQuery(char commandLine[])
{
    Serial.printf("--Test #%d. QUERY %s\n", testNumber++, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = !tester.bExecute && tester.bRead && !tester.bWrite && !tester.bTest;
    if (!test)
        Serial.println("--Test failed.");
    testNumber++;
}

void Test_ActionSet(char commandLine[])
{
    Serial.printf("--Test #%d. SET %s\n", testNumber++, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = !tester.bExecute && !tester.bRead && tester.bWrite && !tester.bTest;
    if (!test)
        Serial.println("--Test failed.");
    testNumber++;
}

void Test_ActionTest(char commandLine[])
{
    Serial.printf("--Test #%d. TEST ACTION %s\n", testNumber++, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = !tester.bExecute && !tester.bRead && !tester.bWrite && tester.bTest;
    if (!test)
        Serial.println("--Test failed.");
    testNumber++;
}

void Test_NoAction(char commandLine[])
{
    Serial.printf("--Test #%d. NO ACTION %s\n", testNumber++, commandLine);
    tester.reset();
    tester.test(commandLine);
    bool test = !tester.bExecute && !tester.bRead && !tester.bWrite && !tester.bTest;
    if (!test)
        Serial.println("--Test failed.");
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

    // tester.setATCallbacks(tester);

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
    Test_parsing("AT&F=\n", AT_RESULT_ERROR);
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
    Test_NoAction("AT&FFF\n");
    Test_NoAction("AT+F/\n");
    Test_ActionExecute("AT&F\n");
    Test_ActionQuery("AT&F?\n");
    Test_ActionSet("AT&F=99\n");

    // Test #30
    Test_ActionTest("AT&F=?\n");
    Test_NoAction("AT&+F/;&G\n");
    Test_ActionExecute("AT&G=?;&F\n");
    Test_ActionQuery("AT&G;&F?\n");
    Test_ActionSet("AT&F;G=99\n");
    Test_ActionTest("AT&F=1&G=?\n");

    Serial.println("*****************************************");
    Serial.println("END");
    Serial.println("*****************************************");
}

void loop()
{
    delay(30000);
}