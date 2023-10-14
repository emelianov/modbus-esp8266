# FAQ

This library allows your Arduino board to communicate via Modbus protocol. The Modbus is a protocol
used in industrial automation and also can be used in other areas, such as home automation.

The Modbus generally uses serial RS-485 as physical layer (then called Modbus Serial) and TCP/IP via Ethernet or WiFi (Modbus TCP and Modbus TCP Security).

---

## Where to get documentation for the library?

- [API](API.md)
- [ModbusTCP](https://github.com/emelianov/modbus-esp8266/tree/master/examples/TCP-ESP#API)
- [ModbusRTU](https://github.com/emelianov/modbus-esp8266/tree/master/examples/RTU#Modbus-RTU-Specific-API)
- [Callbacks](https://github.com/emelianov/modbus-esp8266/tree/master/examples/Callback/#Callback-API)
- [Modbus Security](https://github.com/emelianov/modbus-esp8266/tree/master/examples/TLS)
- [Modbus File operations](https://github.com/emelianov/modbus-esp8266/tree/master/examples/Files#File-block-API)
- [Compile time settings](https://github.com/emelianov/modbus-esp8266/tree/master/src/ModbusSettings.h))

---

## Client work cycle diagram

![Client diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/client.png)

---

## Server work cycle diagram 

![Server diagram](https://github.com/emelianov/modbus-esp8266/blob/master/resources/server.png)

---

## How to send signed value (`int16_t`)?

## How to send `float` or `uint32_t` values?

Modbus standard defines only two types of data: bit value and 16-bit value. All other datatypes should be sent as multiple 16-bit values.

---

## Value not read after `readCoil`/`readHreg`/etc

The library is designed to execute calls async way. That is `readHreg()` function just sends read request to Modbus server device and exits. Response is processed (as suun as it's arrive) by `task()`. `task()` is also async and exits if data hasn't arrive yet.  

---

## When calling `readCoil`/`readHreg`/`writeHreg`/etc multiple times only first of them executed

---

## Transactional callback returns *0xE4* error

It's timeout error. Suggestions below are applicable to persistent errors or frequently errors. Rare timeout errors may be normal in some considerations.

### ModbusRTU

Typically is indicates some kind of wiring or hardware problems.

- Check wiring.
- Check that baudrate settings are identical for client and server.
- Try to reduce it to 9600bps.
- Try to use different power source for Arduino device.
- Try to replace RS-485 tranceiver.
- If using Modbus simulator software on PC check the result with alternative software.

### ModbusTCP

It maybe network problems. Use standard procedures as `ping` and firewall settings checks for diagnostics.

---

## If it's possible to create ModbusTCP to ModbusRTU pass through bridge?

Some ideas to implement full functional brodge may be taken from [this code](https://github.com/emelianov/modbus-esp8266/issues/101#issuecomment-755419095).
Very limited implementation is available in [example](https://github.com/emelianov/modbus-esp8266/examples/bridge).

---

# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2021 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
