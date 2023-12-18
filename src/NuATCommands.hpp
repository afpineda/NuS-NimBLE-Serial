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
 * @brief Result of AT command execution
 *
 */
typedef enum
{
    AT_RESULT_INVALID_PARAM = -2, /// Command not executed due to invalid or missing parameter(s)
    AT_RESULT_FAILURE = -1, /// Command executed with no success
    AT_RESULT_SUCCESS = 0 /// Command executed with success
} NuATCommandResult_t;

/**
 * @brief AT command suffix
 *
 * @note Suffix `=?` (test) is internally handled
 */
typedef enum
{
    AT_SUFFIX_SET, /// Suffix `+`, which means "set" or "write"
    AT_SUFFIX_QUERY, /// Suffix `?` which means "query" or "read"
    AT_SUFFIX_EXECUTE /// No suffix, which means "execute" or "do"
} NuATCommandSuffix_t;

/**
 * @brief A command name, not including any prefix
 *
 * @note Command names are limited to 12 characters by design
 *
 */
typedef const char[13] NuATcommandName_t;

/**
 * @brief Custom AT command processing for your application
 *
 * @note Derive a new class to implement your own AT commands
 */
class NuATCommandCallbacks
{
public:
    /**
     * @brief Custom processing of non-AT command lines
     *
     * @note Optional
     * @param text Received text, not matching an AT command line
     */
    virtual void onNotAnATCommand(char *text){};

    /**
     * @brief Identify valid and invalid command names
     *
     * @note Override this method to know which commands are
     *       supported or not. This is mandatory.
     *
     * @note AT commands should comprise uppercase characters, but this is up
     *       to you. You may return the same commandID for non-case-sensitive
     *       command names. You may also return the same commandID for aliases.
     *
     * @param commandName A null-terminated string containing a command name.
     *        This string does not contain any prefix (`&` or `+`), just the
     *        name. Command names are limited to 12 characters by design and
     *        truncated when exceeded.
     *
     * @return int A negative value if @p commandName is not a valid
     *         AT command. Any positive number as **unique**
     *         identification of a valid command name.
     */
    virtual int getATCommandId(NuATcommandName_t commandName) = 0;

    /**
     * @brief Execute a valid AT command
     *
     * @param commandId Unique identification number as returned by getATCommandId()
     * @param suffix Requested action to perform with this @p commandId over given @p parameters
     * @param parameters A sorted list of null-terminated strings, one for each parameter
     * @return NuATCommandResult_t Proper result of command execution
     */
    virtual NuATCommandResult_t onATCommand(
        int commandId,
        NuATCommandSuffix_t suffix,
        std::vector<const char []> &parameters) = 0;
};

class NuATCommandProcessor : public NordicUARTService
{
public:
    // Singleton pattern

    NuATCommandProcessor(const NuATCommandProcessor &) = delete;
    void operator=(NuATCommandProcessor const &) = delete;

    /**
     * @brief Get the instance of the BLE stream
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

    void onWrite(NimBLECharacteristic *pCharacteristic) override;

public:
    /**
     * @brief Maximum length for any AT command line
     *
     * @note An internal buffer is allocated in the heap for command processing,
     *       so keep a reasonably low maximum length.
     *
     * @note Default maximum length is 42 bytes
     *
     * @param size Maximum length in bytes
     */
    void setMaxLength(size_t size);

    /**
     * @brief Set custom AT command processing callbacks
     *
     * @param pCallbacks A pointer to your own callbacks. Must
     *        remain valid forever (do not destroy);
     */
    void setATCallbacks(NuATCommandCallbacks *pCallbacks);

private:
    NuATCommandCallbacks *executor = nullptr;
    char *buffer;
    size_t bufferSize = 42;
};

/**
 * @brief Singleton instance of the NuATCommandProcessor class
 *
 * @note Use this object as you do with Arduino's `Serial`
 */
extern NuATCommandProcessor &NuSerial;

#endif