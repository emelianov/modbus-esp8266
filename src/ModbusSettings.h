
/*
    Modbus Library for Arduino
    
    Copyright (C) 2019-2021 Alexander Emelianov (a.m.emelianov@gmail.com)
	https://github.com/emelianov/modbus-esp8266
	This code is licensed under the BSD New License. See LICENSE.txt for more info.
    
Prefixes:
MODBUS_     Global library settings
MODBUSIP_   Settings for TCP and TLS both
MODBUSTCP_  Settings for TCP
MODBUSTLS_  Settings for TLS
MODBUSRTU_  Settings for RTU
MODBUSAPI_  Settings for API
*/
#pragma once

/*
#define MODBUS_GLOBAL_REGS
If defined Modbus registers will be shared across all Modbus* instances.
If not defined each Modbus object will have own registers set.
*/
#define MODBUS_GLOBAL_REGS
//#define MODBUS_FREE_REGS

/*
#define ARDUINO_SAM_DUE_STL
Use STL with Arduino Due. Was able to use with Arduino IDE but not with PlatformIO
Also note STL issue workaround code in Modbus.cpp
*/
#if defined(ARDUINO_SAM_DUE)
//#define ARDUINO_SAM_DUE_STL
#endif

/*
#define MODBUS_USE_STL
If defined C STL will be used.
*/
#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_STM32) || defined(ARDUINO_SAM_DUE_STL)
#define MODBUS_USE_STL
#endif

/*
#define MODBUS_MAX_REGS     32
If defined regisers count will be limited.
*/ 
//#define MODBUS_MAX_REGS     32

#define MODBUS_ADD_REG
//#define MODBUS_STRICT_REG
#define MODBUS_MAX_FRAME   256
//#define MODBUS_STATIC_FRAME
#define MODBUS_MAX_WORDS 0x007D
#define MODBUS_MAX_BITS 0x07D0
#define MODBUS_FILES
#define MODBUSTCP_PORT 	  502
#define MODBUSTLS_PORT 	  802
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT 1000
#define MODBUSIP_UNIT	  255
#define MODBUSIP_MAX_TRANSACTIONS 16
#define MODBUSIP_MAX_CLIENTS	  4
#define MODBUSIP_UNIQUE_CLIENTS
#define MODBUSIP_MAX_READMS 100
#define MODBUSIP_FULL

//#define MODBUSRTU_DEBUG
#define MODBUSRTU_BROADCAST 0
#define MB_RESERVE 248
#define MB_SERIAL_BUFFER 128
#define MODBUSRTU_TIMEOUT 1000
#define MODBUSRTU_MAX_READMS 100

// Define for internal use. Do not change.
#define MODBUSRTU_TIMEOUT_US 1000UL * MODBUSRTU_TIMEOUT
#define MODBUSRTU_MAX_READ_US 1000UL * MODBUSRTU_MAX_READMS

#define MODBUSAPI_LEGACY
#define MODBUSAPI_OPTIONAL

#if defined(ARDUINO_UNO) || defined(ARDUINO_LEONARDO)
#undef MODBUS_MAX_REGS
#undef MODBUSIP_MAX_TRANSACTIONS
#undef MODBUS_MAX_WORDS
#undef MODBUS_MAX_BITS
#define MODBUS_MAX_REGS     32
#define MODBUSIP_MAX_TRANSACTIONS 4
#define MODBUS_MAX_WORDS 0x0020
#define MODBUS_MAX_BITS 0x0200
#endif