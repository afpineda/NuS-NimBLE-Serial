/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-24
 * @brief Communications stream based on the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#ifndef __NUSTREAM_HPP__
#define __NUSTREAM_HPP__

#include <climits>
#include <Stream.h>
#include "NuS.hpp"

/**
 * @brief Communications stream through BLE and Nordic UART Service
 *
 */
class NordicUARTStream : public NordicUARTService, public Stream
{
public:
    // Overriden Methods

    void onDisconnect(NimBLEServer *pServer) override;
    void onWrite(NimBLECharacteristic *pCharacteristic) override;

public:
    NordicUARTStream();
    virtual ~NordicUARTStream();

public:
    /**
     * @brief  Gets the number of bytes available in the stream
     *
     * @return int The number of bytes available to read
     */
    virtual int available() override;

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
     * @brief  Reads a single character from an incoming stream
     *
     * @return int The first byte of incoming data available (or -1 if no data is available).
     */
    virtual int read() override;

    /**
     * @brief Read characters from a stream into a buffer.
     *
     * @note Terminates if the determined length has been read, or it times out.
     *       Unlike other read methods, no active waiting is used here.
     *
     * @note Call `setTimeout(ULONG_MAX)` to disable time outs.
     *
     * @param buffer To store the bytes in
     * @param size the Number of bytes to read
     * @return size_t Number of bytes placed in the buffer
     */

    virtual size_t readBytes(uint8_t *buffer, size_t size) override;
    virtual size_t readBytes(char *buffer, size_t length) override
    {
        return NordicUARTStream::readBytes((uint8_t *)buffer, length);
    };

public:
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

private:
    SemaphoreHandle_t dataConsumed;
    SemaphoreHandle_t dataAvailable;
    NimBLEAttValue incomingPacket;
    bool disconnected = false;
    size_t unreadByteCount = 0;
};

#endif