/**
 * @file NuCommands.hpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-30
 * @brief AT/shell command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NU_COMMANDS_HPP__
#define __NU_COMMANDS_HPP__

#include "NuS.hpp"
#include "NuCommandParser.hpp"

/**
 * @brief Execute AT/Shell commands received thanks to the Nordic UART Service
 *
 */
class NuCommandProcessor : public NordicUARTService, public NuCommandParser
{
public:
    // Singleton pattern

    NuCommandProcessor(const NuCommandProcessor &) = delete;
    void operator=(NuCommandProcessor const &) = delete;

    /**
     * @brief Get the instance of the NuCommandProcessor
     *
     * @note No need to use. Use `NuCommands` instead.
     *
     * @return NuCommandProcessor&
     */
    static NuCommandProcessor &getInstance()
    {
        static NuCommandProcessor instance;
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
    void setATCommandCallbacks(NuATCommandCallbacks *pCallbacks);

    /**
     * @brief Set custom shell command processing callbacks
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
    void setShellCommandCallbacks(NuShellCommandCallbacks *pCallbacks);

private:
    NuCommandProcessor(){};
};

/**
 * @brief Singleton instance of the NuATCommandProcessor class
 *
 */
extern NuCommandProcessor &NuCommands;

#endif