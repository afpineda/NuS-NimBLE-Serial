# Nordic UART Service: Arduino demos

## Summary

- [NuSEcho.ino](./NuSEcho/NuSEcho.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it back to the same peer.
  This is called an "echo" service. The serial monitor is also feed with log messages.
  The device is advertised as "NuSerial Echo".
  Demonstrates the usage of non-blocking communications by the means of `NuSerial`.

- [NuSerialDump.ino](./NuSerialDump/NuSerialDump.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it into the serial monitor.
  The device is advertised as "NuStream demo".
  Demonstrates the usage of blocking communications by the means of `NuStream`.

- [CustomCommandProcessor.ino](./CustomCommandProcessor/CustomCommandProcessor.ino)

  Runs a service that parses incoming commands from a BLE peer and executes them.
  The serial monitor is also feed with log messages. The device is advertised as "Custom commands demo".
  Demonstrates how to write a custom protocol based on NuS.

  Commands and their syntax:

  - `exit`: forces the service to disconnect.
  - `sum <int> <int>`: retrieve the sum of two integer numbers. Substitute `<int>` with those numbers.

  All commands are lower-case. Arguments must be separated by blank spaces.

- [ATCommandDemo.ino](./ATCommandDemo/ATCommandDemo.ino)

  Runs a service that parses incoming AT commands from a BLE peer and executes them.
  The serial monitor is also feed with log messages. The device is advertised as "AT commands demo".
  Demonstrates how to write an AT command processor based on NuS. The service works as a simple calculator.

  Supported commands (always follow AT command syntax):

  - `+OP1=<integer>`. Set the value of the first operand.
  - `+OP1?`. Get the value of the first operand.
  - `+OP2=<integer>`. Set the value of the second operand.
  - `+OP2?`. Get the value of the second operand.
  - `+OP=<integer>,<integer>`. Set the value of both operands.
  - `+OP?`. Get the value of both operands, in order.
  - `+SUM` or `+SUM?`. Get the sum OP1+OP2.
  - `+SUB` or `+SUB?`. Get the subtraction OP1-OP2.
  - `+MULT` or `+MULT?`. Get the multiplication OP1*OP2.
  - `+DIV` or `+DIV?`. Get the division OP1/OP2.
  - `&V`. Get the version number.

  For example: `AT+OP=14,2;+DIV?`

## Testing

In order to test those sketches, you need a serial terminal app compatible with NuS in your smartphone or PC. During development, this one was used (Android):
[Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal).
