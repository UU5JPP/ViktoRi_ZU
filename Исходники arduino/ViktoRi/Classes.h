#include "Arduino.h"

// класс управления дисплеем
class Display {
public:
 // Display(void) {}

    // Функции управления подсветкой
#if (TIME_LIGHT)
  void Light_setTime() {
    _light_tm = (uint32_t)EEPROM.read(8+TIMELIGHT) * 60000;  // установить время подсветки
  }

  void Light_high(void) {
    lcd.setBacklight(true);  // включение подсветки
    _light_time = millis();  // старт отсчета времени подсветки
  }

  void Light_low(void) {
    if (EEPROM.read(8+TIMELIGHT) and (millis() - _light_time > _light_tm)) lcd.setBacklight(false);  // если разрешено отключение подсветки и время вышло отключение подсветки
  }
#endif

  // функция для печати из PROGMEM
  void printFromPGM(int charMap) {
    uint16_t ptr = pgm_read_word(charMap);  // получаем адрес из таблицы ссылок
    while (pgm_read_byte(ptr) != NULL) {
      lcd.write((char)(pgm_read_byte(ptr)));  // выводим в монитор или куда нам надо // всю строку до нулевого символа
      ptr++;                                  // следующий символ
    }
  }

  // загрузка в память дисплея своих символов(состояние акб)
  void mysimbol(void) {
    for (uint8_t x = 0; x < 8; x++) {
      lcd.createChar(x, &simb_array[x][0]);  // загружаем символы в память дисплея
    }
  }


private:
// список членов для использования внутри класса
#if (TIME_LIGHT)
  uint32_t _light_time;
  uint32_t _light_tm;
#endif
};
extern Display disp;
Display disp = Display();


// глобальные таймеры
class TimerMs {
public:

  TimerMs(const uint16_t p)
    : _prd(p) {}

  // запустить/перезапустить таймер
  void start(void) {
    _tmr = millis();
  }

  bool tick(void) {
    if (millis() - _tmr >= _prd) {
      start();
      return true;
    }
    return false;
  }

private:
  uint32_t _tmr = 0;
  const uint16_t _prd;
};

// класс управления кулером
class KULER {
public:  
 // KULER(void) {}

#if (FAN)
  // инициализация
  void begin(void) {
    _kul = 0;  // значение ШИМ кулера
    for (uint8_t i = CURFANON; i <= CURFANMAX; i++) _curfan[i - CURFANON] = (int16_t)EEPROM.read(8 + i) * 100;
  }
#endif

  int8_t tQ1;

  void fan(void) {
    // чтение температуры силового транзистора
    tQ1 = 0;
#if (SENSTEMP1 == 1)
    // датчик температуры DS18B20
    if (tr_Q1.readTemp()) tQ1 = (int8_t)tr_Q1.getTempInt();  //  прочитать температуру с датчика. [true если успешно]  // получить значение температуры в int
    tr_Q1.requestTemp();                                     // Запросить новое преобразование температуры
#elif (SENSTEMP1 == 2)
    // датчик температуры NTC
    tQ1 = ntc.Q1;  // чтение температуры с датчика
#endif
// если подключен датчик температуры и разрешено уменьшать ток при превышении температуры транзистора
#if (SENSTEMP1 and GUARDTEMP)
      // если превышена температура
    if (tQ1 > EEPROM.read(8 + DEGCUR)) bitSet(flag_global, TR_Q1_ERR);                                                 // превышена температура Q1
    else if (bitRead(flag_global, TR_Q1_ERR) and tQ1 < EEPROM.read(8 + DEGCUR) - 5) bitClear(flag_global, TR_Q1_ERR);  // температура Q1 снизилась на 5 гр.
#endif

#if (FAN)
    if (KULDISCHAR == 2 and bitRead(pam.MyFlag, CHARGE) and pam.Mode == 4) {
      gio::high(PWMKUL);  // принудительное включение вентилятора при разряде
    } else {
      int16_t amp = abs(ina.ampersec);  // чтение тока
      switch (EEPROM.read(8 + FANMODE)) {
        case 1:
          // управление кулером ШИМ
          if (tQ1 >= EEPROM.read(8 + DEGMAX)) _kul = 255;
          else _kul = tQ1 ? ((tQ1 >= EEPROM.read(8 + DEGON)) ? (_kul ? map(tQ1, EEPROM.read(8 + DEGOFF), EEPROM.read(8 + DEGMAX), 70, 255) : 150) : _kul) : ((amp >= _curfan[curfanOn]) ? (_kul ? map(amp,_curfan[curfanOff], _curfan[curfanMax], 70, 255) : 150) : _kul);
          // если кулер включен
          if (_kul) {
            _kul = tQ1 ? ((tQ1 <= EEPROM.read(8 + DEGOFF)) ? 0 : _kul) : ((amp <= _curfan[curfanOff]) ? 0 : _kul);
          }
          analogWrite_my(PWMKUL, _kul);
          break;

        case 2:
          // управление кулером вкл/откл
          // если кулер включен
          if (_kul) {
            _kul = tQ1 ? (tQ1 <= EEPROM.read(8 + DEGOFF) ? false : _kul) : ((amp <= _curfan[curfanOff]) ? false : _kul);
          } else {
            // если кулер отключен
            _kul = tQ1 ? (tQ1 >= EEPROM.read(8 + DEGON) ? true : _kul) : ((amp >= _curfan[curfanOn]) ? true : _kul);
          }
          gio::write(PWMKUL, _kul);
          break;
      }
    }
#endif
  }

private:
  // список членов для использования внутри класса
#if (FAN)
  uint8_t _kul;  // значение ШИМ кулера
  int16_t _curfan[3];
  enum {curfanOn = 0, curfanOff, curfanMax};
#endif
};
extern KULER kul;
KULER kul = KULER();

  //int16_t _curfanOn;   // значение тока в Амперах при котором включается вентилятор. (до 25,5А) define в 2_menu.h
  //int16_t _curfanOff;  // значение тока в Амперах при котором отключается вентилятор. (до 25,5А)
  //int16_t _curfanMax;  // значение тока в Амперах  для максимальных оборотов вентилятора ШИМ. (до 25,5А)

// настройка команд энкодера и кнопок
#define LEFT -1
#define RIGHT 1
#define ENCCLICK 16
#define ENCHELD 2
#define OKCLICK 26
#define OKHELD 12
#define STOPCLICK 36
#define STOPHELD 22
#define PLUSHELD 32
#define MINUSHELD 42
class Enbutt {
public:
 // Enbutt(void){};

  int8_t tick;
  // функция опрашивает энкодер и кнопки.
  void Tick(void) {
    tick = 0;
// -1 - left, 1 - right, 16 - click, 2 - hold, 4 - step
#if (ENCBUTT == 0)
    // 0 - только энкодер
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
#elif (ENCBUTT == 1)
    // 1 - только кнопки
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
    if (Plus.tick()) {
      if (Plus.click() or Plus.step()) tick = RIGHT;
    }
    if (Minus.tick()) {
      if (Minus.click() or Minus.step()) tick = LEFT;
    }
#elif (ENCBUTT == 2)
    // 2 - энкодер + OK + Stop
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
#elif (ENCBUTT == 3)
    // 3 - энкодер + все кнопки
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
    if (Plus.tick()) {
      if (Plus.click() or Plus.step()) tick = RIGHT;
    }
    if (Minus.tick()) {
      if (Minus.click() or Minus.step()) tick = LEFT;
    }
#endif
#if (TIME_LIGHT)
    if (tick) disp.Light_high();
#endif
  }
  //private:
};
extern Enbutt butt;
Enbutt butt = Enbutt();


/*
    класс для работы с NTC термисторами по закону Стейнхарта-Харта
    Документация: 
    GitHub: https://github.com/GyverLibs/GyverNTC
*/
class NTCv {
public:
 // NTCv() {}

#if (SENSTEMP1 == 2)
  int8_t Q1;
#endif

#if (SENSTEMP2 == 2)
  int8_t akb;
#endif

  // чтение сырых данных с аналогового пина с усреднением. Вызывается часто.
  void readTemp(void) {
#if (SENSTEMP1 == 2)
    _Q1 += ((((int32_t)analogRead_my(PINTERM1) << 4) - _Q1) >> 4);  // скользящее среднее каждые _prd мкс
#endif

#if (SENSTEMP2 == 2)
    _akb += ((((int32_t)analogRead_my(PINTERM2) << 4) - _akb) >> 4);  // скользящее среднее каждые _prd мкс
#endif
  }

// рассчет усредненной температуры с датчиков NTC. Вызывается раз в секунду или реже.
  void compute(void) {
#if (SENSTEMP1 == 2)
    Q1 = NTC_compute((float)(_Q1 >> 4), ((float)NTCRS1 / NTCR1), NTCB1, TEMPBASE1);
#endif
#if (SENSTEMP2 == 2)
    akb = NTC_compute((float)(_akb >> 4), ((float)NTCRS2 / NTCR2), NTCB2, TEMPBASE2);
#endif
  }

private:
#if (SENSTEMP1 == 2)
  int32_t _Q1;
#endif

#if (SENSTEMP2 == 2)
  int32_t _akb;
#endif

  // сигнал АЦП, (R резистора / R термистора), B термистора, t термистора, разрешение АЦП
  int8_t NTC_compute(float analog, float baseDiv, uint16_t B, int8_t tempBase) {
    analog = baseDiv / ((float)(BITRATE - 1) / analog - 1.0f);
    analog = (log(analog) / B) + 1.0f / (tempBase + 273.15f);
    return (int8_t)(1.0f / analog - 273.15f);
  }
};
#if (SENSTEMP1 == 2 or SENSTEMP2 == 2)
extern NTCv ntc;// создать обьект ntc
NTCv ntc = NTCv();
#endif

#if (VOLTIN == 1)
// данный класс нужен для считывания напряжения от БП и вычисления среднего значения
class ADCSred {
public:
  // ADCSred() {}

  // запуск замеров.
  void start(void) {
    _timer = 0;
    _k = 0;    
    _vmax = (uint16_t)EEPROM.read(8 + POWERMAX) * 1000;
    _vinmcrs = (int32_t)analogRead_my(POWIN) << 4;
  }

  // считывает напряжение и суммирует скользящее среднее. Вызывается совместно с замерами INA226 (ina.sample())
  void sample(void) {
    _vinmcrs += ((((int32_t)analogRead_my(POWIN) << 4) - _vinmcrs) >> 4);  // скользящее среднее каждые _prd мкс
    // проверить напряжение на превышение раз в 170 мс
    if (++_timer == 255) {
      _timer = 0;
      uint16_t vin = volt(); // напряжение БП

      bitWrite(flag_global, POWERON, (vin > ina.voltsec + 1000));  // если есть питание от БП (если напряжение от БП превышает напряжение на АКБ)

      if (vin > _vmax) {
        // если напряжение превысит максимальное проводим 10 проверок
        if (++_k > 10) {
          _k = 0;
          bitClear(flag_global, POWERHIGH);
        }
      } else {
        _k = 0;
        bitSet(flag_global, POWERHIGH);
      }


#if (POWPIN == 2)
      gio::write(RELAY220, bitRead(flag_global, POWERON));  // индикатор включения БП в сеть
#endif
    }
  }

  // возвращает реальное напряжение
  uint16_t volt(void) {
    return (uint16_t)((_vinmcrs >> 4) * (((float)DIV_R1 + DIV_R2) / DIV_R2 / BITRATE) * vkr.Vref);  // рассчитать среднее напряжение за  vr мкс
  }

private:
  // список членов для использования внутри класса
  uint16_t _vmax;    // максимальное напряжение от БП
  int32_t _vinmcrs;  // среднее напряжение
  uint8_t _timer;    // таймер проверки напряжения
  uint8_t _k;
};
extern ADCSred vin;  // замер напряжения БП
ADCSred vin = ADCSred();
#endif

// отправка значений в сетевой порт
#if (LOGGER)

class Serial_Out {
public:
 // Serial_Out(){}

  void start(void) {
    _tm = LOGGTIME;
  }

  // Вывод логов в сетевой порт // выполняется 1964 мкс (на lgt8 32МГц - 840 мкс)
  // напряжение, ток, А/ч, (напряжение БП), (темп Q1), (темп Акб) - в скобках вывод при наличии датчиков
  void Serial_out(float Achas) {
    if (--_tm == 0) {
      _tm = LOGGTIME;
      Serial_print((float)ina.voltsec / 1000);   //1 напряжение на АКБ (вольт)
      Serial_print((float)ina.ampersec / 1000);  //2 ток АКБ (ампер)
      Serial_print(Achas / 100000);              //3 Полученная_отданная емкость АКБ (А/ч))
#if (VOLTIN == 1)
      Serial_print((float)vin.volt() / 1000);  //4 напряжение от БП
#endif
#if (SENSTEMP1)
      Serial_print(kul.tQ1);  //5 Температура на Q1 (градусы)
#endif

#if (SENSTEMP2 == 2)
      Serial_print(ntc.akb);  //6 Температура АКБ (градусы)
#endif
      // Serial_print( ); // добавь что хочешь
      // Serial_print( ); // добавь что хочешь
      // Serial_print( ); // добавь что хочешь
      // Serial_print( ); // добавь что хочешь

      //Serial_print(7);      //7  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере. нужна коррекция скетча WiFi переходника
      //Serial_print(8);      //8  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(9);      //9  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(10);    //10  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(11);    //11  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(12);    //12  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(13);    //13  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(14);    //14  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(15);    //15  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(16);    //16  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      //Serial.print(17);    //17  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
      Serial.print("\n");  // Знак переноса строки.00
    }
  }

private:
  uint8_t _tm = LOGGTIME;

  void Serial_print(float x) {
    Serial.print(x, 2);
    Serial.print(F(";"));
  }
};
extern Serial_Out Sout;
Serial_Out Sout = Serial_Out();
#endif


// библиотека для работы с ЦАП MCP4725
// просто отправляет значение 0-4095 напряжения в MCP4725
#if (MCP4725DAC)
#include <microWire.h>
class MCP4725 {
public:
  // инициализация 
  MCP4725(uint8_t addr) : _address(addr) {}

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
extern MCP4725 dac;
MCP4725 dac = MCP4725(ADDR4725);



#endif