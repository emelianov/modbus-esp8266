/*
    Modbus Library for Arduino
	ModbusRTU
    Copyright (C) 2019-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/
#pragma once
#include "ModbusAPI.h"

class ModbusRTUTemplate : public Modbus {
    protected:
        Stream* _port;
        int16_t   _txPin = -1;
		bool _direct = true;	// Transmit control logic (true=direct, false=inverse)
		uint32_t _t;	// inter-frame delay in uS
		uint32_t t = 0;		// time sience last data byte arrived
		bool isMaster = false;
		uint8_t  _slaveId;
		uint32_t _timestamp = 0;
		cbTransaction _cb = nullptr;
		uint8_t* _data = nullptr;
		uint8_t* _sentFrame = nullptr;
		TAddress _sentReg = COIL(0);
		uint16_t maxRegs = 0x007D;

		uint16_t send(uint8_t slaveId, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
		// Prepare and send ModbusRTU frame. _frame buffer and _len should be filled with Modbus data
		// slaveId - slave id
		// startreg - first local register to save returned data to (miningless for write to slave operations)
		// cb - transaction callback function
		// data - if not null use buffer to save returned data instead of local registers
		bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
		bool cleanup(); 	// Free clients if not connected and remove timedout transactions and transaction with forced events
		uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
    public:
		void setBaudrate(uint32_t baud = -1);
		uint32_t calculateMinimumInterFrameTime(uint32_t baud, uint8_t char_bits = 11);
		void setInterFrameTime(uint32_t t_us);
		template <class T>
		bool begin(T* port, int16_t txPin = -1, bool direct = true);
		bool begin(Stream* port);
        void task();
		void client() { isMaster = true; };
		inline void master() {client();}
		void server(uint8_t serverId) {_slaveId = serverId;};
		inline void slave(uint8_t slaveId) {server(slaveId);}
		uint8_t server() { return _slaveId; }
		inline uint8_t slave() { return server(); }
		uint32_t eventSource() override {return _slaveId;}
};

template <class T>
bool ModbusRTUTemplate::begin(T* port, int16_t txPin, bool direct) {
    uint32_t baud = 0;
    #if defined(ESP32) || defined(ESP8266) // baudRate() only available with ESP32+ESP8266
    baud = port->baudRate();
    #else
    baud = 9600;
    #endif
	setInterFrameTime(calculateMinimumInterFrameTime(baud));
    _port = port;
    if (txPin >= 0) {
	    _txPin = txPin;
		_direct = direct;
        pinMode(_txPin, OUTPUT);
        digitalWrite(_txPin, _direct?LOW:HIGH);
    }
    return true;
}

class ModbusRTU : public ModbusAPI<ModbusRTUTemplate> {};
