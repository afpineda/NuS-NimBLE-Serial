/**
 * @file NuPacket.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Communications stream based on the Nordic UART Service
 *        with blocking semantics
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <exception>
#include "NuPacket.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NordicUARTPacket &NuPacket = NordicUARTPacket::getInstance();

//-----------------------------------------------------------------------------
// Constructor / destructor
//-----------------------------------------------------------------------------

NordicUARTPacket::NordicUARTPacket() : NordicUARTService()
{
    availableByteCount = 0;
    incomingBuffer = nullptr;
    dataConsumed = xSemaphoreCreateBinaryStatic(&dataConsumedBuffer);
    dataAvailable = xSemaphoreCreateBinaryStatic(&dataAvailableBuffer);
    xSemaphoreGive(dataConsumed);
}

NordicUARTPacket::~NordicUARTPacket()
{
    vSemaphoreDelete(dataConsumed);
    vSemaphoreDelete(dataAvailable);
}

//-----------------------------------------------------------------------------
// GATT server events
//-----------------------------------------------------------------------------

void NordicUARTPacket::onDisconnect(NimBLEServer *pServer)
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

void NordicUARTPacket::onWrite(NimBLECharacteristic *pCharacteristic)
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
// Reading
//-----------------------------------------------------------------------------

const uint8_t *NordicUARTPacket::read(size_t &size)
{
    xSemaphoreGive(dataConsumed);
    xSemaphoreTake(dataAvailable, portMAX_DELAY);
    size = availableByteCount;
    return incomingBuffer;
}
