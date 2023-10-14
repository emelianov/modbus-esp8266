/*
  ModbusRTU ESP8266/ESP32
  ModbusTCP to ModbusRTU bridge with on-device ModbusRTU simulator
*/
#ifdef ESP8266
 #include <ESP8266WiFi.h>
#else //ESP32
 #include <WiFi.h>
#endif
#include <ModbusTCP.h>
#include <ModbusRTU.h>
//#include <SoftwareSerial.h>
//SoftwareSerial S(13, 15);
#include <StreamBuf.h>
#define BSIZE 1024
uint8_t buf1[BSIZE];
uint8_t buf2[BSIZE];
StreamBuf S1(buf1, BSIZE);
StreamBuf S2(buf2, BSIZE);
DuplexBuf P1(&S1, &S2);
DuplexBuf P2(&S2, &S1);
ModbusRTU sym;

int DE_RE = 2;

ModbusRTU rtu;
ModbusTCP tcp;

IPAddress srcIp;


uint16_t transRunning = 0;  // Currently executed ModbusTCP transaction
uint8_t slaveRunning = 0;   // Current request slave
 
bool cbTcpTrans(Modbus::ResultCode event, uint16_t transactionId, void* data) { // Modbus Transaction callback
  if (event != Modbus::EX_SUCCESS)                  // If transaction got an error
    Serial.printf("Modbus result: %02X, Mem: %d\n", event, ESP.getFreeHeap());  // Display Modbus error code (222527)
  if (event == Modbus::EX_TIMEOUT) {    // If Transaction timeout took place
    tcp.disconnect(tcp.eventSource());          // Close connection
    transRunning = 0;
    slaveRunning = 0;
  }
  return true;
}

bool cbRtuTrans(Modbus::ResultCode event, uint16_t transactionId, void* data) {
    if (event != Modbus::EX_SUCCESS)                  // If transaction got an error
      Serial.printf("Modbus result: %02X, Mem: %d\n", event, ESP.getFreeHeap());  // Display Modbus error code (222527)
    return true;
}


// Callback receives raw data 
Modbus::ResultCode cbTcpRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*) custom;
  
  Serial.print("TCP IP in - ");
  Serial.print(IPAddress(src->ipaddr));
  Serial.printf(" Fn: %02X, len: %d \n\r", data[0], len);

  if (transRunning) { // Note that we can't process new requests from TCP-side while waiting for response from RTU-side.
    tcp.setTransactionId(src->transactionId); // Set transaction id as per incoming request
    tcp.errorResponse(IPAddress((src->ipaddr), (Modbus::FunctionCode)data[0], Modbus::EX_SLAVE_DEVICE_BUSY);
    return Modbus::EX_SLAVE_DEVICE_BUSY;
  }

  rtu.rawRequest(src->unitId, data, len, cbRtuTrans);
  
  if (!src->unitId) { // If broadcast request (no response from slave is expected)
    tcp.setTransactionId(src->transactionId); // Set transaction id as per incoming request
    tcp.errorResponse(IPAddress(src->ipaddr), (Modbus::FunctionCode)data[0], Modbus::EX_ACKNOWLEDGE);

    transRunning = 0;
    slaveRunning = 0;
    return Modbus::EX_ACKNOWLEDGE;
  }
  
  srcIp = IPAddress(src->ipaddr);
  
  slaveRunning = src->unitId;
  
  transRunning = src->transactionId;
  
  return Modbus::EX_SUCCESS;  
  
}


// Callback receives raw data from ModbusTCP and sends it on behalf of slave (slaveRunning) to master
Modbus::ResultCode cbRtuRaw(uint8_t* data, uint8_t len, void* custom) {
  auto src = (Modbus::frame_arg_t*) custom;
  if (!transRunning) // Unexpected incoming data
      return Modbus::EX_PASSTHROUGH;
  tcp.setTransactionId(transRunning); // Set transaction id as per incoming request
  uint16_t succeed = tcp.rawResponse(srcIp, data, len, slaveRunning);
  if (!succeed){
    Serial.print("TCP IP out - failed");
  }
  Serial.printf("RTU Slave: %d, Fn: %02X, len: %d, ", src->slaveId, data[0], len);
  Serial.print("Response TCP IP: ");
  Serial.println(srcIp);
  
  transRunning = 0;
  slaveRunning = 0;
  return Modbus::EX_PASSTHROUGH;
}


void setup() {
  Serial.begin(115000);
  WiFi.begin("E2", "*****");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
    
  tcp.server(); // Initialize ModbusTCP to pracess as server
  tcp.onRaw(cbTcpRaw); // Assign raw data processing callback
  
  //S.begin(19200, SWSERIAL_8E1);
  //rtu.begin(&S, DE_RE);  // Specify RE_DE control pin
  sym.begin((Stream*)&P2);
  sym.slave(1);
  sym.addHreg(1, 100);
  rtu.begin((Stream*)&P1);  // Specify RE_DE control pin
  rtu.master(); // Initialize ModbusRTU as master
  rtu.onRaw(cbRtuRaw); // Assign raw data processing callback
}

void loop() { 
  sym.task();
  rtu.task();
  tcp.task();
  yield();
}