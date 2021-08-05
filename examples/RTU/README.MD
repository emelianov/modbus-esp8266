This example is introduces how to use the library for ModbusRTU (typicaly over RS-485) to act as [master](master) or [slave](slave). Additionally there is [example of master](ESP32-Concurent) device for multithread usage with ESP32.

## [Concurent thread-safe access to Modbus object](ESP32-Concurent)

## [Simple ModbusRTU master](master)

## [Simple ModbusRTU slave](slave)

## [Sync ModbusRTU master](masterSync)

## Modbus RTU Specific API

```c
bool begin(SoftwareSerial* port, int16_t txPin=-1, bool direct=true);
bool begin(HardwareSerial* port, int16_t txPin=-1, bool direct=true);
bool begin(Stream* port);
```

- `port`    Pointer to Serial port
- `txPin`   RX/TX control pin. Not assigned (assume auto RX/TX) by default
- `direct`  Direct (true, default) or inverse (false) RX/TX pin control.

Assing Serial port. txPin controls transmit enable for MAX-485.

```c
void setBaudrte(uint32 baud);
```

- `baud`    New baudrate.

Set or override Serial baudrate. Must be called after .begin() for Non-ESP devices.

```c
void client();
void server(uint8_t slaveId);
void slave(); // Depricated
void master(uint8_t slaveId); // Depricated
```

- `slaveId` Modbus slave id to associate to.

Select and initialize master or slave mode to work. Switching between modes is not supported. Call is not returning error in this case but behaviour is unpredictible.

```c
uint8_t client();
uint8_t slave(); // Depricated
```

- Slave mode: Returns configured slave id.
- Master mode: Returns slave id for active request or 0 if no request in-progress.

# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2020 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.