#include "Arduino.h"

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

TimerMs sekd(1000);  // таймер 1 секунда для вывода на дисплей и периодических функций
TimerMs sekv(1000);  // таймер 1 секунда для постоянно работающих функций
#if (POWPIN == 1)
TimerMs tvin(60000);  // таймер 1 минута для реле 220В
#endif

#if (SENSTEMP1 and SENSTEMP2)
TimerMs fivet(3000);  // таймер 3 секунды для переменного вывода на дисплей температуры транзистора или акб
#endif

// класс управления дисплеем
class Display {
public:
  // Display(void) {}

  // Функции управления подсветкой
#if (TIME_LIGHT)
  void Light_setTime(void) {
    _light_tm = (uint32_t)EEPROM.read(8 + TIMELIGHT) * 60000;  // установить время подсветки
  }

  void Light_high(void) {
    lcd.setBacklight(true);  // включение подсветки
    _light_time = millis();  // старт отсчета времени подсветки
  }

  void Light_low(void) {
    if (EEPROM.read(8 + TIMELIGHT) and (millis() - _light_time > _light_tm)) lcd.setBacklight(false);  // если разрешено отключение подсветки и время вышло отключение подсветки
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

private:
  // список членов для использования внутри класса

#if (TIME_LIGHT)
  uint32_t _light_time;
  uint32_t _light_tm;
#endif
};
extern Display disp;
Display disp = Display();

#if (SENSTEMP2 == 1)
class Temp_akb {
public:
  uint8_t temp;

  void read_temp(void) {
    if (tr_akb.readTemp()) temp = (int8_t)tr_akb.getTempInt();  //  прочитать температуру с датчика. [true если успешно]  // получить значение температуры в int
    tr_akb.requestTemp();                                       // Запросить новое преобразование температуры
  }
};
extern Temp_akb akb;  // создать обьект akb
Temp_akb akb = Temp_akb();
#endif


// работа с АЦП ...
#if (VOLTIN == 1 or SENSTEMP1 == 2 or SENSTEMP2 == 2)

#if (VOLTIN == 1)
#define SENS1 1
#define SPIN1 ADC_A7,
#else
#define SENS1 0
#define SPIN1
#endif
#if (SENSTEMP2 == 2)
#define SENS2 1
#define SPIN2 ADC_A6,
#else
#define SENS2 0
#define SPIN2
#endif
#if (SENSTEMP1 == 2)
#define SENS3 1
#define SPIN3 ADC_A3,
#else
#define SENS3 0
#define SPIN3
#endif


#include <directADC.h>
// класс для работы с АЦП ардуино. Считывает напряжение БП, температуру с датчиков NTC
class ADCSred {
public:
  // ADCSred() {}

#if (SENSTEMP1 == 2)
  int8_t tmp_Q1;
#endif

#if (SENSTEMP2 == 2)
  int8_t tmp_akb;
#endif

  // считывает сырые данные с пинов и суммирует скользящее среднее. Вызывается постоянно.
  void sample(void) {
    if (!adc_busy) {                                     // если ацп свободен
      setAnalogMux((ADC_modes)analog_pins[adc_number]);  // выбрать пин (ADC_A0-ADC_A7)
      ADC_startConvert();                                // ручной старт преобразования
      adc_busy = true;                                   // ацп занят
    }

    if (adc_busy and ADC_available()) {                                                  //  преобразование  готово
      adc_busy = false;                                                                  // ацп свободен
      adc_rez[adc_number] += ((((int32_t)ADC_read() << 4) - adc_rez[adc_number]) >> 4);  // записываем результат в массив // скользящее среднее
      if (++adc_number >= sizeof(analog_pins)) adc_number = 0;                           // выбираем следующий пин или сбрасываем на ноль
    }
  }

#if (VOLTIN == 1)
  uint16_t volt;

  // запуск замеров.
  void start(void) {
    _timer = 0;
    _k = 0;
    _vmax = (uint16_t)EEPROM.read(8 + POWERMAX) * 1000;
  }

  void check(void) {
    // проверить напряжение на превышение раз в 170 мс
    if (++_timer == 255) {
      _timer = 0;

      bitWrite(flag_global, POWERON, (volt > ina.voltsec + 1000));  // если есть питание от БП (если напряжение от БП превышает напряжение на АКБ)

      if (volt > _vmax) {
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
#endif

  // рассчет напряжения БП, температуры с датчиков NTC. Вызывается раз в секунду или реже.
  void compute(void) {
#if (VOLTIN == 1)
    volt = voltin();
#endif
#if (SENSTEMP1 == 2)
    tmp_Q1 = NTC_compute((float)(adc_rez[SENS1 + SENS2] >> 4), ((float)NTCRS1 / NTCR1), NTCB1, TEMPBASE1);
#endif
#if (SENSTEMP2 == 2)
    tmp_akb = NTC_compute((float)(adc_rez[SENS1] >> 4), ((float)NTCRS2 / NTCR2), NTCB2, TEMPBASE2);  //
#endif
  }


private:
  // список членов для использования внутри класса
#if (VOLTIN == 1)
  uint16_t _vmax;  // максимальное напряжение от БП
  uint8_t _timer;  // таймер проверки напряжения
  uint8_t _k;

  // возвращает реальное напряжение
  uint16_t voltin(void) {
    return (uint16_t)((adc_rez[0] >> 4) * (((float)DIV_R1 + DIV_R2) / DIV_R2 / BITRATE) * vkr.Vref);  // рассчитать среднее напряжение за  vr мкс
  }
#endif

  // перечисляем опрашиваемые пины ( ADC_A0-ADC_A7)
  const uint8_t analog_pins[SENS1 + SENS2 + SENS3]{ SPIN1 SPIN2 SPIN3 };  // напряжение БП, температура акб, температура транзистора
  uint8_t adc_number = 0;                                                 // индекс массива с пинами
  int32_t adc_rez[2]{ 0 };                                                // массив с результатами измерения  sizeof(analog_pins)
  bool adc_busy = false;                                                  // флаг - ацп занят работой

#if (SENSTEMP1 == 2 or SENSTEMP2 == 2)
/*  функция для работы с NTC термисторами по закону Стейнхарта-Харта
    Документация. GitHub: https://github.com/GyverLibs/GyverNTC */
  // сигнал АЦП, (R резистора / R термистора), B термистора, t термистора, разрешение АЦП
  int8_t NTC_compute(float analog, float baseDiv, uint16_t B, int8_t tempBase) {
    analog = baseDiv / ((float)(BITRATE - 1) / analog - 1.0f);
    analog = (log(analog) / B) + 1.0f / (tempBase + 273.15f);
    return (int8_t)(1.0f / analog - 273.15f);
  }
#endif
};
extern ADCSred vin;  // замер напряжения БП
ADCSred adc = ADCSred();
#endif
// ...работа с АЦП

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
    tQ1 = adc.tmp_Q1;  // чтение температуры с датчика
#endif
// если подключен датчик температуры и разрешено уменьшать ток при превышении температуры транзистора
#if (SENSTEMP1 and GUARDTEMP)
      // если превышена температура
    if (tQ1 > EEPROM.read(8 + DEGCUR)) bitSet(flag_global, TR_Q1_ERR);                                                 // превышена температура Q1
    else if (bitRead(flag_global, TR_Q1_ERR) and tQ1 < EEPROM.read(8 + DEGCUR) - 5) bitClear(flag_global, TR_Q1_ERR);  // температура Q1 снизилась на 5 гр.
#endif

#if (FAN)
    if (KULDISCHAR == 1 and bitRead(pam.MyFlag, CHARGE) and (pam.Mode == 4 or (pam.Mode == 5 and pam.Number == 3))) {
      bitClear(TCCR1A, COM1A1); // PWM disable пин 9
      gio::high(PWMKUL);  // принудительное включение вентилятора при разряде
    } else {
      int16_t amp = abs(ina.ampersec);  // чтение тока
      switch (EEPROM.read(8 + FANMODE)) {
        case 1:
          // управление кулером ШИМ
          if (tQ1 >= EEPROM.read(8 + DEGMAX)) _kul = 255;
          else _kul = tQ1 ? ((tQ1 >= EEPROM.read(8 + DEGON)) ? (_kul ? map(tQ1, EEPROM.read(8 + DEGOFF), EEPROM.read(8 + DEGMAX), 70, 255) : 150) : _kul) : ((amp >= _curfan[curfanOn]) ? (_kul ? map(amp, _curfan[curfanOff], _curfan[curfanMax], 70, 255) : 150) : _kul);
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
  enum { curfanOn = 0,
         curfanOff,
         curfanMax };
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


// отправка значений в сетевой порт
#if (LOGGER == 2)

class Serial_Out {
public:
  // Serial_Out(){}

  void start(void) {
    _tm = LOGGTIME;
  }

  // Вывод логов в сетевой порт // выполняется ~ 156 мкс
  void Serial_out(int32_t Achas = 0) {
    if (--_tm == 0) {
      _tm = LOGGTIME;
      // uint32_t t = micros();

#pragma pack(push, 1)
      struct
      {
        uint8_t start_send;  //маркер старта
        uint8_t len_send;    //длина посылки
        uint8_t cod_send;    //код данных

        uint16_t volt;     // 1 напряжение на АКБ (милливольт)
        int16_t amper;     // 2 ток АКБ (миллиампер)
        int32_t Achas;     // 5 Полученная_отданная емкость АКБ в Ач
        uint16_t volt_in;  // 4 напряжение от БП (милливольт)
        uint8_t tQ1;       // 3 Температура на Q1 (градусы)
        uint8_t tAkb;      // 6 Температура АКБ (градусы)

        uint8_t crc;  //контролная сумма
      } send_str;
#pragma pack(pop)
      uint8_t *ptr = ((uint8_t *)&send_str);

      //формирование строки для передачи
      send_str.start_send = ':';                 //маркер старта
      send_str.len_send = sizeof(send_str) - 1;  //Длина строки без CRC
      send_str.cod_send = '1';                   //код данных

      send_str.volt = ina.voltsec;    //напряжение на АКБ (милливольт)/1000
      send_str.amper = ina.ampersec;  //ток АКБ (миллиампер)/1000
      send_str.Achas = Achas;         //Полученная_отданная емкость АКБ (А/ч)) /100000

#if (VOLTIN == 1)
      send_str.volt_in = adc.volt;  //напряжение от БП (милливольт)/1000
#else
      send_str.volt_in = 0;
#endif
#if (SENSTEMP1)
      send_str.tQ1 = kul.tQ1;  //Температура на Q1 (градусы)
#else
      send_str.tQ1 = 0;          //Температура на Q1 (градусы)
#endif
#if (SENSTEMP2 == 2)
      send_str.tAkb = adc.tmp_akb;  //6 Температура АКБ (градусы)
#elif (SENSTEMP2 == 1)
      send_str.tAkb = akb.temp;  //6 Температура АКБ (градусы)
#else
      send_str.tAkb = 0;  //6 Температура АКБ (градусы)
#endif
      send_str.crc = 0;
      for (uint8_t i = 0; i < sizeof(send_str) - 1; i++) send_str.crc += ptr[i];  //Расчет контрольной суммы
      Serial.write(ptr, sizeof(send_str));                                        //передать строку

      //  Serial.print(micros() - t);
      //  Serial.write('\r');  // Знак переноса строки.
      //  Serial.write('\n');  // Знак переноса строки.
    }
  }

private:
  uint8_t _tm = LOGGTIME;
};
extern Serial_Out Sout;
Serial_Out Sout = Serial_Out();

#elif (LOGGER == 1)

#include <mString.h>

class Serial_Out {
public:
  // Serial_Out(){}

  void start(void) {
    _tm = LOGGTIME;
  }

  // Вывод логов в сетевой порт // выполняется ~ 344-760 мкс
  // напряжение, ток, А/ч, (напряжение БП), (темп Q1), (темп Акб) - в скобках вывод при наличии датчиков
  void Serial_out(int32_t Achas = 0) {
    if (--_tm == 0) {
      _tm = LOGGTIME;
      // uint32_t t = micros();

      test_add(ina.voltsec);   // 1 напряжение акб - милливольт (/ 1000)
      test_add(ina.ampersec);  // 2 ток акб - миллиампер (/ 1000)
      test_add(Achas);         // амперчасы (/ 100000)
#if (VOLTIN == 1)
      test_add(adc.volt);      // 4 напряжение от БП (милливольт)/1000
#else
      test_add(0);
#endif
#if (SENSTEMP1)
      test_add(kul.tQ1);       // 5 Температура на Q1 (градусы)
#else
      test_add(0);
#endif
#if (SENSTEMP2 == 2)
      test_add(adc.tmp_akb);   // 6 Температура АКБ (градусы)
#elif (SENSTEMP2 == 1)
      test_add(akb.temp);  // 6 Температура АКБ (градусы)
#else
      test_add(0);
#endif
      // можно добавить 17 значений
      Serial.write('\r');  // Знак переноса строки.
      Serial.write('\n');  // Знак переноса строки.

      //  Serial.print(micros() - t);
    }
  }

private:
  uint8_t _tm = LOGGTIME;
  mString<12> test;  // создать текстовый буфер

  void test_add(int32_t x) {
    test.clear();  // очистить буфер
    test.add(x);
    test.add(';');
    Serial.write(test.buf);
  }
};
extern Serial_Out Sout;
Serial_Out Sout = Serial_Out();

#elif (LOGGER == 3)
#include <mString.h>
class Serial_Out {
public:
  // Serial_Out(){}

  void start(void) {
    _tm = LOGGTIME;
  }

  // Вывод логов в сетевой порт // выполняется ~ 344-760 мкс
  // напряжение, ток, А/ч, (напряжение БП), (темп Q1), (темп Акб) - в скобках вывод при наличии датчиков
  void Serial_out(int32_t Achas = 0) {
    if (--_tm == 0) {
      _tm = LOGGTIME;
      // uint32_t t = micros();

      Serial.write('$');       // начало посылки
      test_add(ina.voltsec);   // 1 напряжение акб - милливольт (/ 1000)
      test_add(ina.ampersec);  // 2 ток акб - миллиампер (/ 1000)
      test_add(Achas);         // амперчасы (/ 100000)
#if (VOLTIN == 1)
      test_add(adc.volt);    // 4 напряжение от БП (милливольт)/1000
#else
      test_add(0);
#endif
#if (SENSTEMP1)
      test_add(kul.tQ1);       // 5 Температура на Q1 (градусы)
#else
      test_add(0);
#endif
#if (SENSTEMP2 == 2)
      test_add(adc.tmp_akb);       // 6 Температура АКБ (градусы)
#elif (SENSTEMP2 == 1)
      test_add(akb.temp);    // 6 Температура АКБ (градусы)
#else
      test_add(0);
#endif     
      Serial.write(';');  // конец посылки
      //Serial.write('\r');  // Знак переноса строки.
      // Serial.write('\n');  // Знак переноса строки.

      //  Serial.print(micros() - t);
    }
  }

private:
  uint8_t _tm = LOGGTIME;
  mString<12> test;  // создать текстовый буфер

  void test_add(int32_t x) {
    test.clear();  // очистить буфер
    test.add(x);
    test.add(' ');
    Serial.write(test.buf);
  }
};
extern Serial_Out Sout;
Serial_Out Sout = Serial_Out();
#endif


// библиотека для работы с ЦАП MCP4725
// просто отправляет значение 0-4095 напряжения в MCP4725
#if (MCP4725DAC)
//#include <microWire.h>
class MCP4725 {
public:
  // инициализация
  MCP4725(uint8_t addr)
    : _address(addr) {}

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