/**
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Communications stream based on the Nordic UART Service
 *        with non-blocking Arduino semantics
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include "NuSerial.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NordicUARTSerial &NuSerial = NordicUARTSerial::getInstance();

//-----------------------------------------------------------------------------
// Constructor / destructor
//-----------------------------------------------------------------------------

NordicUARTSerial::NordicUARTSerial() : NordicUARTService(), Stream()
{
    availableByteCount = 0;
    incomingBuffer = nullptr;
    dataConsumed = xSemaphoreCreateBinary();
    xSemaphoreGive(dataConsumed);
}

NordicUARTSerial::~NordicUARTSerial()
{
    vSemaphoreDelete(dataConsumed);
}

//-----------------------------------------------------------------------------
// NordicUARTService implementation
//-----------------------------------------------------------------------------

void NordicUARTSerial::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Wait for previous data to get consumed
    xSemaphoreTake(dataConsumed, portMAX_DELAY);

    // Hold data until next read
    NimBLEAttValue val = pCharacteristic->getValue();
    incomingBuffer = val.data();
    availableByteCount = val.size(); // this must be the last line. Important!!!
}

//-----------------------------------------------------------------------------
// Stream implementation
//-----------------------------------------------------------------------------

int NordicUARTSerial::available()
{
    return availableByteCount;
}

int NordicUARTSerial::peek()
{
    int result = -1;
    if (availableByteCount > 0)
        result = (int)incomingBuffer[0];
    return result;
}

int NordicUARTSerial::read()
{
    int result = -1;
    if (availableByteCount > 0)
    {
        result = (int)(incomingBuffer[0]);
        incomingBuffer++;
        availableByteCount--;
        if (availableByteCount <= 0)
            xSemaphoreGive(dataConsumed);
    }
    return result;
}
