## Common API

```c
void task();
```

Processing routine. Should be periodically called form loop().

## Server API

### Add registers

```c
bool addHreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
bool addCoil(uint16_t offset, bool value = false, uint16_t numregs = 1);
bool addIsts(uint16_t offset, bool value = false, uint16_t numregs = 1);
bool addIreg(uint16_t offset, uint16_t value = 0, uint16_t numregs = 1);
```

### Write local reg

```c
bool Hreg(uint16_t offset, uint16_t value);
bool Coil(uint16_t offset, bool value);
bool Ists(uint16_t offset, bool value);
bool Ireg(uint16_t offset, uint16_t value);
```

### Read local reg

```c
uint16_t Hreg(uint16_t offset);
bool Coil(uint16_t offset);
bool Ists(uint16_t offset);
uint16_t Ireg(uint16_t offset);
```

### Remove reg

```c
bool removeHreg(uint16_t offset, uint16_t numregs = 1);
bool removeCoil(uint16_t offset, uint16_t numregs = 1);
bool removeIsts(uint16_t offset, uint16_t numregs = 1);
bool removeIreg(uint16_t offset, uint16_t numregs = 1);
```

### Modbus RTU Specific API

```c
bool begin(SoftwareSerial* port, int16_t txPin=-1, bool direct=true);
bool begin(HardwareSerial* port, int16_t txPin=-1, bool direct=true);
bool begin(Stream* port);
```

Assing Serial port. txPin controls transmit enable for MAX-485. Pass direct=false if txPin uses inverse logic.

```c
void setBaudrte(uint32 baud);
```

Set or override Serial baudrate. Must be called after .begin() for Non-ESP devices.

```c
void server(uint8_t slaveId);
void slave(uint8_t slaveId); //Depricated
```

Select and initialize master or slave mode to work. Switching between modes is not supported. Call is not returning error in this case but behaviour is unpredictible.

```c
uint8_t server();
uint8_t slave(); //Depricated
```

Slave mode: Returns configured slave id. Master mode: Returns slave id for active request or 0 if no request in-progress.

## Modbus TCP Server specific API

```c
void begin(); // Depricated. Use server() instead.
void slave(uint16_t port = MODBUSIP_PORT); // For compatibility with ModbusRTU calls. Typically may be replaced with server() call.
void server(uint16_t port = MODBUSIP_PORT);
```

## Modbus TCP Client specific

```c
void master(); // For compatibility with ModbusRTU calls. Typically may be replaced with client() call.
void client();
bool connect(IPAddress ip, uint16_t port = MODBUSIP_PORT);
bool disconnect(IPAddress ip);
bool isTransaction(uint16_t id);
bool isConnected(IPAddress ip);
void dropTransactions();
```

```c
void autoConnect(bool enabled);
```

Select behavior of executing read/write/pull/push. If autoConnect disabled (default) execution returns error if connection to slave is not already established. If autoConnect is enabled trying to establish connection during read/write/pull/push function call. Disabled by default.

## Client API

### Read Coils (0x01) from slave/server

```c
uint16_t readCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t readCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(const char* host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readCoil(String host, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);

uint16_t pullCoil(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pullCoilToIsts(uint8_t slaveId, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pullCoil(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pullCoilToIsts(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
```

### Write single Coil to slave/server

```c
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool value, cbTransaction cb = nullptr);
uint16_t writeCoil(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeCoil(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);

uint16_t pushCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pushCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
```

### Read Ists (0x02) from slave/server

```c
uint16_t readIsts(uint8_t slaveId, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t readIsts(IPAddress ip, uint16_t offset, bool* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);

uint16_t pullIsts(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pushIstsToCoil(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);

uint16_t pullIsts(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pushIstsToCoil(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
```

```c
uint16_t pullHreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pullIreg(IPAddress ip, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pullHregToIreg(IPAddress ip, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pullHreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pullIreg(uint8_t slaveId, uint16_t from, uint16_t to, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pullHregToIreg(uint8_t slaveId, uint16_t offset, uint16_t startreg, uint16_t numregs = 1, cbTransaction cb = nullptr);
```

Result is saved to local registers. Method returns corresponding transaction id. [ip/from] or [ip/offset] - slave, [to] or [startreg] - local

### Send [multiple] regs to remote slave/server

```c
uint16_t pushHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t pushIregToHreg(IPAddress ip, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);

uint16_t pushHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t pushIregToHreg(uint8_t slaveId, uint16_t to, uint16_t from, uint16_t numregs = 1, cbTransaction cb = nullptr);
```

Write Register/Coil or Write Multiple Registers/Coils Modbus function selected automaticly depending on 'numregs' value. [ip/to] - slave, [from] - local

### Write [multiple] values to remote slave/servr reg[s]

```c
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t value, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t value, cbTransaction cb = nullptr);
```

Writes single value to remote Hreg/Coil.

```c
uint16_t writeHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t writeHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
```

Writes multiple values from array to remote Coil/Hreg.

### Read values from multiple remote slave/server regs

```c
uint16_t readHreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);
uint16_t readIreg(IPAddress ip, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr, uint8_t uint = MODBUSIP_UNIT);

uint16_t readHreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
uint16_t readIreg(uint8_t slaveId, uint16_t offset, uint16_t* value, uint16_t numregs = 1, cbTransaction cb = nullptr);
```

Reads values from remote Hreg/Coil/Ireg/Ists to array.

## Callbacks API

```c
void cbEnable(bool state = true);
void cbDisable();
```

Callback generation control. Callback generation is enabled by default. *Has no effect on transactions callbacks.*

```c
void onConnect(cbModbusConnect cb);
void onDisonnect(cbModbusConnect cb);
```

*Modbus TCP Server* Assign callback function on new incoming connection event.

```c
typedef bool (*cbModbusConnect)(IPAddress ip);
```

*Modbus TCP Sserver* Connect event callback function definition. For onConnect event client's IP address is passed as argument. onDisconnect callback function always gets INADDR_NONE as parameter.

```c
typedef uint16_t (*cbModbus)(TRegister* reg, uint16_t val);
```

Get/Set register callback function definition. Pointer to TRegister structure (see source for details) of the register and new value are passed as arguments.

```c
typedef bool (*cbTransaction)(Modbus::ResultCode event, uint16_t transactionId, void* data);
```

Transaction end callback function definition. For ModbusIP *data* is currently reserved. For Modbus RTU *transactionId* is also reserved.

```c
uint32_t eventSource();
```

Should be called from onGet/onSet or transaction callback function.

*Modbus TCP Client/Server* Returns IP address of remote requesting operation or INADDR_NONE for local. Use IPAddress(eventSource) to operate result as IPAddress type.

*Note:* For transaction callback INADDR_NONE returned in case if transaction is timedout.

*Modbus RTU Master/Slave* Returns slave id. 

```c
bool onSetCoil(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetHreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetIsts(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onSetIreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
```

Assign callback function on register modify event. Multiple sequental registers can be affected by specifing `numregs` parameter.

```c
bool onGetCoil(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetHreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetIsts(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
bool onGetIreg(uint16_t address, cbModbus cb = nullptr, uint16_t numregs = 1);
```

Assign callback function on register query event. Multiple sequental registers can be affected by specifing `numregs` parameter.

```c
bool removeOnGetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetCoil(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetHreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetIsts(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnGetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
bool removeOnSetIreg(uint16_t offset, cbModbus cb = nullptr, uint16_t numregs = 1);
```

Disconnect specific callback function or all callbacks of the type if cb=NULL.

### Macros

```c
#define COIL_VAL(v)
#define COIL_BOOL(v)
#define ISTS_VAL(v)
#define ISTS_BOOL(v)
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
