/*
    Modbus Library for Arduino
    ModbusTCP general implementation
    Copyright (C) 2014 Andr� Sarmento Barbosa
                  2017-2020 Alexander Emelianov (a.m.emelianov@gmail.com)
*/

#pragma once
#include "Modbus.h"

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1
#ifndef IPADDR_NONE
#define IPADDR_NONE ((uint32_t)0xffffffffUL)
#endif
// Callback function Type
#if defined(MODBUS_USE_STL)
typedef std::function<bool(IPAddress)> cbModbusConnect;
typedef std::function<IPAddress(const char*)> cbModbusResolver;
#else
typedef bool (*cbModbusConnect)(IPAddress ip);
typedef IPAddress (*cbModbusResolver)(const char*);
#endif

struct TTransaction {
	uint16_t	transactionId;
	uint32_t	timestamp;
	cbTransaction cb = nullptr;
	uint8_t*	_frame = nullptr;
	uint8_t*		data = nullptr;
	TAddress	startreg;
	Modbus::ResultCode forcedEvent = Modbus::EX_SUCCESS;	// EX_SUCCESS means no forced event here. Forced EX_SUCCESS is not possible.
	bool operator ==(const TTransaction &obj) const {
		    return transactionId == obj.transactionId;
	}
};

template <class SERVER, class CLIENT>
class ModbusTCPTemplate : public Modbus {
	protected:
	union MBAP_t {
		struct {
			uint16_t transactionId;
			uint16_t protocolId;
			uint16_t length;
			uint8_t	 unitId;
		};
		uint8_t  raw[7];
	};
	cbModbusConnect cbConnect = nullptr;
	cbModbusConnect cbDisconnect = nullptr;
	SERVER* tcpserver = nullptr;
	CLIENT* tcpclient[MODBUSIP_MAX_CLIENTS];
	#if MODBUSIP_MAX_CLIENTS <= 8
	uint8_t tcpServerConnection = 0;
	#elif MODBUSIP_MAX_CLIENTS <= 16
	uint16_t tcpServerConnection = 0;
	#else
	uint32_t tcpServerConnection = 0;
	#endif
	#if defined(MODBUS_USE_STL)
	std::vector<TTransaction> _trans;
	#else
	DArray<TTransaction, 2, 2> _trans;
	#endif
	int16_t		transactionId = 1;  // Last started transaction. Increments on unsuccessful transaction start too.
	int8_t n = -1;
	bool autoConnectMode = false;
	uint16_t serverPort = 0;
	uint16_t defaultPort = MODBUSTCP_PORT;
	cbModbusResolver resolve = nullptr;
	TTransaction* searchTransaction(uint16_t id);
	void cleanupConnections();	// Free clients if not connected
	void cleanupTransactions();	// Remove timedout transactions and forced event

	int8_t getFreeClient();    // Returns free slot position
	int8_t getSlave(IPAddress ip);
	int8_t getMaster(IPAddress ip);
	public:
	uint16_t send(String host, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
	uint16_t send(const char* host, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
	uint16_t send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit = MODBUSIP_UNIT, uint8_t* data = nullptr, bool waitResponse = true);
	// Prepare and send ModbusIP frame. _frame buffer and _len should be filled with Modbus data
	// ip - slave ip address
	// startreg - first local register to save returned data to (miningless for write to slave operations)
	// cb - transaction callback function
	// unit - slave modbus unit id
	// data - if not null use buffer to save returned data instead of local registers
	public:
	ModbusTCPTemplate();
	~ModbusTCPTemplate();
	bool isTransaction(uint16_t id);
#if defined(MODBUSIP_USE_DNS)
	bool isConnected(String host);
	bool isConnected(const char* host);
	bool connect(String host, uint16_t port = 0);
	bool connect(const char* host, uint16_t port = 0);
	bool disconnect(String host);
	bool disconnect(const char* host);
#endif
	bool isConnected(IPAddress ip);
	bool connect(IPAddress ip, uint16_t port = 0);
	bool disconnect(IPAddress ip);
	// ModbusTCP
	void server(uint16_t port = 0);
	// ModbusTCP depricated
	inline void slave(uint16_t port = 0) { server(port); }	// Depricated
	inline void master() { client(); }	// Depricated
	inline void begin() { server(); }; 	// Depricated
	void client();
	void task();
	void onConnect(cbModbusConnect cb = nullptr);
	void onDisconnect(cbModbusConnect cb = nullptr);
	uint32_t eventSource() override;
	void autoConnect(bool enabled = true);
	void dropTransactions();
	uint16_t setTransactionId(uint16_t);
	#if defined(MODBUS_USE_STL)
	static IPAddress defaultResolver(const char*) {return IPADDR_NONE;}
	#else
	static IPAddress defaultResolver(const char*) {return IPADDR_NONE;}
	#endif
};

template <class SERVER, class CLIENT>
ModbusTCPTemplate<SERVER, CLIENT>::ModbusTCPTemplate() {
	//_trans.reserve(MODBUSIP_MAX_TRANSACIONS);
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		tcpclient[i] = nullptr;
	resolve = defaultResolver;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::client() {

}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::server(uint16_t port) {
	if (port)
		serverPort = port;
	else
		serverPort = defaultPort;
	tcpserver = new SERVER(serverPort);
	tcpserver->begin();
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(String host, uint16_t port) {
    return connect(resolve(host.c_str()), port);
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(const char* host, uint16_t port) {
    return connect(resolve(host), port);
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::connect(IPAddress ip, uint16_t port) {
	//cleanupConnections();
	if (!ip)
		return false;
	if(getSlave(ip) != -1)
		return true;
	int8_t p = getFreeClient();
	if (p == -1)
		return false;
	tcpclient[p] = new CLIENT();
	BIT_CLEAR(tcpServerConnection, p);
#if defined(ESP32) && defined(MODBUSIP_CONNECT_TIMEOUT)
	if (!tcpclient[p]->connect(ip, port?port:defaultPort, MODBUSIP_CONNECT_TIMEOUT)) {
#else
	if (!tcpclient[p]->connect(ip, port?port:defaultPort)) {
#endif
		delete(tcpclient[p]);
		tcpclient[p] = nullptr;
		return false;
	}
	return true;
}

template <class SERVER, class CLIENT>
uint32_t ModbusTCPTemplate<SERVER, CLIENT>::eventSource() {		// Returns IP of current processing client query
	if (n >= 0 && n < MODBUSIP_MAX_CLIENTS && tcpclient[n])
	#if !defined(ethernet_h)
		return (uint32_t)tcpclient[n]->remoteIP();
	#else
		return 1;
	#endif
	return (uint32_t)INADDR_NONE;
}

template <class SERVER, class CLIENT>
TTransaction* ModbusTCPTemplate<SERVER, CLIENT>::searchTransaction(uint16_t id) {
#define MODBUSIP_COMPARE_TRANS [id](TTransaction& trans){return trans.transactionId == id;}
	#if defined(MODBUS_USE_STL)
	std::vector<TTransaction>::iterator it = std::find_if(_trans.begin(), _trans.end(), MODBUSIP_COMPARE_TRANS);
   	if (it != _trans.end()) return &*it;
	return nullptr;
	#else
	return _trans.entry(_trans.find(MODBUSIP_COMPARE_TRANS));
	#endif
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::task() {
	MBAP_t _MBAP;
	uint32_t taskStart = millis();
	cleanupConnections();
	if (tcpserver) {
		CLIENT c;
		// WiFiServer.available() == Ethernet.accept() and should wrapped to get code to be compatible with Ethernet library (See ModbusTCP.h code).
		// WiFiServer.available() != Ethernet.available() internally
#if defined(MODBUSIP_USE_AVAILABLE)
		while (millis() - taskStart < MODBUSIP_MAX_READMS && (c = tcpserver->available())) {
#else
		while (millis() - taskStart < MODBUSIP_MAX_READMS && (c = tcpserver->accept())) {
#endif
#if defined(MODBUSIP_DEBUG)
			Serial.println("IP: Accepted");
#endif
			CLIENT* currentClient = new CLIENT(c);
			if (!currentClient || !currentClient->connected()) {
				delete currentClient;
				continue;
			}
#if defined(MODBUSIP_DEBUG)
			Serial.println("IP: Connected");
#endif
			if (cbConnect == nullptr || cbConnect(currentClient->remoteIP())) {
				#if defined(MODBUSIP_UNIQUE_CLIENTS)
				// Disconnect previous connection from same IP if present
				n = getMaster(currentClient->remoteIP());
				if (n != -1) {
					tcpclient[n]->flush();
					delete tcpclient[n];
					tcpclient[n] = nullptr;
				}
				#endif
				n = getFreeClient();
				if (n > -1) {
					tcpclient[n] = currentClient;
					BIT_SET(tcpServerConnection, n);
#if defined(MODBUSIP_DEBUG)
					Serial.print("IP: Conn ");
					Serial.println(n);
#endif
#if defined(MODBUSIP_USE_AVAILABLE)
					break;	// while
#else
					continue; // while
#endif
				}
			}
			// Close connection if callback returns false or MODBUSIP_MAX_CLIENTS reached
			delete currentClient;
		}
	}
	for (n = 0; n < MODBUSIP_MAX_CLIENTS; n++) {
		if (!tcpclient[n]) continue;
		if (!tcpclient[n]->connected()) continue;
		while ((size_t)tcpclient[n]->available() > sizeof(_MBAP) && millis() - taskStart < MODBUSIP_MAX_READMS) {
#if defined(MODBUSIP_DEBUG)
			Serial.print(n);
			Serial.print(": Bytes available ");
			Serial.println(tcpclient[n]->available());
#endif
			tcpclient[n]->readBytes(_MBAP.raw, sizeof(_MBAP.raw));	// Get MBAP
		
			if (__swap_16(_MBAP.protocolId) != 0) {   // Check if MODBUSIP packet. __swap is usless there.
				while (tcpclient[n]->available())	// Drop all incoming if wrong packet
					tcpclient[n]->read();
				continue;
			}
			_len = __swap_16(_MBAP.length);
			if (_len < MODBUSIP_MINFRAME) {	// Length is shorter than MODBUSIP_MINFRAME
				Modbus::FunctionCode fc = FC_READ_COILS; // Just placeholder
				while (tcpclient[n]->available())	// Drop rest of the packet
					tcpclient[n]->read();
				exceptionResponse(fc, EX_ILLEGAL_VALUE);
			}
			_len--; // Do not count with last byte from MBAP
			if (_len > MODBUSIP_MAXFRAME) {	// Length is over MODBUSIP_MAXFRAME
			    Modbus::FunctionCode fc = (Modbus::FunctionCode)tcpclient[n]->read();
				_len--;	// Subtract for read byte
				for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop rest of the packet
					tcpclient[n]->read();
				exceptionResponse(fc, EX_SLAVE_FAILURE);
			}
			else {
				free(_frame);
				_frame = (uint8_t*) malloc(_len);
				if (!_frame) {
			    	Modbus::FunctionCode fc = (Modbus::FunctionCode)tcpclient[n]->read();
					_len--;	// Subtract for read byte
					for (uint8_t i = 0; tcpclient[n]->available() && i < _len; i++)	// Drop rest of the packet
						tcpclient[n]->read();
					exceptionResponse(fc, EX_SLAVE_FAILURE);
				}
				else {
					if (tcpclient[n]->readBytes(_frame, _len) < _len) {	// Try to read MODBUS frame
						exceptionResponse((Modbus::FunctionCode)_frame[0], EX_ILLEGAL_VALUE);
						//while (tcpclient[n]->available())	// Drop all incoming (if any)
						//	tcpclient[n]->read();
					}
					else {
						_reply = EX_PASSTHROUGH;
						// Note on _reply usage
						// it's used and set as ReplyCode by slavePDU and as exceptionCode by masterPDU
						if (_cbRaw) {
							frame_arg_t transData = { _MBAP.unitId, tcpclient[n]->remoteIP(), __swap_16(_MBAP.transactionId), BIT_CHECK(tcpServerConnection, n) };
							_reply = _cbRaw(_frame, _len, &transData);
						}
						if (BIT_CHECK(tcpServerConnection, n)) {
							if (_reply == EX_PASSTHROUGH)
								slavePDU(_frame); // Process incoming frame as slave
							else
								_reply = REPLY_OFF;
						}
						else {
							// Process reply to master request
							TTransaction* trans = searchTransaction(__swap_16(_MBAP.transactionId));
							if (trans) { // if valid transaction id
								if ((_frame[0] & 0x7F) == trans->_frame[0]) { // Check if function code the same as requested
									if (_reply == EX_PASSTHROUGH)
										masterPDU(_frame, trans->_frame, trans->startreg, trans->data);	// Process incoming frame as master
								}
								else {
									_reply = EX_UNEXPECTED_RESPONSE;
								}
								if (trans->cb) {
									trans->cb((ResultCode)_reply, trans->transactionId, nullptr);
								}
								free(trans->_frame);
								#if defined(MODBUS_USE_STL)
								//_trans.erase(std::remove(_trans.begin(), _trans.end(), *trans), _trans.end() );
								std::vector<TTransaction>::iterator it = std::find(_trans.begin(), _trans.end(), *trans);
								if (it != _trans.end())
									_trans.erase(it);
								#else
								size_t r = _trans.find([trans](TTransaction& t){return *trans == t;});
								_trans.remove(r);
								#endif
							}
						}
					}
				}
			}
			if (!BIT_CHECK(tcpServerConnection, n)) _reply = REPLY_OFF;	// No replay if it was responce to master
			if (_reply != REPLY_OFF) {
				_MBAP.length = __swap_16(_len+1);     // _len+1 for last byte from MBAP					
				size_t send_len = (uint16_t)_len + sizeof(_MBAP.raw);
				uint8_t sbuf[send_len];				
				memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
				memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
				tcpclient[n]->write(sbuf, send_len);
				//tcpclient[n]->flush();
			}
			if (_frame) {
				free(_frame);
				_frame = nullptr;
			}
			_len = 0;
		}
	}
	n = -1;
	cleanupTransactions();
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(String host, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
	return send(resolve(host.c_str()), startreg, cb, unit, data, waitResponse);
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(const char* host, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
	return send(resolve(host), startreg, cb, unit, data, waitResponse);
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::send(IPAddress ip, TAddress startreg, cbTransaction cb, uint8_t unit, uint8_t* data, bool waitResponse) {
	MBAP_t _MBAP;
	uint16_t result = 0;
	int8_t p;
#if defined(MODBUSIP_MAX_TRANSACTIONS)
	if (_trans.size() >= MODBUSIP_MAX_TRANSACTIONS)
		goto cleanup;
#endif
	if (!ip)
		return 0;
	if (tcpserver) {
		p = getMaster(ip);
	} else {
		p = getSlave(ip);
	}
	if (p == -1 || !tcpclient[p]->connected()) {
		if (!autoConnectMode)
			goto cleanup;
		if (!connect(ip))
			goto cleanup;
	}
	_MBAP.transactionId	= __swap_16(transactionId);
	_MBAP.protocolId	= __swap_16(0);
	_MBAP.length		= __swap_16(_len+1);     //_len+1 for last byte from MBAP
	_MBAP.unitId		= unit;
	bool writeResult;
	{	// for sbuf isolation
		size_t send_len = _len + sizeof(_MBAP.raw);
		uint8_t sbuf[send_len];
		memcpy(sbuf, _MBAP.raw, sizeof(_MBAP.raw));
		memcpy(sbuf + sizeof(_MBAP.raw), _frame, _len);
		writeResult = (tcpclient[p]->write(sbuf, send_len) == send_len);
	}
	if (!writeResult)
		goto cleanup;
	//tcpclient[p]->flush();
	if (waitResponse) {
		TTransaction tmp;
		tmp.transactionId = transactionId;
		tmp.timestamp = millis();
		tmp.cb = cb;
		tmp.data = data;	// BUG: Should data be saved? It may lead to memory leak or double free.
		tmp._frame = _frame;
		tmp.startreg = startreg;
		_trans.push_back(tmp);
		_frame = nullptr;
	}
	result = transactionId;
	transactionId++;
	if (!transactionId)
		transactionId = 1;
	cleanup:
	free(_frame);
	_frame = nullptr;
	_len = 0;
	return result;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::onConnect(cbModbusConnect cb) {
	cbConnect = cb;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::onDisconnect(cbModbusConnect cb) {
		cbDisconnect = cb;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::cleanupConnections() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		if (tcpclient[i] && !tcpclient[i]->connected()) {
			//IPAddress ip = tcpclient[i]->remoteIP();
			tcpclient[i]->stop();
			delete tcpclient[i];
			tcpclient[i] = nullptr;
			if (cbDisconnect && cbEnabled) 
				cbDisconnect(IPADDR_NONE);
		}
	}
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::cleanupTransactions() {
	#if defined(MODBUS_USE_STL)
	for (auto it = _trans.begin(); it != _trans.end();) {
		if (millis() - it->timestamp > MODBUSIP_TIMEOUT || it->forcedEvent != Modbus::EX_SUCCESS) {
			Modbus::ResultCode res = (it->forcedEvent != Modbus::EX_SUCCESS)?it->forcedEvent:Modbus::EX_TIMEOUT;
			if (it->cb)
				it->cb(res, it->transactionId, nullptr);
			free(it->_frame);
			it = _trans.erase(it);
		} else
			it++;
	}
	#else
	size_t i = 0;
	while (i < _trans.size()) {
		TTransaction t =  _trans[i];
		if (millis() - t.timestamp > MODBUSIP_TIMEOUT || t.forcedEvent != Modbus::EX_SUCCESS) {
			Modbus::ResultCode res = (t.forcedEvent != Modbus::EX_SUCCESS)?t.forcedEvent:Modbus::EX_TIMEOUT;
			if (t.cb)
				t.cb(res, t.transactionId, nullptr);
			free(t._frame);
			_trans.remove(i);
		} else
			i++;
	}
	#endif
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getFreeClient() {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (!tcpclient[i])
			return i;
	return -1;
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getSlave(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && !BIT_CHECK(tcpServerConnection, i))
			return i;
	return -1;
}

template <class SERVER, class CLIENT>
int8_t ModbusTCPTemplate<SERVER, CLIENT>::getMaster(IPAddress ip) {
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++)
		if (tcpclient[i] && tcpclient[i]->connected() && tcpclient[i]->remoteIP() == ip && BIT_CHECK(tcpServerConnection, i))
			return i;
	return -1;
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isTransaction(uint16_t id) {
	return searchTransaction(id) != nullptr;
}
#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(String host) {
	return isConnected(resolve(host.c_str()));
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(const char* host) {
	return isConnected(resolve(host));
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::isConnected(IPAddress ip) {
	if (!ip)
		return false;
	int8_t p = getSlave(ip);
	return  p != -1 && tcpclient[p]->connected();
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::autoConnect(bool enabled) {
	autoConnectMode = enabled;
}

#if defined(MODBUSIP_USE_DNS)
template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(String host) {
	return disconnect(resolve(host.c_str()));
}

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(const char* host) {
	return disconnect(resolve(host));
}
#endif

template <class SERVER, class CLIENT>
bool ModbusTCPTemplate<SERVER, CLIENT>::disconnect(IPAddress ip) {
	if (!ip)
		return false;
	int8_t p = getSlave(ip);
	if (p != -1) {
		tcpclient[p]->stop();
		delete tcpclient[p];
		tcpclient[p] = nullptr;
		return true;
	}
	return false;
}

template <class SERVER, class CLIENT>
void ModbusTCPTemplate<SERVER, CLIENT>::dropTransactions() {
	#if defined(MODBUS_USE_STL)
	for (auto &t : _trans) t.forcedEvent = EX_CANCEL;
	#else
	for (size_t i = 0; i < _trans.size(); i++)
		_trans.entry(i)->forcedEvent = EX_CANCEL;
	#endif
}

template <class SERVER, class CLIENT>
ModbusTCPTemplate<SERVER, CLIENT>::~ModbusTCPTemplate() {
	free(_frame);
	_frame = nullptr;
	dropTransactions();
	cleanupConnections();
	cleanupTransactions();
	delete tcpserver;
	tcpserver = nullptr;
	for (uint8_t i = 0; i < MODBUSIP_MAX_CLIENTS; i++) {
		delete tcpclient[i];
		tcpclient[i] = nullptr;
	}
}

template <class SERVER, class CLIENT>
uint16_t ModbusTCPTemplate<SERVER, CLIENT>::setTransactionId(uint16_t t) {
	transactionId = t;
	if (!transactionId)
		transactionId = 1;
	return transactionId;
}

