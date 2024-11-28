/**
 * @file NuS.hpp
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
#ifndef __NUS_NIMBLE_HPP__
#define __NUS_NIMBLE_HPP__

#include <NimBLEServer.h>
#include <NimBLEService.h>
#include <NimBLECharacteristic.h>
#include <cstring>

/**
 * @brief UUID for the Nordic UART Service
 *
 * @note You may need this to handle advertising on your own
 */
#define NORDIC_UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"

/**
 * @brief Nordic UART Service (NuS) implementation using the NimBLE stack
 *
 * @note This is an abstract class.
 *       Override NimBLECharacteristicCallbacks::onWrite(NimBLECharacteristic *pCharacteristic)
 *       to process incoming data. A singleton pattern is suggested.
 */
class NordicUARTService : protected NimBLECharacteristicCallbacks
{
public:
  /**
   * @brief Check if a peer is connected and subscribed to this service
   *
   * @return true When a connection is established
   * @return false When no peer is connected
   */
  bool isConnected();

  /**
   * @brief Get the count of clients subscribed
   *
   * @return uint8_t Number of clients subscribed to the service
   */
  uint8_t subscriberCount() { return subscribedCount; }

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
   * @return true on peer connection and service subscription
   * @return false on timeout
   */
  bool connect(const unsigned int timeoutMillis = 0);

  /**
   * @brief Terminate all peer connections (if any),
   *        subscribed or not.
   *
   */
  void disconnect(void);

  /**
   * @brief Send bytes
   *
   * @param[in] data Pointer to bytes to be sent.
   * @param[in] size Count of bytes to be sent.
   * @return size_t @p size .
   */
  size_t write(const uint8_t *data, size_t size);

  /**
   * @brief Send a null-terminated string (ANSI encoded)
   *
   * @param[in] str Pointer to null-terminated string to be sent.
   * @param[in] includeNullTerminatingChar When true, the null terminating character is sent too.
   *            When false, such a character is not sent, so @p str should end with another
   *            termination token, like CR (Unix), LF (old MacOS) or CR+LF (Windows).
   *
   * @return size_t Count of bytes sent.
   */
  size_t send(const char *str, bool includeNullTerminatingChar = false);

  /**
   * @brief Send a string (any encoding)
   *
   * @param str String to send
   * @return size_t Count of bytes sent.
   */
  size_t print(std::string str)
  {
    return write((const uint8_t *)str.data(), str.length());
  };

  /**
   * @brief Send a formatted string (ANSI encoded)
   *
   * @note The null terminating character is sent too.
   *
   * @param[in] format String that follows the same specifications as format in printf()
   * @param[in] ... Depending on the format string, a sequence of additional arguments,
   *            each containing a value to replace a format specifier in the format string.
   *
   * @return size_t Count of bytes sent.
   */
  size_t printf(const char *format, ...);

  /**
   * @brief Start the Nordic UART Service
   *
   * @note NimBLEDevice::init() **must** be called before.
   * @note The service is unavailable if start() is not called.
   *       Do not call start() before initialization is complete in your application.
   *
   * @throws std::runtime_error if the UART service is already created or can not be created
   */
  void start(void);

  /**
   * @brief Set your own server callbacks
   *
   * @deprecated Use NimBLEDevice::createServer()->setCallbacks()
   *
   * @param pServerCallbacks The callbacks to be invoked. Must remain
   *                         valid forever (do not destroy).
   */
  void setCallbacks(NimBLEServerCallbacks *pServerCallbacks);

  /**
   * @brief Automatically advertise BLE services when no peer is connected
   *
   * @note This is the default behavior.
   *
   */
  void enableAutoAdvertising()
  {
    autoAdvertising = true;
  };

  /**
   * @brief Do not advertise BLE services when no peer is connected
   *
   * @note You should handle advertising on your own if you call this method.
   *
   */
  void disableAutoAdvertising()
  {
    autoAdvertising = false;
  };

protected:
  virtual void onSubscribe(
      NimBLECharacteristic *pCharacteristic,
      ble_gap_conn_desc *desc,
      uint16_t subValue) override;

protected:
  /**
   * @brief Event callback for client subscription to the TX characteristic
   *
   * @note Called before the semaphore is released.
   *
   * @param subscriberCount Number of subscribed clients
   */
  virtual void onSubscribe(uint8_t subscriberCount) {};

  /**
   * @brief Event callback for client unsubscription to the TX characteristic
   *
   * @param subscriberCount Number of subscribed clients
   */
  virtual void onUnsubscribe(uint8_t subscriberCount) {};

protected:
  NordicUARTService();
  virtual ~NordicUARTService();

private:
  NimBLEServer *pServer = nullptr;
  NimBLEService *pNuS = nullptr;
  NimBLECharacteristic *pTxCharacteristic = nullptr;
  SemaphoreHandle_t peerConnected;
  StaticSemaphore_t peerConnectedBuffer;
  bool autoAdvertising = true;
  bool started = false;
  uint8_t subscribedCount = 0;

  /**
   * @brief Create the NuS service in a new GATT server
   *
   * @throws std::runtime_error if the UART service is already created or can not be created
   */
  void init();
};

#endif