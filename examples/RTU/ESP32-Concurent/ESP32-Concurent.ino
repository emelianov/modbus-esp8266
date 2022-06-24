/*
  ModbusRTU ESP32
  Concurent thread example
  
  (c)2020 Alexander Emelianov (a.m.emelianov@gmail.com)
  https://github.com/emelianov/modbus-esp8266

  Tool Modbus Slave on PC for test
  https://www.modbustools.com/download.html
*/

#include <ModbusRTU.h>

#define REG 0
#define REG_NUM 32
#define SLAVE_ID1 51
#define SLAVE_ID2 52

#define MBUS_HW_SERIAL Serial1
#define MBUS_TXD_PIN   16         
#define MBUS_RXD_PIN   35

ModbusRTU mb;

xSemaphoreHandle xMutex;
Modbus::ResultCode err;

Modbus::ResultCode readSync(uint8_t address, uint16_t start, uint16_t num, uint16_t* buf) {
  xSemaphoreTake(xMutex, portMAX_DELAY);
  if (mb.slave()) {
    xSemaphoreGive(xMutex);
    return Modbus::EX_GENERAL_FAILURE;
  }
  Serial.printf("SlaveID: %d Hreg %d\r\n", address, start);
  mb.readIreg(address, start, buf, num, [](Modbus::ResultCode event, uint16_t, void*) {
    err = event;
    return true;
  });
  while (mb.slave()) {
    vTaskDelay(1);
    mb.task();
  }
  Modbus::ResultCode res = err;
  xSemaphoreGive(xMutex);
  return res;
}

void loop1( void * pvParameters );
void loop2( void * pvParameters );

void setup() {
  Serial.begin(115200);
  MBUS_HW_SERIAL.begin(9600, SERIAL_8N1, MBUS_RXD_PIN, MBUS_TXD_PIN);
  mb.begin(&MBUS_HW_SERIAL);
  mb.master();
  xMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
                    loop1,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    10,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 1 */
 
 xTaskCreatePinnedToCore(
                    loop2,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    NULL,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */

}

uint16_t hregs1[REG_NUM];
void loop1( void * pvParameters ){
  while(true) {
      delay(10);
      if (readSync(SLAVE_ID1, REG, REG_NUM, hregs1) == Modbus::EX_SUCCESS)
        Serial.println("OK 1");
      else
        Serial.println("Error 1");
  }
}

uint16_t hregs2[REG_NUM];
void loop2( void * pvParameters ){
  while(true) {
      delay(100);
      if (readSync(SLAVE_ID2, REG, REG_NUM, hregs2) == Modbus::EX_SUCCESS)
        Serial.println("OK 2");
      else
        Serial.println("Error 2");
  }
}

void loop() {
  delay(100);
}
