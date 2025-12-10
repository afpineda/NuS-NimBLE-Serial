/**
 * @file UartBleAdapter.ino
 * @author Sergii Pylypenko.
 * @date 2025-12-10
 * @brief UART to BLE adapter - transfers data between ESP32 TX/RX pins
 *        and Bluetooth service. Also copies data to USB UART interface,
 *        and to UART1 on pins 4 and 5, because UART0 prints boot logs.
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <Arduino.h>
#include "NuSerial.hpp"
#include "NimBLEDevice.h"

#define UART_SPEED 115200

uint8_t rxBuf[512];
size_t rxBufLen = 0;
uint8_t txBuf[512];
size_t txBufLen = 0;
uint32_t lastTransferTime = 0;

void setup()
{
    // Initialize serial monitor
    Serial0.setRxBufferSize(2048);
    Serial0.begin(UART_SPEED);
    Serial0.setTimeout(20);
    Serial1.setRxBufferSize(2048);
    Serial1.begin(UART_SPEED, SERIAL_8N1, 4, 5);
    Serial1.setTimeout(20);
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    HWCDCSerial.setRxBufferSize(2048);
    HWCDCSerial.begin(UART_SPEED);
    HWCDCSerial.setTimeout(20);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    USBSerial.setRxBufferSize(2048);
    USBSerial.begin(UART_SPEED);
    USBSerial.setTimeout(20);
#endif

    char name[17];
    snprintf(name, sizeof(name), "BLE-Serial-%04X", (uint16_t)ESP.getEfuseMac());
    // Initialize BLE stack and Nordic UART service
    NimBLEDevice::init(name);
    NimBLEDevice::getAdvertising()->setName(name);
    NuSerial.begin(UART_SPEED);
}

void loop()
{
    if (!NuSerial.isConnected()) {
        delay(10);
        return;
    }

    if (Serial0.available()) {
        rxBuf[rxBufLen] = (uint8_t)Serial0.read();
        rxBufLen ++;
    } else if (Serial1.available()) {
        rxBuf[rxBufLen] = (uint8_t)Serial1.read();
        rxBufLen ++;
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    } else if (HWCDCSerial.available()) {
        rxBuf[rxBufLen] = (uint8_t)HWCDCSerial.read();
        rxBufLen ++;
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    } else if (USBSerial.available()) {
        rxBuf[rxBufLen] = (uint8_t)USBSerial.read();
        rxBufLen ++;
#endif
    } else {
        delay(1);
    }

    if (rxBufLen >= sizeof(rxBuf) || rxBufLen > 0 && (
            rxBuf[rxBufLen - 1] == '\n' ||
            rxBuf[rxBufLen - 1] == '\r' ||
            millis() > lastTransferTime + 100)) {
        NuSerial.write(rxBuf, rxBufLen);
        rxBufLen = 0;
        lastTransferTime = millis();
    }

    while (NuSerial.available()) {
        txBuf[txBufLen] = (uint8_t)NuSerial.read();
        txBufLen ++;

        if (txBufLen >= sizeof(txBuf) || txBufLen > 0 && (
                txBuf[txBufLen - 1] == '\n' ||
                txBuf[txBufLen - 1] == '\r' ||
                millis() > lastTransferTime + 100)) {
            Serial0.write(txBuf, txBufLen);
            Serial1.write(txBuf, txBufLen);
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
            HWCDCSerial.write(txBuf, txBufLen);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
            USBSerial.write(txBuf, txBufLen);
#endif
            txBufLen = 0;
            lastTransferTime = millis();
        }
    }
}
