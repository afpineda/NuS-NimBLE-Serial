/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-26
 * @brief Simple command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NUSHELLCMDPARSER_HPP__
#define __NUSHELLCMDPARSER_HPP__

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
} NuShellParsingResult_t;

typedef std::vector<const char *> NuShellCommand_t;

/**
 * @brief Custom simple command processing for your application
 *
 * @note Derive a new class to implement your own AT commands
 */
class NuShellCommandCallbacks
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
    virtual void onExecute(NuShellCommand_t &commandLine) = 0;

    /**
     * @brief Get informed of parsing errors
     *
     * @note Override this method to generate a common response to parsing errors.
     *       Optional.
     *
     * @param[in] parsingResult  Detailed result of command parsing
     */
    virtual void onParseError(NuShellParsingResult_t parsingResult){};
};

/**
 * @brief Parse and execute simple commands
 *
 */
class NuShellCommandParser
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
    void setShellCommandCallbacks(NuShellCommandCallbacks *pCallbacks)
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
    void setBufferSize(size_t size)
    {
        bufferSize = size;
    };

    /**
     * @brief Automatically convert command names to upper case
     *
     * @note When set to `true`, `commandLine[0]` will be converted
     *       to upper case before calling onExecute(). This simplifies
     *       command recognition.
     *
     * @param yesOrNo True to convert to upper case, false otherwise.
     */
    void forceUpperCaseCommandName(bool yesOrNo = true)
    {
        bForceUpperCaseCommandName = yesOrNo;
    };

public:
    /**
     * @brief Check this attribute to know why parsing failed (or not)
     *        on the last received command
     *
     * @note Exposed for testing, mainly. Do not write.
     */
    NuShellParsingResult_t lastParsingResult = SIMPLE_PR_OK;

private:
    NuShellCommandCallbacks *pCmdCallbacks = nullptr;
    size_t bufferSize = 64;
    bool bForceUpperCaseCommandName = false;

    const char *ignoreSeparator(const char *in);
    const char *parseNext(char *dest, const char *in, size_t bufferSize, size_t &usedBytes, bool forceUpperCase);

protected:
    void parseCommandLine(const char *in);
};

#endif