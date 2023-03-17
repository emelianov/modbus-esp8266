# ESP8266/ESP32 TCP Examples

## [Basic client](client/client.ino)

## [Client with blocking read operation](clientSync/clientSync.ino)

## [Server](server/server.ino)

### API

```c
void client();
```

Initialize internal structures to act as a Modbus client.

```c
bool connect(IPAddress ip, uint16_t port = MODBUSIP_PORT);
bool disconnect(IPAddress ip);
```

- `ip`  IP address of the remote Modbus server
- `port`    TCP port of remote Modbus server (standard value is 502)

Note: Just one connection to the specific address is supported. That is the library is unable to simultaniousaly connect to Modbus servers that has same IP address but diffrent ports.

```c
uint16_t readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(const char* host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(String host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
```

- `ip` IP Address of Modbus server to get registers from
- `host` Hostname fo Modbus server to get registers from
- `offset` Address of first Modbus register to read/write
- `numregs` Count of registers to read/write
- `cb` Transaction callback function (see [Calback examples](../calback) for details). `NULL` if not used
- `unit` Modbus unit

Sends corresponding Modbus read request to Modbus server at `ip`. Connection with server shoud be already established by connect(ip).
Returns transaction `id` or `0` on failure. Failure maens that client unable to send the request bacause of no connection to the Modbus server is established or other internal error.
Note: read/write functions just sending requests to remote Modbus server. The functions returns immediate after request sent and doesn't waiting for result. That is `value` contains no result data on the function exit. `value` will be filled as responce arrive and processed by .task() function.

```c
bool isTransaction(uint16_t id);
```

- `id`  Transaction id.

Returns `true` if transaction with `id` is active.

```c
bool isConnected(IPAddress ip);
```

- `ip`    Remote Modbus server IP address

Returns `true` is connection with Modbus server at `ip` is established.

```c
void dropTransactions();
```

Cancel all active transactions. Callback with result code `Modbus::EX_CANCEL` will be called for each transaction (if assigned).

### Add local register
```c
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1);
bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1);
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t nemregs = 1);
```

- `offset` Address of the first register to add
- `value` Initial value to be assigned to register(s)
- `numregs` Count of registers to be created

Adding new register(s) and assigning value(s). If [some] registers already exists value will be updated.
Returns `true` on success. `false` if operation is failed for some reason.

### Write local reg

```c
bool Hreg(uint16_t offset, uint16_t value);
bool Coil(uint16_t offset, bool value);
bool Ists(uint16_t offset, bool value);
bool Ireg(uint16_t offset, uint16_t value);
```

- `offset` Address of the register
- `value` Value to be assigned to register

Returns `true` on success. `false` if register not previousely added or other error.

### Read local reg

```c
uint16_t Hreg(uint16_t offset);
bool Coil(uint16_t offset);
bool Ists(uint16_t offset);
uint16_t Ireg(uint16_t offset);
```

- `offset` Address of the register to read


Returns current value of the register.

### Remove reg(s)

```c
bool removeHreg(uint16_t offset, uint16_t numregs = 1);
bool removeCoil(uint16_t offset, uint16_t numregs = 1);
bool removeIsts(uint16_t offset, uint16_t numregs = 1);
bool removeIreg(uint16_t offset, uint16_t numregs = 1);
```

- `offset` Address of the first register to remove
- `numregs` Count of registers to be created

Function trying to remove `numregs` registers starting from `offset`. If some of registers within the range are not exists removal continues execution.
Returns `true` if atleast one register in the range was removed.

# Modbus Library for Arduino
### ModbusRTU, ModbusTCP and ModbusTCP Security

(c)2020 [Alexander Emelianov](mailto:a.m.emelianov@gmail.com)

The code in this repo is licensed under the BSD New License. See LICENSE.txt for more info.