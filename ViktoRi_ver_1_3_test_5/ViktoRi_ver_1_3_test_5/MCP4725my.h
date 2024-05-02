// библиотека для работы с ЦАП MCP4725
// просто отправляет значение 0-4095 напряжения в MCP4725
#include "Arduino.h"
#include <microWire.h>

class MCP4725 {
public:
  // список членов, доступных в программе

  // инициализация (номер аналогового пина)
  MCP4725(uint8_t addr) : _address(addr) {
    //Wire.begin();
  }

 // Запись 16-ти битного регистра MCP4725
  void setVoltage(uint16_t data) {
    Wire.beginTransmission(_address);         // Начинаем передачу
    Wire.write(0b01000000);                   // передаем контрольный байт (010-Sets in Write mode)
    Wire.write((uint8_t)(data >> 4));         // Upper data bits (D11.D10.D9.D8.D7.D6.D5.D4)Отправляем старший байт
    Wire.write((uint8_t)((data % 12) << 4));  // Lower data bits (D3.D2.D1.D0.x.x.x.x));         // Отправляем младший байт
    Wire.endTransmission();                   // Заканчиваем передачу
  }

  // Проверка присутствия
  bool testConnection(void) {
    Wire.beginTransmission(_address);      // Начинаем передачу
    return (bool)!Wire.endTransmission();  // Сразу заканчиваем
  }

private:  
  const uint8_t _address;
};
