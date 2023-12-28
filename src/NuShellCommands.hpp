/**
 * @file NuShellCommands.hpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-27
 * @brief Shell command processor using the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */
#ifndef __NUSHELLCOMMANDS_HPP__
#define __NUSHELLCOMMANDS_HPP__

#include "NuS.hpp"
#include "NuShellCmdParser.hpp"

/**
 * @brief Execute AT commands received thanks to the Nordic UART Service
 *
 */
class NuShellCommandProcessor : public NordicUARTService, public NuShellCommandParser
{
public:
    // Singleton pattern

    NuShellCommandProcessor(const NuShellCommandProcessor &) = delete;
    void operator=(NuShellCommandProcessor const &) = delete;

    /**
     * @brief Get the instance of the NuShellCommandProcessor
     *
     * @note No need to use. Use `NuShellCommands` instead.
     *
     * @return NuShellCommandProcessor&
     */
    static NuShellCommandProcessor &getInstance()
    {
        static NuShellCommandProcessor instance;
        return instance;
    };

public:
    // Overriden Methods
    virtual void onWrite(NimBLECharacteristic *pCharacteristic) override;

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
    NuShellCommandProcessor(){};
};

/**
 * @brief Singleton instance of the NuATCommandProcessor class
 *
 */
extern NuShellCommandProcessor &NuShellCommands;

#endif