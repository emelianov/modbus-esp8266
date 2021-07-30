# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

|If the library is helpful for your projects you can support it by a glass of beer|[![paypal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_SM.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=Z38SLGAKGM93S&source=url)|
|---|---|


The library is under active development. Visit [Releases](https://github.com/emelianov/modbus-esp8266/releases) page for stable one.

---

This library allows your Arduino board to communicate via Modbus protocol. The Modbus is a protocol
used in industrial automation and also can be used in other areas, such as home automation.

The Modbus generally uses serial RS-485 as physical layer (then called Modbus Serial) and TCP/IP via Ethernet or WiFi (Modbus TCP).

## Features

* Supports all Arduino platforms
* Operates in any combination of multiple instances of
  * [Modbus RTU server](examples/RTU)
  * [Modbus RTU client](examples/RTU)
  * Modbus TCP server for [ESP8266/ESP32](examples/TCP-ESP) and [Ethernet library](examples/TCP-Ethernet)
  * Modbus TCP client for [ESP8266/ESP32](examples/TCP-ESP) and [Ethernet library](examples/TCP-Ethernet)
  * [MODBUS/TCP Security server (ESP8266)](examples/TLS)
  * [MODBUS/TCP Security client (ESP8266/ESP32)](examples/TLS)
* Reply exception messages for all supported functions
* Modbus functions supported:
  * 0x01 - Read Coils
  * 0x02 - Read Input Status (Read Discrete Inputs)
  * 0x03 - Read Holding Registers
  * 0x04 - Read Input Registers
  * 0x05 - Write Single Coil
  * 0x06 - Write Single Register
  * 0x0F - Write Multiple Coils
  * 0x10 - Write Multiple Registers
  * 0x14 - Read File Record
  * 0x15 - Write File Record
  * 0x16 - Mask Write Register
  * 0x17 - Read/Write multiple registers
* [Callbacks](examples/Callback) for
  * Client connect (Modbus TCP)
  * Server/Client disconnect (Modbus TCP)
  * Read specific Register
  * Write specific Register
  * Transaction result (Client side)
  * Transaction start/end (Server side)

## Notes

1. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in [ScadaBR](http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the [CAS Modbus Scanner](http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.
2. For API refer [API.md](https://github.com/emelianov/modbus-esp8266/blob/master/API.md)
3. RS-485 transivers based on MAX-485 is working on at least up to 115200. XY-017/XY-485 working only up to 9600 for some reason.

For more information about Modbus see:

* [Modbus (From Wikipedia, the free encyclopedia)](http://pt.wikipedia.org/wiki/Modbus)
* [MODBUS APPLICATION PROTOCOL SPECIFICATION V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
* [MODBUS MESSAGING ON TCP/IP IMPLEMENTATION GUIDE V1.0b](http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf)
* [MODBUS over Serial Line Specification and Implementation Guide V1.02](http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf)
* [MODBUS/TCP Security Protocol Specification](https://modbus.org/docs/MB-TCP-Security-v21_2018-07-24.pdf)

## Last Changes

```diff
// 4.0.0-DEV
+ ModbusTLS: Modbus TCP Security Client/Server
+ ModbusTLS: ESP8266 Client/Server
+ Test: TLS ESP8266 Client/Server
+ Examples: TLS added
- Examples: TLS Certificate test Role extension and Alt-Name
- Examples: TLS Add example explanation
+ ModbusTLS: ESP32 Client
- Test: TLS ESP32 Client
+ Build with no STL dependency
+ Test: No-STL mode
+ ModbusTCP: ModbusEthernet - W5x00 Ethernet library support
+ Test: W5x00 support
- Test: W5x00 with Ethernet library v1
+ API: Implementation code merge
+ API: Access ModbusTCP server by name
+ API: Set local multiple registers from an array
+ ModbusIP => ModbusTCP
+ 0x14 - Read File Records function
+ Test: 0x14
+ 0x15 - Write File Records function
+ Test: 0x15
- Examples: Basic file operations
- Examples: FW update
+ 0x16 - Write Mask Register function
- Test: 0x16
+ 0x17 - Read/Write Registers function
- Test: 0x17
+ API: Access control callback for individual Modbus function
- Slave/Server: slavePDU use early exit by return where possible
- Master/Client: Check frame size against header data where possible
- Master/Client: Additional responce data validation
- Test: push/pull functions
- Test: Frame accuracy to specefication
- Documentation: Update
- Examples: Revising
+ Remove unneeded register count check in private functions
+ Check startreg + numreg < 65535
+ ModbusRTU: ESP32 SoftwareSerial support
+ ModbusRTU: Fix transaction callback remains assigned after request end
+ ModbusTCP: Free server connection in destructor
+ Declare global registers and callbacks as static members
- Free global registers and callbacks on remove last Modbus instance
+ ModbusRTU: Refactor .task() for more relaibe processing of incoming data
- ModbusRTU: Static buffer allocation.
- Buffer/packet size limitation support
+ API: Declare all callbacks as std::function (for STL)
- API: Master/Slave => Client/Server according to [PRESS RELEASE](https://modbus.org/docs/Client-ServerPR-07-2020-final.docx.pdf)
// 4.1.0
- ModbusTLS: ESP32 Server
- Test: TLS ESP32 Server
```

## Contributions

https://github.com/emelianov/modbus-esp8266

a.m.emelianov@gmail.com

Original version:

https://github.com/andresarmento/modbus-esp8266

https://github.com/andresarmento/modbus-arduino

prof (at) andresarmento (dot) com

## License

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
