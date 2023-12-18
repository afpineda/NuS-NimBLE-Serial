# Nordic UART Service: Arduino demos

## Summary

- [NuSEcho.ino](./NuSEcho/NuSEcho.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it back to the same peer.
  This is called an "echo" service. The serial monitor is also feed with log messages.
  The device is advertised as "NuSerial Echo".
  Demonstrates the use of non-blocking communications by the means of `NuSerial`.

- [NuSerialDump.ino](./NuSerialDump/NuSerialDump.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it into the serial monitor.
  The device is advertised as "NuStream demo".
  Demonstrates the use of blocking communications by the means of `NuStream`.

- [CustomCommandProcessor.ino](./CustomCommandProcessor/CustomCommandProcessor.ino)

  Runs a service that parses commands and executes them. The serial monitor is also feed with log messages. The device is advertised as "Custom commands demo".
  Demonstrates how to write a custom protocol based on NuS.

  Commands and their syntax:

  - `exit`: forces the service to disconnect.
  - `sum <int> <int>`: retrieve the sum of two integer numbers. Substitute `<int>` with those numbers.

  All commands are lower-case. Arguments must be separated by blank spaces.

## Testing

In order to test those sketches, you need a serial terminal app compatible with NuS in your smartphone or PC. During development, this one was used (Android):
[Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal).
