# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

For detailes on the library usage visit [documentation](documentation) section.

## Features

* Supports all Arduino platforms
* Operates in any combination of multiple instances of
  * [Modbus RTU server](examples/RTU)
  * [Modbus RTU client](examples/RTU)
  * Modbus TCP server for [ESP8266/ESP32](examples/TCP-ESP) and [Ethernet library](examples/TCP-Ethernet)
  * Modbus TCP client for [ESP8266/ESP32](examples/TCP-ESP) and [Ethernet library](examples/TCP-Ethernet)
  * [MODBUS/TCP Security server (ESP8266)](examples/TLS)
  * [MODBUS/TCP Security client (ESP8266/ESP32)](examples/TLS)
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
* [Callbacks](examples/Callback) driven design
* Real life complex examples:
  * [ESP8266/ESP32 firmware update over Modbus](examples/Files)
  * [ModbusRTU to ModbusTCP bridge](examples/Bridge)

## Notes

1. The offsets for registers are 0-based. So be careful when setting your supervisory system or your testing software. For example, in [ScadaBR](http://www.scadabr.com.br) offsets are 0-based, then, a register configured as 100 in the library is set to 100 in ScadaBR. On the other hand, in the [CAS Modbus Scanner](http://www.chipkin.com/products/software/modbus-software/cas-modbus-scanner/) offsets are 1-based, so a register configured as 100 in library should be 101 in this software.
2. RS-485 transivers based on MAX-485 is working on at least up to 115200. XY-017/XY-485 working only up to 9600 for some reason.

For more information about Modbus see:

* [Modbus (From Wikipedia, the free encyclopedia)](http://pt.wikipedia.org/wiki/Modbus)
* [MODBUS APPLICATION PROTOCOL SPECIFICATION V1.1b3](https://modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf)
* [MODBUS MESSAGING ON TCP/IP IMPLEMENTATION GUIDE V1.0b](http://www.modbus.org/docs/Modbus_Messaging_Implementation_Guide_V1_0b.pdf)
* [MODBUS over Serial Line Specification and Implementation Guide V1.02](http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf)
* [MODBUS/TCP Security Protocol Specification](https://modbus.org/docs/MB-TCP-Security-v21_2018-07-24.pdf)

## Last Changes

```diff
// 4.1.1
+ Protocol: Fix wrong error code response on non-existent register
+ ModbusTCP: Fix potential memory leak
+ API: cbEnable/cbDisable functionality extended
+ ESP-IDF: CMakeList.txt added
+ Examples: TCP-to-RTU fixed
// 4.1.0
+ API: Raw Modbus frame processing functionality
+ ModbusRTU: Precise inter-frame interval control
+ Examples: True ModbusRTU to ModbusTCP Server bridge
+ Examples: ModbusRTU respond to multiple ID from single device
+ ModbusRTU: Add direction control pin for Stream
+ STL: Add Reg count limitation to vector limit of 4000 (for ESP8266 and ESP32)
+ Settings: Added MODBUSIP_CONNECTION_TIMEOUT (ESP32 only)
+ Settings: Set MODBUSIP_MAX_CLIENTS = 8 for ESP32
+ ModbusTCP: Make using DNS names optional feature
+ ModbusRTU: Add separate RE/DE pins control optional feature
+ API: Drop support of Ethernet library v1
+ Examples: Teknic ClearCore ArduinoWrapper examples added
+ Examples: ModbusTCP to ModbusRTU example added
+ ModbusRTU: Flush extra delay optional feature
// 4.0.0
+ Support of all Arduino boards
+ ModbusTLS: ESP8266 Client/Server and ESP32 Client
+ ModbusTCP: ModbusEthernet - WizNet W5x00, ENC28J60 Ethernet library support
+ 0x14 - Read File Records function
+ 0x15 - Write File Records function
+ Examples: FW update over Modbus fullfunctional example
+ 0x16 - Write Mask Register function+ Test: 0x16
+ 0x17 - Read/Write Registers function
+ ModbusRTU: ESP32 SoftwareSerial support
+ Build with no STL dependency (switchable)
+ API: ModbusIP => ModbusTCP
+ API: Access control callback for individual Modbus function
+ API: Master/Slave => Client/Server according to [PRESS RELEASE](https://modbus.org/docs/Client-ServerPR-07-2020-final.docx.pdf)
+ Lot of code refacting and small fixes
```

## Roadmap

```diff
// 4.2.0
- API: Alternative CRC calulation (reduced memory footprint)
- ModbusRTU: Static buffer allocation
- Test: Frame accuracy to specefication
- Buffer/packet size limitation support
- Slave/Server: slavePDU use early exit by return where possible
- Master/Client: Check frame size against header data where possible
- Master/Client: Additional response data validation
- Free global registers and callbacks on remove last Modbus instance
- Test: push/pull functions
- ModbusTCP: Refactor connect by dns name (using native implementation for ESP32 etc)
// 4.3.0
- ModbusTLS: ESP32 Server
- Test: TLS ESP32 Server
- Test: TLS ESP32 Client
- Examples: TLS Certificate test Role extension and Alt-Name
- Examples: TLS Add example explanation
- ModbusTCP: ModbusAsyncTCP
- API: Extend API to allow custom Modbus commands
- Examples: Basic file operations
- Examples: Revising
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
