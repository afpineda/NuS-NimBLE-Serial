#ifndef __NUSTREAM_HPP__
#define __NUSTREAM_HPP__

#include "NuS.hpp"

/**
 * @brief Blocking serial communications through BLE and Nordic UART Service
 *
 * @note Unlike `Serial`, the semantics
 *       are those of blocking communications. This is more efficient in
 *       terms of CPU usage, since no active waiting is used, and a performance
 *       boost, since incoming bytes are processed in packets, not one bye one.
 *       However, a multi-tasking app design must be adopted.
 *
 */
class NordicUARTBlockingStream : public NordicUARTService
{
public:
    NordicUARTBlockingStream(const NordicUARTBlockingStream &) = delete;
    void operator=(NordicUARTBlockingStream const &) = delete;

    /**
     * @brief Get the instance of the BLE stream
     *
     * @note No need to use. Use `NuStream` instead.
     *
     * @return NordicUARTBlockingStream&
     */
    static NordicUARTBlockingStream &getInstance()
    {
        static NordicUARTBlockingStream instance;
        return instance;
    };

public:
    void onConnect(NimBLEServer *pServer) override;
    void onDisconnect(NimBLEServer *pServer) override;
    void onWrite(NimBLECharacteristic *pCharacteristic) override;

public:
    /**
     * @brief Wait for a peer connection or a timeout if set (blocking)
     *
     * @param[in] timeoutMillis Maximum time to wait (in milliseconds) or
     *                          zero to disable timeouts and wait forever
     *
     * @note It is not mandatory to call this method in order to read or write.
     *
     * @note Just one task can go beyond connect(), except in case of timeout,
     *       if more than one exists.
     *
     * @return true on peer connection
     * @return false on timeout
     */
    bool connect(const unsigned int timeoutMillis = 0);

    /**
     * @brief Wait for and get incoming data (blocking)
     *
     * @note The calling task will get blocked until incoming data is
     *       available or the connection is dropped. Just one task
     *       can go beyond beginRead() if more than one exists.
     *
     * @note You should not perform any time-consuming task between calls.
     *       Use buffers/queues/etc for that.
     *
     * @param[out] size Count of incoming bytes,
     *                  or zero if the connection was dropped.
     * @return uint8_t* Pointer to incoming data, or `nullptr` if the connection
     *                  was dropped.
     *                  Do not access more bytes than available as given in
     *                  @p size. Otherwise, a segmentation fault may occur.
     */
    const uint8_t *read(size_t &size);

private:
    SemaphoreHandle_t dataConsumed;
    SemaphoreHandle_t dataAvailable;
    SemaphoreHandle_t peerConnected;
    size_t availableByteCount = 0;
    const uint8_t *incomingBuffer = nullptr;
    NordicUARTBlockingStream();
    ~NordicUARTBlockingStream();
};

/**
 * @brief Singleton instance of the NordicUARTBlockingStream class
 *
 */
extern NordicUARTBlockingStream &NuStream;

#endif