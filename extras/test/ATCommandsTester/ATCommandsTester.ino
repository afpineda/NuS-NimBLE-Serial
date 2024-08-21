/**
 * @file ATCommandsTester.ino
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2024-08-20
 * @brief Automated test
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuATParser.hpp"
#include <string.h>

//-----------------------------------------------------------------------------
// MOCK
//-----------------------------------------------------------------------------

class NuATCommandTester : public NuATParser
{
public:
    std::string lastCommand;
    NuATSyntaxError_t lastSyntaxError;
    bool bSyntaxError = false;
    bool bExecute = false;
    bool bWrite = false;
    bool bRead = false;
    bool bTest = false;
    bool bNoCommands = false;
    bool bPrintParams = false;
    bool bPrintCmd = false;

protected:
    virtual void notifyError(
        std::string command,
        NuATSyntaxError_t errorCode) override
    {
        bSyntaxError = true;
        lastSyntaxError = errorCode;
    };

    virtual void doExecute(const uint8_t *in, size_t size) override
    {
        bExecute = true;
        bSyntaxError = false;
        lastCommand.assign((const char *)in, size);
    };

    virtual void doQuery(const uint8_t *in, size_t size) override
    {
        bRead = true;
        bSyntaxError = false;
        lastCommand.assign((const char *)in, size);
    };

    virtual void doTest(const uint8_t *in, size_t size) override
    {
        bTest = true;
        bSyntaxError = false;
        lastCommand.assign((const char *)in, size);
    };

    virtual void doSet(std::string command, NuATCommandParameters_t &params) override
    {
        if (bPrintParams)
        {
            int count = 1;
            // Serial.printf("Count: %d\n",params.size());
            for (std::string param : params)
            {
                param.push_back(0);
                Serial.printf("  Parameter %d: %s\n", count++, param.c_str());
            }
        }
        bWrite = true;
        bSyntaxError = false;
        lastCommand.assign(command);
    };

    virtual void doNotACommandLine(const uint8_t *in, size_t size) override
    {
        bNoCommands = true;
        bSyntaxError = false;
        lastCommand.assign((const char *)in, size);
    };

public:
    virtual void printATResponse(std::string message) override {};

    void reset()
    {
        bExecute = false;
        bWrite = false;
        bRead = false;
        bTest = false;
        bNoCommands = false;
        bSyntaxError = false;
        bPrintParams = false;
        bPrintCmd = false;
        lastCommand.clear();
    };

} tester;

class NuATCommandTester2 : public NuATParser
{
public:
    virtual void printATResponse(std::string message) override {};
} tester2;

NuATCommandResult_t testOkCallback(NuATCommandParameters_t &params)
{
    Serial.printf("  AT command callback\n");
    return NuATCommandResult_t::AT_RESULT_OK;
}

void testErrorCallback(const std::string text, NuATSyntaxError_t errorCode)
{
    if (errorCode == NuATSyntaxError_t::AT_ERR_NO_CALLBACK)
        Serial.printf("  No callback for %s\n", text.c_str());
    else
        Serial.printf("  Syntax error in %s\n", text.c_str());
}

void testNoCommandLineCallback(const uint8_t *text, size_t size)
{
    Serial.printf("  Not an AT command line\n");
}

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
    }
}

//-----------------------------------------------------------------------------
// Test macros
//-----------------------------------------------------------------------------

int testNumber = 1;

void Test_parsing(char commandLine[], NuATSyntaxError_t expectedError)
{
    tester.reset();
    Serial.printf("--Test #%d. Parsing command line: %s\n", testNumber, commandLine);
    tester.execute(commandLine);
    if (tester.bNoCommands)
        Serial.printf("  --Failure. Parsed as non-AT text.\n");
    if (!tester.bSyntaxError)
        Serial.printf("  --Failure. Expected syntax error not found.\n");
    else
        assert_eq<NuATSyntaxError_t>(expectedError, tester.lastSyntaxError, testNumber);
    testNumber++;
}

void Test_parsing(char commandLine[])
{
    tester.reset();
    Serial.printf("--Test #%d. Parsing command line: %s\n", testNumber, commandLine);
    tester.execute(commandLine);
    if (tester.bNoCommands)
        Serial.printf("  --Failure. Parsed as non-AT text.\n");
    if (tester.bSyntaxError)
        Serial.printf("  --Failure. Unexpected syntax error %d.\n", tester.lastSyntaxError);
    testNumber++;
}

void Test_noCommand(char commandLine[])
{
    tester.reset();
    Serial.printf("--Test #%d. Parsing command line: %s\n", testNumber, commandLine);
    tester.execute(commandLine);
    if (!tester.bNoCommands)
        Serial.printf("  --Failure. Expected a non-AT command line.\n");
    testNumber++;
}

void Test_actionFlags(char commandLine[], bool mustExecute, bool mustRead, bool mustWrite, bool mustTest)
{
    Serial.printf("--Test #%d. Callbacks for %s\n", testNumber, commandLine);
    tester.reset();
    tester.execute(commandLine);
    if (tester.bNoCommands)
        Serial.printf("  --Failure. Parsed as non-AT text.\n");
    else
    {
        bool test = (tester.bExecute == mustExecute) && (tester.bRead == mustRead) && (tester.bWrite == mustWrite) && (tester.bTest == mustTest);
        if (!test)
        {
            Serial.printf("  --Failure (execute,read,write,test). Expected (%d,%d,%d,%d). Found (%d,%d,%d,%d)\n",
                          mustExecute, mustRead, mustWrite, mustTest,
                          tester.bExecute, tester.bRead, tester.bWrite, tester.bTest);
            if (tester.bSyntaxError)
                Serial.printf("  --There is a syntax error %d.\n", tester.lastSyntaxError);
        }
    }
    testNumber++;
}

void Test_setActionParameters(char commandLine[])
{
    tester.reset();
    tester.bPrintParams = true;
    Serial.printf("-- Test #%d. Check parameters for %s\n", testNumber, commandLine);
    tester.execute(commandLine);
    testNumber++;
}

void Test_callbacks(char commandLine[])
{
    Serial.printf("-- Test #%d. Check execution for %s\n", testNumber, commandLine);
    tester2.execute(commandLine);
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

    tester.stopOnFirstFailure(false);

    // Test #1
    tester.allowLowerCase(false);
    Test_parsing("AT\n");
    Test_noCommand("at\n");
    tester.allowLowerCase(true);
    Test_parsing("at\n");
    tester.allowLowerCase(false);
    Test_noCommand("at");

    Test_noCommand("\n");
    Test_parsing("ATn\n", NuATSyntaxError_t::AT_ERR_INVALID_PREFIX);
    Test_parsing("AT+\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);
    Test_parsing("AT&\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);
    Test_parsing("AT+F;;", NuATSyntaxError_t::AT_ERR_EMPTY_COMMAND);

    // Test #10
    Test_parsing("AT&F\n");
    Test_parsing("AT&FF\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);
    Test_parsing("AT+F\n");
    Test_parsing("AT+FFFF\n");
    Test_parsing("AT&+F\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);

    Test_parsing("AT&F=\n");
    Test_parsing("AT+FFFF=\"value\"\n");
    Test_parsing("AT+FFFF=\"value\",1\n");
    Test_parsing("AT+FFFF=\"value\",\n");
    Test_parsing("AT+FFFF=,1\n");

    // Test #20
    Test_parsing("AT+FFFF=,,,\n");
    Test_parsing("AT+F?\n");
    Test_parsing("AT+F=?\n");
    Test_parsing("AT+F/\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);
    Test_parsing("AT+F1F\n", NuATSyntaxError_t::AT_ERR_INVALID_NAME);

    Test_parsing("AT&F;AT&F\n", NuATSyntaxError_t::AT_ERR_INVALID_PREFIX);
    Test_parsing("AT&F;&G=1;&H?\n");
    Test_parsing("AT&F;&G=1;&H?;\n", NuATSyntaxError_t::AT_ERR_EMPTY_COMMAND);
    Test_parsing("AT&F;;&H?\n");
    Test_parsing("AT&F=\"\"\n");

    // Test #30
    Test_parsing("AT&F=error\"string\"\n", NuATSyntaxError_t::AT_ERR_ILL_FORMED_NUMBER);
    Test_parsing("AT&F=\"string\"error\n", NuATSyntaxError_t::AT_ERR_ILL_FORMED_STRING);
    Test_parsing("AT&F=\"error\n", NuATSyntaxError_t::AT_ERR_ILL_FORMED_STRING);
    Test_parsing("AT&F=error\"\n", NuATSyntaxError_t::AT_ERR_ILL_FORMED_NUMBER);
    Test_parsing("AT&F=\"a \\\\ b\"\n");

    Test_parsing("AT&F=\"a \\, b\"\n");
    Test_parsing("AT&F=\"a \\; b\"\n");
    Test_parsing("AT&F=\"a \\\" b\"\n");
    Test_parsing("AT&F=1234567890ABCDEF");
    Test_parsing("AT&F=1234567890abcdef");

    // Test 40
    Test_parsing("AT&F=1234567890 abcdef", NuATSyntaxError_t::AT_ERR_ILL_FORMED_NUMBER);
    Test_parsing("AT&F=\"\\41\\41\""); // Equal to "AA"
    Test_parsing("AT&F=\" \\\" \"");
    Test_parsing("AT&F=\"param\nATnotACommand\"");
    Test_parsing("AT\nAT&F");

    Test_parsing("AT&F;&G\nAT&F;&G");
    tester.allowLowerCase(true);
    Test_parsing("AT+F?;+f?\n");

    tester.stopOnFirstFailure(true);
    Test_actionFlags("AT&FFF\n", false, false, false, false);
    Test_actionFlags("AT+F/\n", false, false, false, false);
    Test_actionFlags("AT&F\n", true, false, false, false);

    // Test #50
    Test_actionFlags("AT&F?\n", false, true, false, false);
    Test_actionFlags("AT&F=99\n", false, false, true, false);
    Test_actionFlags("AT&F=?\n", false, false, false, true);
    Test_actionFlags("AT&+F/;&G\n", false, false, false, false);
    Test_actionFlags("AT&G=?;&F\n", true, false, false, true);

    Test_actionFlags("AT&G;&F?\n", true, true, false, false);
    Test_actionFlags("AT&F;&G=99\n", true, false, true, false);
    Test_actionFlags("AT&F=1;&G=?\n", false, false, true, true);

    Serial.println("*****************************************");
    Serial.println(" Non-Automated test (check visually)     ");
    Serial.println("*****************************************");

    // Test #58

    Test_setActionParameters("AT&F=\"value\"\n");
    Test_setActionParameters("AT&F=1,2,3,4,5\n");

    // Test #60
    Test_setActionParameters("AT&F=\"a \\\\ b\"\n");
    Test_setActionParameters("AT&F=\"a \\, b\"\n");
    Test_setActionParameters("AT&F=\"a \\; b\"\n");
    Test_setActionParameters("AT&F=\"a \\\" b\"\n");
    Test_setActionParameters("AT&F=\"a \\\n b\"\n");

    Test_setActionParameters("AT&F=\"\\41\\42\"\n"); // Equals to "AB"
    Test_setActionParameters("AT&F=\"\\4\\5\"\n");   // Equals to "45"

    tester2.stopOnFirstFailure(false);
    tester2.allowLowerCase(false);

    // Serial.printf("(Installing callbacks)\n");
    tester2.onError(testErrorCallback)
        .onExecute("a", testOkCallback)
        .onSet("a", testOkCallback)
        .onQuery("a", testOkCallback)
        .onTest("a", testOkCallback)
        .onNotACommandLine(testNoCommandLineCallback);
    // Serial.printf("(Callbacks installed)\n");

    // Test #67
    Test_callbacks("AT&A;&A=?\nAT&A=1234;&A?");
    Test_callbacks("AT&A;&B");
    Test_callbacks("AT&A;ERROR;&A?");

    // Test #70
    Test_callbacks("ATERROR;&A");
    Test_callbacks("not an at command line");
    Test_callbacks("AT&A\nnot an at command line");
    tester2.allowLowerCase(true);
    Test_callbacks("AT&a");

    // Test #74

    Serial.println("*****************************************");
    Serial.println("END");
    Serial.println("*****************************************");
}

void loop()
{
    delay(30000);
}