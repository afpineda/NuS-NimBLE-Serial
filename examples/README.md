# Nordic UART Service: Arduino demos

## Summary

- [NuSEcho.ino](./NuSEcho/NuSEcho.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it back to the same peer.
  This is called an "echo" service. The serial monitor is also feed with log messages.
  The device is advertised as "NuSerial Echo".
  Demonstrates the usage of non-blocking communications by the means of `NuSerial`.

  You may type `E` or `e` at the serial monitor to forcedly terminate current peer connection and wait for another.

- [NuSerialDump.ino](./NuSerialDump/NuSerialDump.ino)

  Runs a service that takes incoming data from a BLE peer and dumps it to the serial monitor.
  The device is advertised as "NuPacket demo".
  Demonstrates the usage of blocking communications by the means of `NuPacket`.

- [ReadBytesDemo.ino](./ReadBytesDemo/ReadBytesDemo.ino)

  As the previous one, runs a service that takes incoming data from a BLE peer and dumps it into the serial monitor.
  However, this example uses a fixed size buffer and does not care about packet size.
  The device is advertised as "ReadBytes demo".
  Demonstrates the usage of blocking communications by the means of `NuSerial.readBytes()` with no active waiting.

  Since incoming data is buffered, you have to send at least 4 characters or disconnect to see any output at the serial monitor.
  Note that the terminating LF character (`\n`) also counts.

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
  Demonstrates how to serve custom AT commands on NuS. The service works as a simple calculator.

  Supported commands (always follow AT command syntax):

  - `+A=<integer>`. Set the value of the first operand.
  - `+A?`. Get the value of the first operand.
  - `+B=<integer>`. Set the value of the second operand.
  - `+B?`. Get the value of the second operand.
  - `+OP=<integer>,<integer>`. Set the value of both operands.
  - `+OP?`. Get the value of both operands, A then B.
  - `+SUM` or `+SUM?`. Get the sum A+B.
  - `+SUB` or `+SUB?`. Get the subtraction A-B.
  - `+MULT` or `+MULT?`. Get the multiplication A*B.
  - `+DIV` or `+DIV?`. Get the division A/B.
  - `&V`. Get the version number.

  For example: `AT+OP=14,2;+DIV?`

- [ShellCommandDemo.ino](./ShellCommandDemo/ShellCommandDemo.ino)

  Runs a service that parses shell-like commands from a BLE peer and executes them.
  The serial monitor is also feed with log messages. The device is advertised as "Shell commands demo".
  Demonstrates how to serve shell commands on NuS. The service works as a simple calculator.

  Supported commands (one per line):

  - `sum <integer> <integer>`
  - `sub <integer> <integer>`
  - `mult <integer> <integer>`
  - `div <integer> <integer>`

  Replace `<integer>` with an integer number.

## Testing

In order to test those sketches, you need a serial terminal app compatible with NuS in your smartphone or PC. During development, this one was used (Android):
[Serial bluetooth terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal).

Configure LF (line feed, aka `\n`) as the line-terminating character.
