/**
 * @file NuS.cpp
 * @author Ángel Fernández Pineda. Madrid. Spain.
 * @date 2023-12-18
 * @brief Nordic UART Service implementation on NimBLE stack
 *
 * @note NimBLE-Arduino library is required.
 *       https://github.com/h2zero/NimBLE-Arduino
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <NimBLEDevice.h>
#include <exception>
#include <vector>
#include <string.h>
#include <cstdio>
#include "NuS.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

#define NORDIC_UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define RX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//-----------------------------------------------------------------------------
// Constructor / Initialization
//-----------------------------------------------------------------------------

NordicUARTService::NordicUARTService()
{
  peerConnected = xSemaphoreCreateBinaryStatic(&peerConnectedBuffer);
}

NordicUARTService::~NordicUARTService()
{
  vSemaphoreDelete(peerConnected);
}

void NordicUARTService::init()
{
  pServer = NimBLEDevice::createServer();
  if (pServer)
  {
    pServer->setCallbacks(this);
    pServer->getAdvertising()->addServiceUUID(NORDIC_UART_SERVICE_UUID);
    pNuS = pServer->createService(NORDIC_UART_SERVICE_UUID);
    if (pNuS)
    {
      pTxCharacteristic = pNuS->createCharacteristic(TX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::NOTIFY);
      if (pTxCharacteristic)
      {
        NimBLECharacteristic *pRxCharacteristic = pNuS->createCharacteristic(RX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE);
        if (pRxCharacteristic)
        {
          pRxCharacteristic->setCallbacks(this);
          connected = false;
          return;
        }
      }
    }
  }
  // Unable to initialize server
  throw std::runtime_error("Unable to create BLE server and/or Nordic UART Service");
}

//-----------------------------------------------------------------------------
// Start service
//-----------------------------------------------------------------------------

void NordicUARTService::start(void)
{
  if (!started)
  {
    init();
    pNuS->start();
    started = true;
    if (autoAdvertising)
      pServer->startAdvertising();
  }
}

//-----------------------------------------------------------------------------
// Connection
//-----------------------------------------------------------------------------

bool NordicUARTService::connect(const unsigned int timeoutMillis)
{
  TickType_t waitTicks = (timeoutMillis == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMillis);
  xSemaphoreTake(peerConnected, waitTicks);
  return connected;
}

void NordicUARTService::disconnect(void)
{
  std::vector<uint16_t> devices = pServer->getPeerDevices();
  for (uint16_t id : devices)
    pServer->disconnect(id);
}

//-----------------------------------------------------------------------------
// GATT server events
//-----------------------------------------------------------------------------

void NordicUARTService::onConnect(NimBLEServer *pServer)
{
  connected = true;
  if (pOtherServerCallbacks)
    pOtherServerCallbacks->onConnect(pServer);
  xSemaphoreGive(peerConnected);
}

void NordicUARTService::onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
  connected = true;
  if (pOtherServerCallbacks)
    pOtherServerCallbacks->onConnect(pServer, desc);
  xSemaphoreGive(peerConnected);
}

void NordicUARTService::onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
{
  connected = false;
  if (pOtherServerCallbacks)
    pOtherServerCallbacks->onDisconnect(pServer, desc);
  if (autoAdvertising)
    pServer->startAdvertising();
}

void NordicUARTService::onDisconnect(NimBLEServer *pServer)
{
  connected = false;
  if (pOtherServerCallbacks)
    pOtherServerCallbacks->onDisconnect(pServer);
  if (autoAdvertising)
    pServer->startAdvertising();
}

void NordicUARTService::setCallbacks(NimBLEServerCallbacks *pServerCallbacks)
{
  pOtherServerCallbacks = pServerCallbacks;
}

//-----------------------------------------------------------------------------
// Data transmission
//-----------------------------------------------------------------------------

size_t NordicUARTService::write(const uint8_t *data, size_t size)
{
  if (connected)
  {
    pTxCharacteristic->notify(data, size);
    return size;
  }
  return 0;
}

size_t NordicUARTService::send(const char *str, bool includeNullTerminatingChar)
{
  if (connected)
  {
    size_t size = includeNullTerminatingChar ? strlen(str) + 1 : strlen(str);
    pTxCharacteristic->notify((uint8_t *)str, size);
    return size;
  }
  return 0;
}

size_t NordicUARTService::printf(const char *format, ...)
{
  char dummy;
  va_list args;
  va_start(args, format);
  int requiredSize = vsnprintf(&dummy, 1, format, args);
  va_end(args);
  if (requiredSize == 0)
  {
    return write((uint8_t *)&dummy, 1);
  }
  else if (requiredSize > 0)
  {
    char *buffer = (char *)malloc(requiredSize + 1);
    if (buffer)
    {
      va_start(args, format);
      int result = vsnprintf(buffer, requiredSize + 1, format, args);
      va_end(args);
      if ((result >= 0) && (result <= requiredSize))
      {
        size_t writtenBytesCount = write((uint8_t *)buffer, result + 1);
        free(buffer);
        return writtenBytesCount;
      }
      free(buffer);
    }
  }
  return 0;
}