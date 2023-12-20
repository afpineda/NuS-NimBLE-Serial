/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief AT command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */
#ifndef __NUATCOMMANDS_HPP__
#define __NUATCOMMANDS_HPP__

#include "NuS.hpp"
#include <vector>

/**
 * @brief Pseudo-standardized result of AT command execution
 *
 * @note Negative means error and non-negative means success
 */
typedef enum
{
    AT_RESULT_SEND_FAIL = -3,     /// Failure to send a command to a protocol stack
    AT_RESULT_INVALID_PARAM = -2, /// Command not executed due to invalid or missing parameter(s)
    AT_RESULT_ERROR = -1,         /// Command executed with no success
    AT_RESULT_OK = 0,             /// Command executed with success
    AT_RESULT_SEND_OK = 1         /// Command send successfully to a protocol stack but execution pending
} NuATCommandResult_t;

typedef std::vector<const char *> NuATCommandParameters_t;

/**
 * @brief Custom AT command processing for your application
 *
 * @note Derive a new class to implement your own AT commands
 */
class NuATCommandCallbacks
{
public:
    /**
     * @brief Custom processing of non-AT data
     *
     * @note Optional
     *
     * @param text Null-terminated incoming string,
     *             not matching an AT command line.
     */
    virtual void onNonATCommand(const char text[]){};

    /**
     * @brief Identify supported command names
     *
     * @note Override this method to know which commands are
     *       supported or not. This is mandatory.
     *
     * @note AT commands should comprise uppercase characters, but this is up
     *       to you. You may return the same ID for lowercase
     *       command names. You may also return the same ID for aliases.
     *
     * @param commandName A null-terminated string containing a command name.
     *        This string does not contain any prefix (`&` or `+`), just the
     *        name. Length of command names is limited by buffer size.
     *
     * @return int Any negative value if @p commandName is not a supported
     *         AT command. Any positive number as an **unique**
     *         identification (ID) of a supported command name.
     */
    virtual int getATCommandId(const char commandName[]) = 0;

    /**
     * @brief Execute a supported AT command (with no suffix)
     *
     * @param commandId Unique identification number as returned by getATCommandId()
     * @return NuATCommandResult_t Proper result of command execution
     */
    virtual NuATCommandResult_t onExecute(int commandId) = 0;

    /**
     * @brief Execute or set the value given in a supported AT command (with '=' suffix)
     *
     * @param commandId Unique identification number as returned by getATCommandId()
     *
     * @param parameters A sorted list of null-terminated strings, one for each parameter,
     *                   from left to right. Total length of all parameters
     *                   is limited by buffer size.
     *
     * @return NuATCommandResult_t Proper result of command execution
     */
    virtual NuATCommandResult_t onSet(int commandId, NuATCommandParameters_t &parameters) = 0;

    /**
     * @brief Print the value requested in a supported AT command (with '?' suffix)
     *
     * @note Use NuATCommands.printATResponse() to print the requested value.
     *
     * @param commandId Unique identification number as returned by getATCommandId()
     *
     * @param parameters A sorted list of null-terminated strings, one for each parameter
     *
     * @return NuATCommandResult_t Proper result of command execution
     */
    virtual NuATCommandResult_t onQuery(int commandId) = 0;

    /**
     * @brief Print the syntax and parameters of a supported command (with '=?' suffix)
     *
     * @note Optional. Use NuATCommands.printATResponse() to print.
     *
     * @param commandId Unique identification number as returned by getATCommandId()
     */
    virtual void onTest(int commandId){};
};

/**
 * @brief Parse and execute AT commands
 *
 */
class NuATCommandParser
{
public:
    /**
     * @brief Print a message properly formatted as an AT response
     *
     * @note Error and success messages are already managed by this class.
     *       Do not print those messages to avoid misunderstandings.
     *
     * @param message Text to print.
     *                Must not contain the CR+LF sequence of characters.
     */
    virtual void printATResponse(const char message[]) = 0;

    /**
     * @brief Set custom AT command processing callbacks
     *
     * @param pCallbacks A pointer to your own callbacks. Must
     *        remain valid forever (do not destroy).
     *
     */
    void setATCallbacks(NuATCommandCallbacks *pCallbacks)
    {
        pCmdCallbacks = pCallbacks;
    };

    /**
     * @brief Size of the parsing buffer
     *
     * @note An error response will be printed if command names or
     *       command parameters exceed this size
     *
     * @note Default size is 42 bytes
     *
     * @param size Size in bytes
     */
    void setBufferSize(size_t size);

private:
    NuATCommandCallbacks *pCmdCallbacks = nullptr;
    size_t bufferSize = 42;

    virtual const char *parseSingleCommand(const char *in);
    virtual const char *parseAction(const char *in, int commandId);
    virtual const char *parseWriteParameters(const char *in, int commandId);

protected:
    virtual void printResultResponse(const NuATCommandResult_t response);
    void parseCommandLine(const char *in);
};

/**
 * @brief Execute AT commands received thanks to the Nordic UART Service
 *
 */
class NuATCommandProcessor : public NordicUARTService, public NuATCommandParser
{
public:
    // Singleton pattern

    NuATCommandProcessor(const NuATCommandProcessor &) = delete;
    void operator=(NuATCommandProcessor const &) = delete;

    /**
     * @brief Get the instance of the NuATCommandProcessor
     *
     * @note No need to use. Use `NuATCommands` instead.
     *
     * @return NuATCommandProcessor&
     */
    static NuATCommandProcessor &getInstance()
    {
        static NuATCommandProcessor instance;
        return instance;
    };

public:
    // Overriden Methods
    virtual void onWrite(NimBLECharacteristic *pCharacteristic) override;
    virtual void printATResponse(const char message[]) override;

    /**
     * @brief Set custom AT command processing callbacks
     *
     * @note This method should be called before start(). Any way,
     *       you are not allowed to set a new callbacks object while
     *       a peer is connected.
     *
     * @param pCallbacks A pointer to your own callbacks. Must
     *        remain valid forever (do not destroy).
     *
     * @throws std::runtime_error If called while a peer is connected.
     */
    void setATCallbacks(NuATCommandCallbacks *pCallbacks);

private:
    NuATCommandProcessor(){};
};

/**
 * @brief Singleton instance of the NuATCommandProcessor class
 *
 */
extern NuATCommandProcessor &NuSerial;

#endif