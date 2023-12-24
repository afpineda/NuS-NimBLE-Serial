/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Communications stream based on the Nordic UART Service
 *        with blocking semantics
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuStream.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NordicUARTBlockingStream &NuStream = NordicUARTBlockingStream::getInstance();

//-----------------------------------------------------------------------------
// Constructor / destructor
//-----------------------------------------------------------------------------

NordicUARTBlockingStream::NordicUARTBlockingStream() : NordicUARTService()
{
    availableByteCount = 0;
    incomingBuffer = nullptr;
    dataConsumed = xSemaphoreCreateBinary();
    dataAvailable = xSemaphoreCreateBinary();
    peerConnected = xSemaphoreCreateBinary();
    xSemaphoreGive(dataConsumed);
}

NordicUARTBlockingStream::~NordicUARTBlockingStream()
{
    vSemaphoreDelete(dataConsumed);
    vSemaphoreDelete(dataAvailable);
    vSemaphoreDelete(peerConnected);
}

//-----------------------------------------------------------------------------
// GATT server events
//-----------------------------------------------------------------------------

void NordicUARTBlockingStream::onConnect(NimBLEServer *pServer)
{
    NordicUARTService::onConnect(pServer);
    xSemaphoreGive(peerConnected);
};

void NordicUARTBlockingStream::onDisconnect(NimBLEServer *pServer)
{
    NordicUARTService::onDisconnect(pServer);

    // Awake task at read()
    availableByteCount = 0;
    incomingBuffer = nullptr;
    xSemaphoreGive(dataAvailable);
};

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NordicUARTBlockingStream::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Wait for previous data to get consumed
    xSemaphoreTake(dataConsumed, portMAX_DELAY);

    // Hold data until next read
    incomingPacket = pCharacteristic->getValue();
    incomingBuffer = incomingPacket.data();
    availableByteCount = incomingPacket.size();

    // signal available data
    xSemaphoreGive(dataAvailable);
}

//-----------------------------------------------------------------------------
// Connection events
//-----------------------------------------------------------------------------

bool NordicUARTBlockingStream::connect(const unsigned int timeoutMillis)
{
    TickType_t waitTicks = (timeoutMillis == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMillis);
    return (xSemaphoreTake(peerConnected, waitTicks) == pdTRUE);
}

//-----------------------------------------------------------------------------
// Reading
//-----------------------------------------------------------------------------

const uint8_t *NordicUARTBlockingStream::read(size_t &size)
{
    xSemaphoreGive(dataConsumed);
    xSemaphoreTake(dataAvailable, portMAX_DELAY);
    size = availableByteCount;
    return incomingBuffer;
}
