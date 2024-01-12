/**
 * @file NuStream.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-24
 * @brief Communications stream based on the Nordic UART Service
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuStream.hpp"

//-----------------------------------------------------------------------------
// Constructor and destructor
//-----------------------------------------------------------------------------

NordicUARTStream::NordicUARTStream() : NordicUARTService(), Stream()
{
    dataConsumed = xSemaphoreCreateBinaryStatic(&dataConsumedBuffer);
    dataAvailable = xSemaphoreCreateBinaryStatic(&dataAvailableBuffer);
    xSemaphoreGive(dataConsumed);
}

NordicUARTStream::~NordicUARTStream()
{
    vSemaphoreDelete(dataConsumed);
    vSemaphoreDelete(dataAvailable);
}

//-----------------------------------------------------------------------------
// GATT server events
//-----------------------------------------------------------------------------

void NordicUARTStream::onDisconnect(NimBLEServer *pServer)
{
    NordicUARTService::onDisconnect(pServer);

    // Awake task at readBytes()
    disconnected = true;
    xSemaphoreGive(dataAvailable);
};

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NordicUARTStream::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Wait for previous data to get consumed
    xSemaphoreTake(dataConsumed, portMAX_DELAY);

    // Hold data until next read
    incomingPacket = pCharacteristic->getValue();
    unreadByteCount = incomingPacket.size();
    disconnected = false;

    // signal available data
    xSemaphoreGive(dataAvailable);
}

//-----------------------------------------------------------------------------
// Reading with no active wait
//-----------------------------------------------------------------------------

size_t NordicUARTStream::readBytes(uint8_t *buffer, size_t size)
{
    size_t totalReadCount = 0;
    while (size > 0)
    {
        // copy previously available data, if any
        if (unreadByteCount > 0)
        {
            const uint8_t *incomingData = incomingPacket.data() + incomingPacket.size() - unreadByteCount;
            size_t readBytesCount = (unreadByteCount > size) ? size : unreadByteCount;
            memcpy(buffer, incomingData, readBytesCount);
            buffer = buffer + readBytesCount;
            unreadByteCount = unreadByteCount - readBytesCount;
            totalReadCount = totalReadCount + readBytesCount;
            size = size - readBytesCount;
        } // note: at this point (unreadByteCount == 0) || (size == 0)
        if (unreadByteCount == 0)
        {
            xSemaphoreGive(dataConsumed);
        }
        if (size > 0)
        {
            // wait for more data or timeout or disconnection
            TickType_t timeoutTicks = (_timeout == ULONG_MAX) ? portMAX_DELAY : pdMS_TO_TICKS(_timeout);
            if ((xSemaphoreTake(dataAvailable, timeoutTicks) == pdFALSE) || disconnected)
                size = 0; // break;
            // Note: at this point, readBuffer and unreadByteCount were updated thanks to onWrite()
        }
    }
    return totalReadCount;
}

//-----------------------------------------------------------------------------
// Stream implementation
//-----------------------------------------------------------------------------

int NordicUARTStream::available()
{
    return unreadByteCount;
}

int NordicUARTStream::peek()
{
    if (unreadByteCount > 0)
    {
        const uint8_t *readBuffer = incomingPacket.data();
        size_t index = incomingPacket.size() - unreadByteCount;
        return readBuffer[index];
    }
    return -1;
}

int NordicUARTStream::read()
{
    if (unreadByteCount > 0)
    {
        const uint8_t *readBuffer = incomingPacket.data();
        size_t index = incomingPacket.size() - unreadByteCount;
        int result = readBuffer[index];
        unreadByteCount--;
        if (unreadByteCount == 0)
            xSemaphoreGive(dataConsumed);
        return result;
    }
    return -1;
}