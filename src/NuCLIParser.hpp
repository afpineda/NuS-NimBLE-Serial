/**
 * @file NuCLIParser.hpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-26
 * @brief Simple command parser
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NU_CLI_PARSER_HPP__
#define __NU_CLI_PARSER_HPP__

#include <vector>
#include <cstring>

/**
 * @brief Parsing state of a received command
 *
 * @note Additional information about parsing for debug or logging purposes
 */
typedef enum
{
    /** No Parsing error */
    CLI_PR_OK = 0,
    /** Callbacks not set */
    CLI_PR_NO_CALLBACKS,
    /** Command line is empty */
    CLI_PR_NO_COMMAND,
    /** A string parameter is not properly enclosed between double quotes */
    CLI_PR_ILL_FORMED_STRING

} NuCLIParsingResult_t;

/**
 * @brief Parsed strings in a command line, from left to right
 *
 */
typedef std::vector<std::string> NuCommandLine_t;

/**
 * @brief Callback to execute for a parsed command line
 *
 */
typedef void (*NuCLICommandCallback_t)(NuCommandLine_t &);

/**
 * @brief Callback to execute in case of parsing errors
 *
 */
typedef void (*NuCLIParseErrorCallback_t)(NuCLIParsingResult_t, size_t);

/**
 * @brief Parse and execute simple commands
 *
 */
class NuCLIParser
{
public:
    /**
     * @brief Enable or disable case-sensitive command names
     *
     * @param[in] yesOrNo True for case-sensitive. False, otherwise.
     * @return true Previously, case-sensitive.
     * @return false Previously, case-insensitive.
     */
    bool caseSensitive(bool yesOrNo)
    {
        bool result = bCaseSensitive;
        bCaseSensitive = yesOrNo;
        return result;
    };

    /**
     * @brief Set a callback for a command name
     *
     * @note If you set two or more callbacks for the same command name,
     *       just the first will be executed, so don't do that.
     *
     * @param[in] commandName Command name
     * @param[in] callback Function to execute
     *
     * @return NuCLIParser& This instance. Used to chain calls.
     */
    NuCLIParser &on(const std::string commandName, NuCLICommandCallback_t callback);

    /**
     * @brief Set a callback for unknown commands
     *
     * @param[in] callback Function to execute
     *
     * @return NuCLIParser& This instance. Used to chain calls.
     */
    NuCLIParser &onUnknown(NuCLICommandCallback_t callback)
    {
        cbUnknown = callback;
        return *this;
    };

    /**
     * @brief Set a callback for parsing errors
     *
     * @param[in] callback Function to execute
     * @return NuCLIParser& This instance. Used to chain calls.
     */
    NuCLIParser &onParseError(NuCLIParseErrorCallback_t callback)
    {
        cbParseError = callback;
        return *this;
    };

    /**
     * @brief Execute the given command line
     *
     * @param commandLine Text of command line
     * @param size size of the command line
     */
    void execute(const uint8_t *commandLine, size_t size);

    /**
     * @brief Execute the given command line
     *
     * @param commandLine Text of command line
     */
    void execute(std::string commandLine)
    {
        execute((const uint8_t *)commandLine.data(), commandLine.length());
    };

    /**
     * @brief Execute the given command line
     *
     * @param commandLine Text of command line
     */
    void execute(const char *commandLine)
    {
        if (commandLine)
            execute((const uint8_t *)commandLine, strlen(commandLine));
    };

protected:
    static NuCLIParsingResult_t parse(const uint8_t *in, size_t size, size_t &index, NuCommandLine_t &parsedCommandLine);
    static NuCLIParsingResult_t parseNext(const uint8_t *in, size_t size, size_t &index, NuCommandLine_t &parsedCommandLine);
    static void ignoreSeparator(const uint8_t *in, size_t size, size_t &index);
    static bool isSeparator(const uint8_t *in, size_t size, size_t index);

private:
private:
    bool bCaseSensitive = false;
    NuCLIParseErrorCallback_t cbParseError = nullptr;
    NuCLICommandCallback_t cbUnknown = nullptr;
    std::vector<std::string> vsCommandName;
    std::vector<NuCLICommandCallback_t> vcbCommand;
};

#endif