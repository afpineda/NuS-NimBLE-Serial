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
#include "NuCLIParser.hpp"

/**
 * @brief Execute shell commands received thanks to the Nordic UART Service
 *
 */
class NuShellCommandProcessor : public NordicUARTService, public NuCLIParser
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

private:
    NuShellCommandProcessor(){};
};

/**
 * @brief Singleton instance of the NuShellCommandProcessor class
 *
 */
extern NuShellCommandProcessor &NuShellCommands;

#endif