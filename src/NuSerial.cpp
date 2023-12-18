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
    // Note: do note change the order of these lines. This is critical.
    NimBLEAttValue val = pCharacteristic->getValue();
    buffer = (uint8_t *)val.data();
    availableByteCount = val.size();
    //Serial.printf("onWrite: enter. %d bytes\n", availableByteCount);
    while (availableByteCount > 0)
        xSemaphoreTake(dataConsumed, portMAX_DELAY);
    //Serial.printf("onWrite: exit. %d bytes\n", availableByteCount);
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
        result = (int)buffer[0];
    return result;
}

int NordicUARTSerial::read()
{
    int result = -1;
    if (availableByteCount > 0)
    {
        result = (int)buffer[0];
        buffer++;
        availableByteCount--;
        if (availableByteCount <= 0)
            xSemaphoreGive(dataConsumed);
    }
    return result;
}

size_t NordicUARTSerial::write(uint8_t byte)
{
    return NordicUARTService::write(&byte, 1);
};
