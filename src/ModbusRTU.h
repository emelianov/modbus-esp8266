/*
    Modbus Library for Arduino
	ModbusRTU
    Copyright (C) 2019-2022 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#pragma once
#include "ModbusAPI.h"

class ModbusRTUTemplate : public Modbus {
    protected:
        Stream* _port;
        int16_t   _txEnablePin = -1;
#if defined(MODBUSRTU_REDE)
        int16_t   _rxPin = -1;
#endif
		bool _direct = true;	// Transmit control logic (true=txEnableDirect, false=inverse)
		uint32_t _t;	// inter-frame delay in uS
#if defined(MODBUSRTU_FLUSH_DELAY)
		uint32_t _t1;	// char send time
#endif
		uint32_t t = 0;		// time sience last data byte arrived
		bool isMaster = false;
		uint8_t  _slaveId;
		uint32_t _timestamp = 0;
		cbTransaction _cb = nullptr;
		uint8_t* _data = nullptr;
		uint8_t* _sentFrame = nullptr;
		TAddress _sentReg = COIL(0);
		uint16_t maxRegs = MODBUS_MAX_WORDS;
		uint8_t address = 0;
		bool _cbRawIncludeCrcErrors = false;

		uint16_t send(uint8_t slaveId, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
		// Prepare and send ModbusRTU frame. _frame buffer and _len should be filled with Modbus data
		// slaveId - slave id
		// startreg - first local register to save returned data to (miningless for write to slave operations)
		// cb - transaction callback function
		// data - if not null use buffer to save returned data instead of local registers
		bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
		bool cleanup(); 	// Free clients if not connected and remove timedout transactions and transaction with forced events
		uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
		uint16_t crc16_alt(uint8_t address, uint8_t* frame, uint8_t pduLen);
    public:
		void setBaudrate(uint32_t baud = -1);
		uint32_t calculateMinimumInterFrameTime(uint32_t baud, uint8_t char_bits = 11);
		void setInterFrameTime(uint32_t t_us);
		uint32_t charSendTime(uint32_t baud, uint8_t char_bits = 11);
		template <class T>
		bool begin(T* port, int16_t txEnablePin = -1, bool txEnableDirect = true);
#if defined(MODBUSRTU_REDE)
		template <class T>
		bool begin(T* port, int16_t txEnablePin, int16_t rxEnablePin, bool txEnableDirect);
#endif
		bool begin(Stream* port, int16_t txEnablePin = -1, bool txEnableDirect = true);
        void task();
		void client() { isMaster = true; };
		inline void master() {client();}
		void server(uint8_t serverId) {_slaveId = serverId;};
		inline void slave(uint8_t slaveId) {server(slaveId);}
		uint8_t server() { return _slaveId; }
		inline uint8_t slave() { return server(); }
		uint32_t eventSource() override {return address;}
		// Overload
        bool onRaw(cbRaw cb = nullptr, bool includeCrcErrors = false);
};

template <class T>
bool ModbusRTUTemplate::begin(T* port, int16_t txEnablePin, bool txEnableDirect) {
    uint32_t baud = 0;
    #if defined(ESP32) || defined(ESP8266) // baudRate() only available with ESP32+ESP8266
    baud = port->baudRate();
    #else
    baud = 9600;
    #endif
	setInterFrameTime(calculateMinimumInterFrameTime(baud));
#if defined(MODBUSRTU_FLUSH_DELAY)
	_t1 = charSendTime(baud);
#endif
    _port = port;
    if (txEnablePin >= 0) {
	    _txEnablePin = txEnablePin;
		_direct = txEnableDirect;
        pinMode(_txEnablePin, OUTPUT);
        digitalWrite(_txEnablePin, _direct?LOW:HIGH);
    }
    return true;
}
#if defined(MODBUSRTU_REDE)
template <class T>
bool ModbusRTUTemplate::begin(T* port, int16_t txEnablePin, int16_t rxEnablePin, bool txEnableDirect) {
	begin(port, txEnablePin, txEnableDirect);
	if (rxEnablePin > 0) {
		_rxPin = rxEnablePin;
        pinMode(_rxPin, OUTPUT);
        digitalWrite(_rxPin, _direct?LOW:HIGH);
	}
	return true;
}
#endif
class ModbusRTU : public ModbusAPI<ModbusRTUTemplate> {};
