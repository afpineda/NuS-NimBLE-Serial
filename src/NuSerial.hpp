/**
 * @file NuSerial.hpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Communications stream based on the Nordic UART Service
 *        with non-blocking Arduino semantics
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NUSERIAL_HPP__
#define __NUSERIAL_HPP__

#include <Arduino.h>
#include "NuStream.hpp"

/**
 * @brief Non-blocking serial communications through BLE and Nordic UART Service
 *
 */
class NordicUARTSerial : public NordicUARTStream
{
public:
    // Singleton pattern and Rule of Five

    NordicUARTSerial(const NordicUARTSerial &) = delete;
    NordicUARTSerial(NordicUARTSerial &&) = delete;
    NordicUARTSerial &operator=(const NordicUARTSerial &) = delete;
    NordicUARTSerial &operator=(NordicUARTSerial &&) = delete;

    /**
     * @brief Get the instance of the BLE stream
     *
     * @note No need to use. Use `NuSerial` instead.
     *
     * @return NordicUARTSerial&
     */
    static NordicUARTSerial &getInstance()
    {
        static NordicUARTSerial instance;
        return instance;
    };

public:
    // Methods not strictly needed. Provided to mimic `Serial`

    /**
     * @brief Start the Nordic UART Service
     *
     * @param baud Ignored parameter
     * @param ...  Ignored parameters
     */
    void begin(unsigned long baud, ...)
    {
        start();
    };

    /**
     * @brief
     *
     * @param dummy
     */
    void end(bool dummy = true)
    {
        disconnect();
    };

private:
    // Singleton pattern
    NordicUARTSerial() : NordicUARTStream() {};
    ~NordicUARTSerial() {};
};

/**
 * @brief Singleton instance of the NordicUARTSerial class
 *
 * @note Use this object as you do with Arduino's `Serial`
 */
extern NordicUARTSerial &NuSerial;

#endif