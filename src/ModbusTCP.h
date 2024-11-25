/*
    Modbus Library for Arduino
    ModbusTCP for ESP8266/ESP32
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include "ModbusAPI.h"
#include "ModbusTCPTemplate.h"

class WiFiServerESPWrapper : public WiFiServer {
  public:
    WiFiServerESPWrapper(uint16_t port) : WiFiServer(port) {}
    inline WiFiClient accept() {
        return WiFiServer::accept();
    }
};

class ModbusTCP : public ModbusAPI<ModbusTCPTemplate<WiFiServerESPWrapper, WiFiClient>> {
#if defined(MODBUSIP_USE_DNS)
  private:
    static IPAddress resolver(const char *host) {
        IPAddress remote_addr;
        if (WiFi.hostByName(host, remote_addr))
            return remote_addr;
        return IPADDR_NONE;
    }

  public:
    ModbusTCP() : ModbusAPI() {
        resolve = resolver;
    }
#endif
};
