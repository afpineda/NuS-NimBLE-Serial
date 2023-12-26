/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-26
 * @brief Simple command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NUSIMPLECOMMANDPARSER_HPP__
#define __NUSIMPLECOMMANDPARSER_HPP__

#include <vector>

/**
 * @brief Parsing state of a received command
 *
 * @note Additional information about parsing for debug or logging purposes
 */
typedef enum
{
    SIMPLE_PR_OK = 0,            /// No Parsing error
    SIMPLE_PR_NO_CALLBACKS,      /// Callbacks not set
    SIMPLE_PR_NO_COMMAND,        /// Command line is empty
    SIMPLE_PR_BUFFER_OVERFLOW,   /// Buffer overflow (command line too long)
    SIMPLE_PR_ILL_FORMED_STRING, /// A string parameter is not properly enclosed between double quotes
    SIMPLE_PR_NO_HEAP            /// Unable to allocate buffer memory
} NuSimpleParsingResult_t;

typedef std::vector<const char *> NuSimpleCommand_t;

/**
 * @brief Custom simple command processing for your application
 *
 * @note Derive a new class to implement your own AT commands
 */
class NuSimpleCommandCallbacks
{
public:
    /**
     * @brief Execute a simple command
     *
     * @param[in] commandLine A sorted list of null-terminated strings as typed in the command line,
     *                        from left to right. The first one should be interpreted as the command name.
     *                        This method is never called with an empty command line, however, it may
     *                        contain empty strings typed as "".
     */
    virtual void onExecute(NuSimpleCommand_t &commandLine) = 0;

    /**
     * @brief Get informed of parsing errors
     *
     * @note Override this method to generate a common response to parsing errors.
     *       Optional.
     *
     * @param[in] parsingResult  Detailed result of command parsing
     */
    virtual void onParseError(NuSimpleParsingResult_t parsingResult){};
};

/**
 * @brief Parse and execute simple commands
 *
 */
class NuSimpleCommandParser
{
public:
    /**
     * @brief Set custom AT command processing callbacks
     *
     * @note Not thread-safe.
     *
     * @param[in] pCallbacks A pointer to your own callbacks. Must
     *            remain valid forever (do not destroy).
     *
     */
    void setSimpleCommandCallbacks(NuATCommandCallbacks *pCallbacks)
    {
        pCmdCallbacks = pCallbacks;
    };

    /**
     * @brief Size of the parsing buffer
     *
     * @note Should be enough to hold a single command line. On buffer
     *       overflow, the command line is ignored.
     *
     * @note Default size is 64 bytes. Thread-safe.
     *
     * @param[in] size Size in bytes
     */
    void setBufferSize(size_t size) {
        bufferSize = size;
    };

public:
    /**
     * @brief Check this attribute to know why parsing failed (or not)
     *        on the last received command
     *
     * @note Exposed for testing, mainly. Do not write.
     */
    NuATParsingResult_t lastParsingResult = SIMPLE_PR_OK;

private:
    NuATCommandCallbacks *pCmdCallbacks = nullptr;
    size_t bufferSize = 64;

    const char *ignoreSeparator(const char *in);
    const char *parseNext(char *dest, const char *in, size_t bufferSize, size_t &usedBytes);

protected:
    void parseCommandLine(const char *in);
};

#endif