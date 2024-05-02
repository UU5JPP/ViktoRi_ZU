#include "Arduino.h"
#include <microWire.h>

/*
Конструктор:	
    INA226 ina226 (Адрес на шине I2c)    
    INA226 ina226 (0x41);				// Шунт и макс. ток по умолчанию, адрес 0x41 - подойдет для нескольких модулей   
    
Методы:
    bool begin();							    // Инициализация модуля и проверка присутствия, вернет false если INA226 не найдена
    void sleep(true / false);				    // Включение и выключение режима низкого энергопотребления, в зависимости от аргумента
    void setAveraging(avg);					    // Установка количества усреднений измерений (см. таблицу ниже)
    void setSampleTime(ch, time);			    // Установка времени выборки напряжения и тока (INA226_VBUS / INA226_VSHUNT), по умолчанию INA226_CONV_1100US
        
    float getShuntVoltage(); 				  // Прочитать напряжение на шунте
    float getVoltage();	 					    // Прочитать напряжение
    float getCurrent();  					    // Прочитать ток
    float getPower(); 						    // Прочитать мощность
    
    uint16_t getCalibration();	 			    // Прочитать калибровочное значение (после старта рассчитывается автоматически)
    void setCalibration(calibration value);	    // Записать калибровочное значение 	(можно хранить его в EEPROM)	 		
    void adjCalibration(calibration offset);    // Подкрутить калибровочное значение на указанное значение (можно менять на ходу)
    
    Версия 1.5 от 06.01.2023
*/

/* Public-определения (константы) */
#define INA226_VBUS true     // Канал АЦП, измеряющий напряжение шины (0-36в)
#define INA226_VSHUNT false  // Канал АЦП, измеряющий напряжение на шунте

// Время выборки (накопления сигнала для оцифровки)
#define INA226_CONV_140US 0b000
#define INA226_CONV_204US 0b001
#define INA226_CONV_332US 0b010
#define INA226_CONV_588US 0b011
#define INA226_CONV_1100US 0b100
#define INA226_CONV_2116US 0b101
#define INA226_CONV_4156US 0b110
#define INA226_CONV_8244US 0b111

// Встроенное усреднение (пропорционально увеличивает время оцифровки)
#define INA226_AVG_X1 0b000
#define INA226_AVG_X4 0b001
#define INA226_AVG_X16 0b010
#define INA226_AVG_X64 0b011
#define INA226_AVG_X128 0b100
#define INA226_AVG_X256 0b101
#define INA226_AVG_X512 0b110
#define INA226_AVG_X1024 0b111

/* Private-определения (адреса) */
#define INA226_CFG_REG_ADDR 0x00
#define INA226_SHUNT_REG_ADDR 0x01
#define INA226_VBUS_REG_ADDR 0x02
#define INA226_POWER_REG_ADDR 0x03
#define INA226_CUR_REG_ADDR 0x04
#define INA226_CAL_REG_ADDR 0x05

#define INA226_REG_MASKENABLE 0x06
#define INA226_REG_ALERTLIMIT 0x07

#define INA226_BIT_AFF 0x0010  // (бит 4)  // Чтение регистра срабатывания Alert

#define INA226_BIT_SOL          (1 << 15) //0x8000  // (бит 15)Shunt Voltage Over-Voltage. Установка в 1 этого бита конфигурирует установку ножки Alert, если измеренное напряжение шунта превысило значение, запрограммированное в Alert Limit Register.
//#define INA226_BIT_SUL        0x4000  // (бит 14)Shunt Voltage Under-Voltage. Установка в 1 этого бита конфигурирует установку ножки Alert, если измеренное напряжение шунта упало ниже значения, запрограммированного в Alert Limit Register.
//#define INA226_BIT_BOL        0x2000  // (бит 13) Bus Voltage Over-Voltage. Установка в 1 этого бита конфигурирует установку ножки Alert, если измеренное напряжение VBUS превысило значение, запрограммированное в Alert Limit Register.
//#define INA226_BIT_BUL        0x1000  // (бит 12)Bus Voltage Under-Voltage. Установка в 1 этого бита конфигурирует установку ножки Alert, если измеренное напряжение VBUS упало ниже значения, запрограммированного в Alert Limit Register.
//#define INA226_BIT_POL        0x0800  // (бит 11)Power Over-Limit. Установка в 1 этого бита конфигурирует установку ножки Alert, если вычисленная мощность после измерения напряжения VBUS превысило значение, запрограммированное в Alert Limit Register.
//#define INA226_BIT_CNVR       0x0400  // (бит 10)Conversion Ready. Установка в 1 этого бита конфигурирует установку ножки Alert, когда установится Conversion Ready Flag (бит 3), показывая тем самым готовность к следующему преобразованию.
//#define INA226_BIT_AFF        0x0010  // (бит 4)Alert Function Flag. Хотя в любой момент времени ножкой Alert может отслеживаться только одна функция Alert, ножкой Alert также может отслеживаться и флаг Conversion Ready. Чтение Alert Function Flag после события alert дает возможность пользователю определить, была ли источником этого события функция Alert.
//#define INA226_BIT_CVRF       0x0008  // (бит 3)Conversion Ready Flag, чтобы помочь координировать однократные преобразования (или triggered-преобразования). Бит Conversion Ready Flag установится после завершения всех преобразований, усреднений и умножений.
//#define INA226_BIT_OVF        0x0004  // (бит 2)Math Overflow Flag. Этот бит установится в 1, если арифметическая операция привела к ошибке переполнения. Это показывает, что данные тока и мощности могут быть недостоверными.
//#define INA226_BIT_APOL       0x0002  // (бит 1)Alert Polarity bit. Этот бит устанавливает полярность активации ножки Alert. 1 = инвертированная полярность (открытый коллектор с активной лог. 1). 0 = нормальная полярность (открытый коллектор с активным лог. 0, настройка по умолчанию).
//#define INA226_BIT_LEN        0x0001  // (бит 0)Alert Latch Enable - Конфигурирует разрешение защелкивания функции ножки Alert и бит Alert Flag. 1 = защелкивание разрешено (режим Latch). 0 = прозрачный режим (режим Transparent, настройка по умолчанию).



class INA226 {
public:

  INA226(void){}

  // Инициализация и проверка
  bool begin() {
    Wire.begin();                                          // Инициализация шины I2c
    if (!testConnection()) return false;                   // Проверка присутствия

    writeRegister(INA226_REG_MASKENABLE, INA226_BIT_SOL);  // сигнал Alert сработает при превышении напряжения на шунте 0,081 Вольт. При шунте 0,01 Ом ток срабатывания 0,081/0,01=8,1А
    writeRegister(INA226_REG_ALERTLIMIT, 32400);           // 0,081/0,0000025f   ограничение по напряжению на шунте 32400 * 0.0025 = 81мВ
    return true;                                           // Вернуть true если все ок
  }

  // Установка / снятие режима сна
  void sleep(bool state) {
    uint16_t cfg_register = readRegister(INA226_CFG_REG_ADDR) & ~(0b111);        // Прочитать конф. регистр и стереть биты режима
    writeRegister(INA226_CFG_REG_ADDR, cfg_register | (state ? 0b000 : 0b111));  // Записать новое значение конф. регистра с выбранным режимом
  }

  // Установка калибровочного значения
  void setCalibration(uint16_t cal) {
    writeRegister(INA226_CAL_REG_ADDR, cal);  // Пишем значение в регистр калибровки
    _cal_value = cal;                         // Обновляем внутреннюю переменную
  }

  // Подстройка калибровочного значения
  void adjCalibration(int16_t adj) {
    setCalibration(getCalibration() + adj);  // Читаем и модифицируем значение
    _cal_value = _cal_value + adj;           // Обновляем внутреннюю переменную
  }

  // Чтение калибровочного значения
  uint16_t getCalibration(void) {
    _cal_value = readRegister(INA226_CAL_REG_ADDR);  // Обновляем внутреннюю переменну
    return _cal_value;                               // Возвращаем значение
  }

  // Установка встроенного усреднения выборок
  void setAveraging(uint8_t avg) {
    uint16_t cfg_register = readRegister(INA226_CFG_REG_ADDR) & ~(0b111 << 9);  // Читаем конф. регистр, сбросив биты AVG2-0
    writeRegister(INA226_CFG_REG_ADDR, cfg_register | avg << 9);                // Пишем новое значение конф. регистр
  }

  // Установка разрешения для выбранного канала
  void setSampleTime(bool ch, uint8_t mode) {
    uint16_t cfg_register = readRegister(INA226_CFG_REG_ADDR);  // Читаем конф. регистр
    cfg_register &= ~((0b111) << (ch ? 6 : 3));                 // Сбрасываем нужную пачку бит, в зависимости от канала
    cfg_register |= mode << (ch ? 6 : 3);                       // Пишем нужную пачку бит, в зависимости от канала
    writeRegister(INA226_CFG_REG_ADDR, cfg_register);           // Пишем новое значение конф. регистра
  }

  // Чтение напряжения на шунте
  int8_t getShuntVoltage(void) {
   // setCalibration(_cal_value);                           // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    int16_t value = readRegister(INA226_SHUNT_REG_ADDR);  // Чтение регистра напряжения шунта //    0x7FFF (максимальное значение int16_t)
    return (int8_t)(value * 0.0025f);                     // LSB = 2.5uV = 0.0000025V, умножаем и возвращаем //  return value * 0.0000025f;
  }

  // Чтение напряжения на шунте из регистра
  int16_t getShuntHex(void) {
   // setCalibration(_cal_value);                           // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    int16_t value = readRegister(INA226_SHUNT_REG_ADDR);  // Чтение регистра напряжения шунта //  0x7FFF (максимальное значение int16_t) +- 32767
    return value;
  }

  // Чтение напряжения
  uint16_t getVoltage(void) {
    uint16_t value = readRegister(INA226_VBUS_REG_ADDR);  // Чтение регистра напряжения
    return (uint16_t)trunc(value * _vkoof);                    // LSB = 1.25mV = 0.00125V, Сдвигаем значение до 12 бит и умножаем  // return value * 0.00125f;
  }

  // Чтение напряжения  из регистра
  uint16_t getVoltageHex(void) {
    uint16_t value = readRegister(INA226_VBUS_REG_ADDR);  // Чтение регистра напряжения 0x7FFF (максимальное значение uint16_t) + 32767 Диапазон полной шкалы 40.96V
    return value;                                         // преобразовать в напряжение * 1.25f    // преобразоват напр. в рег. ИНА * 0.8f
  }

  // Чтение тока
  int16_t getCurrent(void) {
    //setCalibration(_cal_value);                         // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    int16_t value = readRegister(INA226_CUR_REG_ADDR);  // Чтение регистра тока
    return (int16_t)trunc(value * _current_lsb);             // LSB рассчитывается на основе макс. ожидаемого тока, умножаем и возвращаем  return value * _current_lsb;
  }

  // Чтение тока
  int16_t getCurrentHex(void) {
    setCalibration(_cal_value);                         // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    int16_t value = readRegister(INA226_CUR_REG_ADDR);  // Чтение регистра тока
    return value;
  }

  // Чтение мощности
  int32_t getPower(void) {
    //setCalibration(_cal_value);                           // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    int16_t value = readRegister(INA226_POWER_REG_ADDR);  // Чтение регистра мощности
    return (int32_t)trunc(value * _power_lsb);                 // LSB в 25 раз больше LSB для тока, умножаем возвращаем  // return value * _power_lsb;
  }

  // Чтение регистра срабатывания Alert
  bool isAlert(void) {
    return ((readRegister(INA226_REG_MASKENABLE) & INA226_BIT_AFF) == INA226_BIT_AFF);
  }

  //********* вычисление среднего ***********
  uint16_t voltms = 0;
  int16_t amperms = 0;
  uint16_t voltsec = 0;
  int16_t ampersec = 0;

/*
время выборки (накопления сигнала для оцифровки)
INA226_CONV_140US // 140 мкс
INA226_CONV_204US // 204 мкс
INA226_CONV_332US // 332 мкс
INA226_CONV_588US // 588 мкс
INA226_CONV_1100US // 1100 мкс
INA226_CONV_2116US // 2116 мкс
INA226_CONV_4156US // 4156 мкс
INA226_CONV_8244US // 8244 мкс
*/

  void start(uint8_t i) {
    _r_shunt = (float)vkr.shunt / 100000;                  // сопротивление шунта
    _i_max = 0.08192f / _r_shunt;                          // максимальный ток в зависимости от сопротивления шунта
    _vkoof = INAVKOOFV;                                    // коэффициент напряжения INA226
    _ohms = (float)vkr.Ohms / 1000;                        // сопротивление цепи от INA до аккумулятора Ом
    calibrate();                                           // расчёт калибровочного значения и инициализация
    setCalibration(_cal_value);                            // Принудительное обновление калибровки (на случай внезапного ребута INA226)
    _vs = 0;
    _as = 0;
    switch (i) {
      case 1:
        // параметры для заряда
        setSampleTime(INA226_VBUS, INA226_CONV_332US);    // время выборки напряжения // 140 * 16 * 2 = 4480 // 2116 * 2 = 4232 //332 * 16 * 2 = 10624 мкс
        setSampleTime(INA226_VSHUNT, INA226_CONV_332US);  // время выборки тока
       setAveraging(INA226_AVG_X1);                    // 4х кратное усреднение, по умолчанию усреднения нет(1, 4, 16, 64, 128, 256, 512, 1024).
        _prd = 670;                                      // 780 / 846 Гц
        break;
      case 2:
        // параметры для разряда
        setSampleTime(INA226_VBUS, INA226_CONV_588US);    // время выборки напряжения // 140 * 16 * 2 = 4480 // 2116 * 2 = 4232 //332 * 16 * 2 = 10624 мкс
        setSampleTime(INA226_VSHUNT, INA226_CONV_588US);  // время выборки тока
        setAveraging(INA226_AVG_X4);                      // 4х кратное усреднение, по умолчанию усреднения нет(1, 4, 16, 64, 128, 256, 512, 1024).
        _prd = 4704;                                      // 212 Гц
        break;
    }
    _tmp = micros();
  }

  bool sample(void) {
    bool fg = false;
    if (micros() - _tmp >= _prd) {
      _tmp = micros();
      fg = true;
      amperms = getCurrent();                              // чтение тока -32767mA - +32767mA max
      voltms = getVoltage() - (int16_t)(amperms * _ohms);  // vkr.Ohms / 1000 // чтение напряжения 0 - 65535mV max с корректировкой

      _vs += ((((int32_t)voltms << 4) - _vs) >> 4);  // скользящее среднее - сгладить в 16 раз
      voltsec = (uint16_t)(_vs >> 4);
      _as += ((((int32_t)amperms << 4) - _as) >> 4);  // скользящее среднее - сгладить в 16 раз
      ampersec = (int16_t)(_as >> 4);
    }
    return fg;
  }

  // функция INA226 error
  void error(void) {
    lcd.clear();
    lcd.print(F(txt6));   // INA226 error     
    delay(3000);
  }


private:
  uint8_t _iic_address = 0x40;  // Адрес на шине I2c
  float _r_shunt;               // Сопротивление шунта  = 0.1f
  float _i_max;                 // Макс. ожидаемый ток  = 0.8f

  float _current_lsb = 0.0;  // LSB для тока
  float _power_lsb = 0.0;    // LSB для мощности
  uint16_t _cal_value = 0;   // Калибровочное значение

  // Запись 16-ти битного регистра INA226
  void writeRegister(uint8_t address, uint16_t data) {
    Wire.beginTransmission(_iic_address);  // Начинаем передачу
    Wire.write(address);                   // Отправляем адрес
    Wire.write(highByte(data));            // Отправляем старший байт
    Wire.write(lowByte(data));             // Отправляем младший байт
    Wire.endTransmission();                // Заканчиваем передачу
  }

  // Чтение 16-ти битного регистра INA226
  uint16_t readRegister(uint8_t address) {
    Wire.beginTransmission(_iic_address);        // Начинаем передачу
    Wire.write(address);                         // Отправляем адрес
    Wire.endTransmission();                      // Заканчиваем передачу
    Wire.requestFrom(_iic_address, (uint8_t)2);  // Запрашиваем 2 байта
    return Wire.read() << 8 | Wire.read();       // Клеим и возвращаем результат
  }

  // Проверка присутствия
  bool testConnection(void) {
    Wire.beginTransmission(_iic_address);  // Начинаем передачу
    return (bool)!Wire.endTransmission();  // Сразу заканчиваем, инвертируем результат
  }

  // Процедура расчёта калибровочного значения и инициализации
  void calibrate(void) {
    writeRegister(INA226_CFG_REG_ADDR, 0x8000);                     // Принудительный сброс
    _current_lsb = _i_max / 32.768f;                                // расчёт LSB для тока (см. доку INA226)
    _power_lsb = _current_lsb * 25.0f;                              // расчёт LSB для мощности (см. доку INA226)
    _cal_value = trunc(0.00512f / (_i_max / 32768.0f * _r_shunt));  // расчёт калибровочного значения (см. доку INA226)
    setCalibration(_cal_value);                                     // Записываем стандартное калибровочное значение
  }

  //********* вычисление среднего ***********
  uint16_t _prd;  // период замеров в мkс
  uint32_t _tmp;  // время
  float _ohms;
  int32_t _vs = 0;
  int32_t _as = 0;
  float _vkoof = 1.25f;
};
