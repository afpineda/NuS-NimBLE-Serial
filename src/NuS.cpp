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
#include <stdexcept>
#include <vector>
#include <string.h>
#include <cstdio>
#include <chrono>
#include "NuS.hpp"

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

#define RX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define TX_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//-----------------------------------------------------------------------------
// Constructor / Initialization
//-----------------------------------------------------------------------------

void NordicUARTService::init(bool advertise)
{
   pServer = NimBLEDevice::getServer();
   if (pServer == nullptr)
      pServer = NimBLEDevice::createServer();
   if (pServer)
   {
      if (advertise)
         pServer->getAdvertising()->addServiceUUID(NORDIC_UART_SERVICE_UUID);
      pNuS = pServer->createService(NORDIC_UART_SERVICE_UUID);
      if (pNuS)
      {
         pTxCharacteristic = pNuS->createCharacteristic(TX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::NOTIFY);
         if (pTxCharacteristic)
         {
            pTxCharacteristic->setCallbacks(this); // uses onSubscribe
            NimBLECharacteristic *pRxCharacteristic = pNuS->createCharacteristic(RX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE);
            if (pRxCharacteristic)
            {
               pRxCharacteristic->setCallbacks(this); // uses onWrite
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

void NordicUARTService::start(bool autoAdvertising)
{
   if (!started)
   {
      init(autoAdvertising);
      pNuS->start();
      started = true;
      if (autoAdvertising)
      {
         pServer->advertiseOnDisconnect(true);
         pServer->startAdvertising();
      }
   }
}

//-----------------------------------------------------------------------------
// Connection
//-----------------------------------------------------------------------------

bool NordicUARTService::isConnected()
{
   return (_subscriberCount > 0);
}

bool NordicUARTService::connect(const unsigned int timeoutMillis)
{
   if (timeoutMillis == 0)
   {
      peerConnected.acquire();
      return true;
   }
   else
   {
      return peerConnected.try_acquire_for(std::chrono::milliseconds(timeoutMillis));
   }
}

void NordicUARTService::disconnect(void)
{
   std::vector<uint16_t> devices = pServer->getPeerDevices();
   for (uint16_t id : devices)
      pServer->disconnect(id);
}

//-----------------------------------------------------------------------------
// TX events
//-----------------------------------------------------------------------------

void NordicUARTService::onSubscribe(
    NimBLECharacteristic *pCharacteristic,
    NimBLEConnInfo &connInfo,
    uint16_t subValue)
{
   // Note: for robustness, we assume this callback could be called
   // even if no subscription event exists.

   if (subValue == 0)
   {
      // unsubscribe
      if (_subscriberCount > 0)
      {
         _subscriberCount--;
         onUnsubscribe(_subscriberCount);
      }
   }
   else if (subValue < 4)
   {
      // subscribe
      _subscriberCount++;
      onSubscribe(_subscriberCount);
      peerConnected.release();
   }
   // else: Invalid subscription value, ignore
}

//-----------------------------------------------------------------------------
// Data transmission
//-----------------------------------------------------------------------------

size_t NordicUARTService::write(const uint8_t *data, size_t size)
{
   pTxCharacteristic->notify(data, size);
   return size;
}

size_t NordicUARTService::send(const char *str, bool includeNullTerminatingChar)
{
   size_t size = includeNullTerminatingChar ? strlen(str) + 1 : strlen(str);
   pTxCharacteristic->notify((uint8_t *)str, size);
   return size;
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