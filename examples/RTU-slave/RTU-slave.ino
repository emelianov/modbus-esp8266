/*
  ModbusRTU ESP8266/ESP32
  Simple slave example

  (c)2019 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  modified 13 May 2020
  by brainelectronics

  This code is licensed under the BSD New License. See LICENSE.txt for more info.
*/

#include <ModbusRTU.h>

#define REGN 10
#define SLAVE_ID 1

ModbusRTU mb;

void setup() {
  Serial.begin(9600, SERIAL_8N1);
#if defined(ESP32) || defined(ESP8266)
  mb.begin(&Serial);
#else
  mb.setBaudrate(9600);
  mb.begin(&Serial);
#endif
  mb.slave(SLAVE_ID);
  mb.addHreg(REGN);
  mb.Hreg(REGN, 100);
}

void loop() {
  mb.task();
  yield();
}