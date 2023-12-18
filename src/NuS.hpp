/**
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

/**
 * @brief Nordic UART Service (NuS) implementation using the NimBLE stack
 *
 * @note This is an abstract class.
 *       Override NimBLECharacteristicCallbacks::onWrite(NimBLECharacteristic *pCharacteristic)
 *       to process incoming data. A singleton pattern is suggested.
 */
class NordicUARTService : public NimBLEServerCallbacks, public NimBLECharacteristicCallbacks
{
public:
  /**
   * @brief Check if a peer is connected
   *
   * @return true When a connection is established
   * @return false When no peer is connected
   */
  bool isConnected() { return connected; };

  /**
   * @brief Send bytes
   *
   * @param data Pointer to bytes to be sent.
   * @param size Count of bytes to be sent.
   * @return size_t Zero if no peer is connected, @p size otherwise.
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
   * @return size_t Zero if no peer is connected.
   *                Otherwise, count of bytes sent.
   */
  size_t send(const char *str, bool includeNullTerminatingChar = false);

  /**
   * @brief Start the Nordic UART Service
   *
   * @note NimBLEDevice::init() **must** be called before.
   * @note The service is unavailable if start() is not called, thus no peer connections
   *       are accepted. Do not call start() before initialization is complete in your application.
   *
   * @throws std::runtime_error if the UART service is already created or can not be created
   */
  void start(void);

  /**
   * @brief Terminate current peer connection (if any)
   *
   */
  void disconnect(void);

public:
  void onConnect(NimBLEServer *pServer) override;
  void onDisconnect(NimBLEServer *pServer) override;

private:
  NimBLEServer *pServer = nullptr;
  NimBLEService *pNuS = nullptr;
  NimBLECharacteristic *pTxCharacteristic = nullptr;
  bool connected = false;
  bool started = false;

  /**
   * @brief Create the NuS service in a new GATT server
   *
   * @throws std::runtime_error if the UART service is already created or can not be created
   */
  void init();
};

#endif