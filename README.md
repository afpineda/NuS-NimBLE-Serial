# Nordic UART Service (NuS) and BLE serial communications (NimBLE stack)

Library for serial communications through Bluetooth Low Energy on ESP32-Arduino boards

In summary, this library provides:

- A BLE serial communications object that can be used as Arduino's [Serial](https://www.arduino.cc/reference/en/language/functions/communication/serial/).
- A BLE serial communications object that can handle incoming data in packets, eluding active waiting thanks to blocking semantics.
- A customizable and easy to use [AT command](https://www.twilio.com/docs/iot/supersim/introduction-to-modem-at-commands) processor based on NuS.
- A customizable [shell](https://en.wikipedia.org/wiki/Shell_(computing)) command processor based on NuS.
- A generic class to implement custom protocols for serial communications through BLE.

## Supported DevKit boards

Any DevKit supported by [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) and based on the [Arduino core for Espressif's boards](https://github.com/espressif/arduino-esp32)
(since [FreeRTOS](https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html) is required).

## Introduction

Serial communications are already available through the old [Bluetooth classic](https://www.argenox.com/library/bluetooth-classic/introduction-to-bluetooth-classic/) specification (see [this tutorial](https://circuitdigest.com/microcontroller-projects/using-classic-bluetooth-in-esp32-and-toogle-an-led)), [Serial Port Profile (SPP)](https://www.bluetooth.com/specifications/specs/serial-port-profile-1-2/).
However, this is not the case with the [Bluetooth Low Energy (BLE) specification](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy).
**No standard** protocol was defined for serial communications in BLE (see [this article](https://punchthrough.com/serial-over-ble/) for further information).

As bluetooth classic is being dropped in favor of BLE, an alternative is needed. [Nordic UART Service (NuS)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/libraries/bluetooth_services/services/nus.html) is a popular alternative, if not the *de facto* standard.
This library implements the Nordic UART service on the *NimBLE-Arduino* stack.

## Client-side application

You may need a generic terminal (PC or smartphone) application in order to communicate with your Arduino application through BLE. Such a generic application must support the Nordic UART Service. There are several free alternatives (known to me):

- Android:
  - [nRF connect for mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
  - [Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal)
- iOS:
  - [nRF connect for mobile](https://apps.apple.com/es/app/nrf-connect-for-mobile/id1054362403)

## How to use this library

Summary:

- The `NuSerial` object provides non-blocking serial communications through BLE, *Arduino's style*.
- The `NuPacket` object provides blocking serial communications through BLE.
- The `NuATCommands` object provides custom processing of AT commands through BLE.
- The `NuShellCommands` object provides custom processing of shell commands through BLE.
- Create your own object to provide a custom protocol based on serial communications through BLE, by deriving a new class from `NordicUARTService`.

The **basic rules** are:

- You must initialize the *NimBLE stack* **before** using this library. See [NimBLEDevice::init()](https://h2zero.github.io/NimBLE-Arduino/class_nim_b_l_e_device.html).
- You must also call `<object>.start()` **after** all code initialization is complete.
- Just one object can use the Nordic UART Service. For example, this code **fails** at run time:

  ```c++
  void setup() {
    ...
    NuSerial.start();
    NuPacket.start(); // raises an exception (runtime_error)
  }
  ```

- This library sets their own [server callbacks](https://h2zero.github.io/esp-nimble-cpp/class_nim_b_l_e_server_callbacks.html), so **don't overwrite them**. For example, this code **does not work**:

  ```c++
  void setup() {
    ...
    NimBLEDevice::init("MyDevice");
    NuSerial.start();
    // NuSerial callbacks are overwritten
    NimBLEDevice::createServer()->setCallbacks(myOwnCallbacks);
  }
  ```

  **Nor this one**:

  ```c++
  void setup() {
    ...
    NimBLEDevice::init("MyDevice");
    NimBLEDevice::createServer()->setCallbacks(myOwnCallbacks);
    // Your own callbacks are overwritten
    NuSerial.start();
  }
  ```

- Nevertheless, you can have your own server callbacks. Use `<object>.setCallbacks()` instead of `NimBLEServer::setCallbacks()`.
  For example:

  ```c++
  void setup() {
    ...
    NimBLEDevice::init("MyDevice");
    // Your own callbacks are NOT overwritten in this way
    NuSerial.setCallbacks(myOwnCallbacks);
    NuSerial.start();
  }
  ```

- The Nordic UART Service can coexist with other GATT services in your application.

- By default, this library will automatically advertise existing GATT services when no peer is connected. This includes the Nordic UART Service and other
  services (if any). To change this behavior, call `<object>.disableAutoAdvertising()` and handle advertising on your own.

You may learn from the provided [examples](./examples/README.md). Read code commentaries for more information.

### Non-blocking serial communications

```c++
#include "NuSerial.hpp"
```

In short, use the `NuSerial` object as you do with the Arduino's `Serial` object. For example:

```c++
void setup()
{
    ...
    NimBLEDevice::init("My device");
    ...
    NuSerial.begin(115200); // Note: parameter is ignored
}

void loop()
{
    if (NuSerial.available())
    {
        // read incoming data and do something
        ...
    } else {
        // other background processing
        ...
    }
}
```

Take into account:

- `NuSerial` inherits from Arduino's `Stream`, so you can use it with other libraries.
- As you should know, `read()` will immediately return if there is no data available.
  But, this is also the case when no peer device is connected.
  Use `NuSerial.isConnected()` to know the case (if you need to).
- `NuSerial.begin()` or `NuSerial.start()` must be called at least once before reading. Calling more than once have no effect.
- `NuSerial.end()` (as well as `NuSerial.disconnect()`) will terminate any peer connection.
  If you pretend to read again, it's not mandatory to call `NuSerial.begin()` (nor `NuSerial.start()`) again, but you can.
- As a bonus, `NuSerial.readBytes()` does not perform active waiting, unlike `Serial.readBytes()`.
- As you should know, `Stream` read methods are not thread-safe. Do not read from two different OS tasks.

### Blocking serial communications

```c++
#include "NuPacket.hpp"
```

Use the `NuPacket` object, based on blocking semantics. The advantages are:

- Efficiency in terms of CPU usage, since no active waiting is used.
- Performance, since incoming bytes are processed in packets, not one bye one.
- Simplicity. Only two methods are strictly needed: `read()` and `write()`.
  You don't need to worry about data being available or not.
  However, you have to handle packet size.

For example:

```c++
void setup()
{
    ...
    NimBLEDevice::init("My device");
    ... // other initialization
    NuPacket.start(); // don't forget this!!
}

void loop()
{
    size_t size;
    const uint8_t *data = NuPacket.read(size); // "size" is an output parameter
    while (data)
    {
        // do something with data and size
        ...
        data = NuPacket.read(size);
    }
    // No peer connection at this point
}
```

Take into account:

- **Just one** OS task can work with `NuPacket` (others will get blocked).
- Data should be processed as soon as possible. Use other tasks and buffers/queues for time-consuming computation.
  While data is being processed, the peer will stay blocked, unable to send another packet.
- If you just pretend to read a known-sized burst of bytes, `NuSerial.readBytes()` do the job with the same benefits as `NuPacket`
  and there is no need to manage packet sizes. Call `NuSerial.setTimeout(ULONG_MAX)` previously to get the blocking semantics.

### Custom AT commands

```c++
#include "NuATCommands.hpp"

class MyATCommands: public NuATCommandCallbacks {
    public:
        virtual int getATCommandId(const char commandName[]) override;
    ...
} myATCommandsObject;
```

- Derive a new class from `NuATCommandCallbacks`.
- Override `getATCommandId()` to return a positive number on supported commands or a negative number on unsupported commands. This is mandatory.
- Override `onExecute()` to run commands with no suffix.
- Override `onSet()` to run commands with "=" suffix.
- Override `onQuery()` to run commands with "?" suffix.
- Override `onTest()` to run commands with "=?" suffix.
- Create a single instance of your derived class and pass it to `NuATCommands.setATCallbacks()`.
- Call `NuATCommands.start()`


Implementation is based in these sources:

- [Espressif's AT command set](https://docs.espressif.com/projects/esp-at/en/release-v2.2.0.0_esp8266/AT_Command_Set/index.html)
- [An Introduction to AT Commands](https://www.twilio.com/docs/iot/supersim/introduction-to-modem-at-commands)
- [GSM AT Commands Tutorial](https://microcontrollerslab.com/at-commands-tutorial/#Response_of_AT_commands)
- [General Syntax of Extended AT Commands](https://www.developershome.com/sms/atCommandsIntro2.asp)

Current implementation only accepts ASCII/ANSI character encoding.
As a bonus, you may use class `NuATCommandParser` to implement an AT command processor that takes data from other sources.

### Custom shell commands

```c++
#include "NuShellCommands.hpp"

void setup()
{
  NuShellCommands
    .on("cmd1", [](NuCommandLine_t &commandLine)
    {
      // Note: commandLine[0] == "cmd1"
      //       commandLine[1] is the first argument an so on
      ...
    }
    )
    .on("cmd2", [](NuCommandLine_t &commandLine)
    {
      ...
    }
    .onUnknown([](NuCommandLine_t &commandLine)
    {
      Serial.printf("ERROR: unknown command \"%s\"\n",commandLine[0].c_str());
    }
    )
    .onParseError([](NuCLIParsingResult_t result, size_t index)
    {
      if (result == CLI_PR_ILL_FORMED_STRING)
        Serial.printf("Syntax error at character index %d\n",index);
    }
    )
    .start();
}
```

- Call `NuShellCommands.caseSensitive()` to your convenience. By default, command names are not case-sensitive.
- Call `on()` to provide a command name and the callback to be executed if such a command is found.
- Call `onUnknown()` to provide a callback to be executed if the command line does not contain any command name.
- Call `onParseError()` to provide a callback to be executed in case of error.
- You can chain calls to "`on*`" methods.
- Call `NuShellCommands.start()`.
- Note that all callbacks will be executed at the NimBLE OS task, so make them thread-safe.

Command line syntax:

- Blank spaces, LF and CR characters are separators.
- Command arguments are separated by one or more consecutive separators. For example, the command line `cmd   arg1  arg2 arg3\n` is parsed as the command "cmd" with three arguments: "arg1", "arg2" and "arg3", being `\n` the LF character. `cmd arg1\narg2\n\narg3` would be parsed just the same. Usually, LF and CR characters are command line terminators, so don't worry about them.
- Unquoted arguments can not contain a separator, but can contain double quotes. For example: `this"is"valid`.
- Quoted arguments can contain a separator, but double quotes have to be escaped with another double quote.
  For example: `"this ""is"" valid"` is parsed to `this "is" valid` as a single argument.
- ASCII, ANSI and UTF-8 character encodings are supported. Take into account that client software must use the same character encoding as your application.

As a bonus, you may use class `NuCLIParser` to implement a shell that takes data from other sources.

### Custom serial communications protocol

```c++
#include "NuS.hpp"

class MyCustomSerialProtocol: public NordicUARTService {
    public:
        void onWrite(NimBLECharacteristic *pCharacteristic) override;
    ...
}
```

Derive a new class and override `onWrite(NimBLECharacteristic *pCharacteristic)` (see [NimBLECharacteristicCallbacks::onWrite](https://h2zero.github.io/NimBLE-Arduino/class_nim_b_l_e_characteristic_callbacks.html)). Then, use `pCharacteristic` to read incoming data. For example:

```c++
void MyCustomSerialProtocol::onWrite(NimBLECharacteristic *pCharacteristic)
{
    // Retrieve a pointer to received data and its size
    NimBLEAttValue val = pCharacteristic->getValue();
    const uint8_t *receivedData = val.data();
    size_t receivedDataSize = val.size();

    // Custom processing here
    ...
}
```

In the previous example, the data pointed by `*receivedData` will **not remain valid** after `onWrite()` has finished to execute. If you need that data for later use, you must make a copy of the data itself, not just the pointer. For that purpose, you may store a non-local copy of the `pCharacteristic->getValue()` object.

Since just one object can use the Nordic UART Service, you should also implement a
[singleton pattern](https://www.geeksforgeeks.org/implementation-of-singleton-class-in-cpp/) (not mandatory).
