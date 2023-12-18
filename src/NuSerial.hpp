/**
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
#include "NuS.hpp"

/**
 * @brief Non-blocking serial communications through BLE and Nordic UART Service
 *
 */
class NordicUARTSerial : public NordicUARTService, public Stream
{
public:
    // Singleton pattern

    NordicUARTSerial(const NordicUARTSerial &) = delete;
    void operator=(NordicUARTSerial const &) = delete;

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
    // Overriden methods (note there is multiple inheritance)

    void onWrite(NimBLECharacteristic *pCharacteristic) override;

    /**
     * @brief  Gets the number of bytes available in the stream
     *
     * @return int The number of bytes available to read
     */
    virtual int available() override;

    /**
     * @brief  Reads a single character from an incoming stream
     *
     * @return int The first byte of incoming data available (or -1 if no data is available).
     */
    virtual int read() override;

    /**
     * @brief Read a byte from the stream without advancing to the next one
     *
     * @note Successive calls to peek() will return the same value,
     *       as will the next call to read().
     *
     * @return int The next byte or -1 if none is available.
     */
    virtual int peek() override;

    /**
     * @brief Write a single byte to the stream
     *
     * @param byte Byte to write
     * @return size_t The number of bytes written
     */
    virtual size_t write(uint8_t byte) override
    {
        return NordicUARTService::write(&byte, 1);
    };

    /**
     * @brief Write bytes to the stream
     *
     * @param buffer Pointer to first byte to write
     * @param size Count of bytes to write
     * @return size_t Actual count of bytes that were written
     */
    virtual size_t write(const uint8_t *buffer, size_t size) override
    {
        return NordicUARTService::write(buffer, size);
    };

public:
    // Methods not strictly needed. Provided to mimic `Serial`

    void begin(unsigned long baud, ...)
    {
        start();
    };
    void end(bool dummy = true)
    {
        disconnect();
    };

private:
    SemaphoreHandle_t dataConsumed;
    size_t availableByteCount = 0;
    const uint8_t *incomingBuffer = nullptr;
    NordicUARTSerial();
    ~NordicUARTSerial();
};

/**
 * @brief Singleton instance of the NordicUARTSerial class
 *
 * @note Use this object as you do with Arduino's `Serial`
 */
extern NordicUARTSerial &NuSerial;

#endif