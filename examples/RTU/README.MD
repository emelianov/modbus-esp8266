This example introduces how to use the library for ModbusRTU (typicaly over RS-485) to act as [master](master) or [slave](slave). Additionally there is [example of master](ESP32-Concurent) device for multithread usage with ESP32.

## [Concurrent thread-safe access to Modbus object](ESP32-Concurent/ESP32-Concurent.ino)

## [Simple ModbusRTU master](master/master.ino)

## [Simple ModbusRTU slave](slave/slave.ino)

## [Sync ModbusRTU master](masterSync/masterSync.ino)

## Modbus RTU Specific API

```c
bool begin(SoftwareSerial* port, int16_t txEnablePin=-1, ModbusRTUTxEnableMode txEnableMode=TxEnableHigh);
bool begin(HardwareSerial* port, int16_t txEnablePin=-1, ModbusRTUTxEnableMode txEnableMode=TxEnableHigh);
bool begin(Stream* port);
```

- `port`    Pointer to Serial port
- `txEnablePin`   RX/TX control pin. Not assigned (assume auto RX/TX) by default
- `txEnablePin` controls transmit enable for MAX-485. Use `txEnableMode = TxEnableLow` if `txEnablePin` uses inverse logic (low when transmitting).

Assign Serial port. txEnablePin controls transmit enable for MAX-485.

```c
void setBaudrate(uint32 baud);
```

- `baud`    New baudrate.

Set or override Serial baudrate. Must be called after .begin() for Non-ESP devices.

```c
void client();
void server(uint8_t slaveId);
void slave(); // Deprecated
void master(uint8_t slaveId); // Deprecated
```

- `slaveId` Modbus slave id to associate to.

Select and initialize master or slave mode to work. Switching between modes is not supported. Call is not returning error in this case but behaviour is unpredictable.

```c
uint8_t client();
uint8_t slave(); // Deprecated
```

- Slave mode: Returns configured slave id.
- Master mode: Returns slave id for active request or 0 if no request in progress.

# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2020 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.
