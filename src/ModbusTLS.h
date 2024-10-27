/*
    Modbus Library for Arduino
    ModbusTLS - ModbusTCP Security for ESP8266
    Copyright (C) 2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/
#pragma once
#if !defined(ESP8266) && !defined(ESP32)
#error Unsupported architecture
#endif
#include <WiFiClientSecure.h>
#if defined(ESP8266)
#include <WiFiServerSecure.h>
#else
// Just emty stub
class WiFiServerSecure {
public:
    WiFiServerSecure(uint16_t){}
    WiFiClientSecure available(){}
    void begin();
    inline WiFiClientSecure accept() {
        return available();
    }
};
#endif
#include "ModbusTCPTemplate.h"
#include "ModbusAPI.h"

class ModbusTLS : public ModbusAPI<ModbusTCPTemplate<WiFiServerSecure, WiFiClientSecure>> {
    private:
    int8_t _connect(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr) {
	    int8_t p = getFreeClient();
	    if (p < 0)
		    return p;
	    tcpclient[p] = new WiFiClientSecure();
        BIT_CLEAR(tcpServerConnection, p);
        #if defined(ESP8266)
        BearSSL::X509List *clientCertList = new BearSSL::X509List(client_cert);
        BearSSL::PrivateKey *clientPrivKey = new BearSSL::PrivateKey(client_private_key);
        tcpclient[p]->setClientRSACert(clientCertList, clientPrivKey);
        tcpclient[p]->setBufferSizes(512, 512);
        #else
        tcpclient[p]->setCertificate(client_cert);
        tcpclient[p]->setPrivateKey(client_private_key);
        #endif
        return p;
    }
#if defined(MODBUSIP_USE_DNS)
    static IPAddress resolver (const char* host) {
        IPAddress remote_addr;
        if (WiFi.hostByName(host, remote_addr))
                return remote_addr;
        return IPADDR_NONE;
    }
#endif
    public:
    ModbusTLS() : ModbusAPI() {
        defaultPort = MODBUSTLS_PORT;
#if defined(MODBUSIP_USE_DNS)
        resolve = resolver;
#endif
    }
    #if defined(ESP8266)
	void server(uint16_t port, const char* server_cert = nullptr, const char* server_private_key = nullptr, const char* ca_cert = nullptr) {
        serverPort = port;
	    tcpserver = new WiFiServerSecure(serverPort);
        BearSSL::X509List *serverCertList = new BearSSL::X509List(server_cert);
        BearSSL::PrivateKey *serverPrivKey = new BearSSL::PrivateKey(server_private_key);
        tcpserver->setRSACert(serverCertList, serverPrivKey);
        if (ca_cert) {
            BearSSL::X509List *trustedCA = new BearSSL::X509List(ca_cert);
            tcpserver->setClientTrustAnchor(trustedCA);
        }
        //tcpserver->setBufferSizes(512, 512);
	    tcpserver->begin();
    }

    bool connectWithKnownKey(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* key = nullptr) {
        if(getSlave(ip) >= 0)
		    return true;
        int8_t p = _connect(ip, port, client_cert, client_private_key);
        BearSSL::PublicKey *clientPublicKey = new BearSSL::PublicKey(key);
        tcpclient[p]->setKnownKey(clientPublicKey);
        return tcpclient[p]->connect(ip, port);
    }

    #endif
#if defined(MODBUSIP_USE_DNS)
    bool connect(String host, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr) {
        return connect(resolver(host.c_str()), port, client_cert, client_private_key, ca_cert, host.c_str());
    }
    bool connect(const char* host, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr) {
        return connect(resolver(host), port, client_cert, client_private_key, ca_cert, host);
    }
#endif
    bool connect(IPAddress ip, uint16_t port, const char* client_cert = nullptr, const char* client_private_key = nullptr, const char* ca_cert = nullptr, const char* host = nullptr) {
        if (!ip)
            return false;
        if(getSlave(ip) >= 0)
		    return false;
        int8_t p = _connect(ip, port, client_cert, client_private_key);
        if (p < 0)
            return false;
        #if defined(ESP8266)
        if (ca_cert) {
            BearSSL::X509List *trustedCA = new BearSSL::X509List(ca_cert);
            tcpclient[p]->setTrustAnchors(trustedCA);
        } else {
            tcpclient[p]->setInsecure();
        }
        #else
        if (ca_cert) {
            tcpclient[p]->setCACert(ca_cert);
        }
        #endif
        //return tcpclient[p]->connect(ip, port);
        if (host) {
          if (!tcpclient[p]->connect(host, port))
            return false;
        }
        else {
          if (!tcpclient[p]->connect(ip, port))
            return false;
        }
        return true;
    }
};
