/**
 * @file UartBleAdapter.ino
 * @author Sergii Pylypenko.
 *
 * @date 2025-12-10
 *
 * @brief UART to BLE adapter - transfers data between
 *        the configured ESP32's UARTS and the Nordic UART Service,
 *        including hardware UARTS and the USB CDC UART (if available).
 *
 * @note See examples/README.md for a description
 *
 * @copyright Creative Commons Attribution 4.0 International (CC BY 4.0)
 *
 */

#include <HardwareSerial.h>
#include "NuSerial.hpp"
#include "NimBLEDevice.h"

// Expected hardware UART baud rate for UART0
// (using the default RX/TX pins) and
// UART1 (configured to pins 4 and 5 below).
// **Set to ZERO to ignore such an UART**.
// All ESP32 chips are configured by default to 115200 bauds.
// Make sure the device connected to the other endpoint
// matches this baud rate.
#define UART0_BAUD_RATE 115200
#define UART1_BAUD_RATE 115200

// NOTE:
// To enable or disable the USB CDC UART (if available)
// go to the "Board Configuration" option in Arduino IDE
// and choose a value for "USB CDC on boot"

// Buffer size (set as you wish)
#define BUFFER_SIZE 2048
// Read/write buffer to hold data in transit
uint8_t data_buffer[BUFFER_SIZE];

void setup()
{
#if UART0_BAUD_RATE > 0
    // Initialize the 1st hardware UART
    Serial0.setRxBufferSize(2048);
    Serial0.begin(UART0_BAUD_RATE);
    Serial0.setTimeout(20);
#endif
#if UART1_BAUD_RATE > 0
    // Initialize the 2nd hardware UART
    Serial1.setRxBufferSize(2048);
    // Configured to pins 4 and 5. Feel free to change.
    Serial1.begin(UART1_BAUD_RATE, SERIAL_8N1, 4, 5);
    Serial1.setTimeout(20);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    // Initialize the USB CDC UART (if available)
    HWCDCSerial.setRxBufferSize(2048);
    HWCDCSerial.begin(); // Note: USB CDC ignores the baud parameter
    HWCDCSerial.setTimeout(20);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    USBSerial.setRxBufferSize(2048);
    USBSerial.begin(); // Note: USB CDC ignores the baud parameter
    USBSerial.setTimeout(20);
#endif

    char name[17];
    snprintf(name, sizeof(name), "BLE-Serial-%04X", (uint16_t)ESP.getEfuseMac());
    // Initialize the BLE stack and the Nordic UART service
    NimBLEDevice::init(name);
    NimBLEDevice::getAdvertising()->setName(name);
    NuSerial.begin(); // Note: NuS ignores the baud parameter
}

// Some general notes:
// - We don't care about the connection state (there is no need to).
// - We don't handle errors as there is no
//   place to report them.

void loop()
{
    // First, we read data from the configured UARTS
    // and send it to NuSerial
    // ---------------------------------------------
    size_t available_size;

#if UART0_BAUD_RATE > 0
    available_size = Serial0.readBytes(data_buffer, Serial0.available());
    NuSerial.write(data_buffer, available_size);
#endif
#if UART1_BAUD_RATE > 0
    available_size = Serial1.readBytes(data_buffer, Serial1.available());
    NuSerial.write(data_buffer, available_size);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    available_size = HWCDCSerial.readBytes(data_buffer, HWCDCSerial.available());
    NuSerial.write(data_buffer, available_size);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    available_size = USBSerial.readBytes(data_buffer, USBSerial.available());
    NuSerial.write(data_buffer, available_size);
#endif

    // Next, we read data from NuSerial
    // and send it to the configured UARTS
    // ---------------------------------------------
    available_size = NuSerial.readBytes(data_buffer, NuSerial.available());
#if UART0_BAUD_RATE > 0
    Serial0.write(data_buffer, available_size);
#endif
#if UART1_BAUD_RATE > 0
    Serial1.write(data_buffer, available_size);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE
    HWCDCSerial.write(data_buffer, available_size);
#endif
#if ARDUINO_USB_CDC_ON_BOOT && !ARDUINO_USB_MODE
    USBSerial.write(data_buffer, available_size);
#endif
}
