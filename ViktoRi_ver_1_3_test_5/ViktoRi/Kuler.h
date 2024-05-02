#include "Arduino.h"

// класс управления кулером
class KULER {
public:
  // список членов, доступных в программе
  KULER(void) {}

#if (FAN)
  // инициализация
  void begin(void) {
    _kul = 0;                                            // значение ШИМ кулера
    _kulb = false;                                       // кулер вкл/откл
    _curfanOn = (int16_t)vkr.service[CURFANON] * 100;    // значение тока в Амперах при котором включается вентилятор. (до 25,5А) define в 2_menu.h
    _curfanOff = (int16_t)vkr.service[CURFANOFF] * 100;  // значение тока в Амперах при котором отключается вентилятор. (до 25,5А)
    _curfanMax = (int16_t)vkr.service[CURFANMAX] * 100;  // значение тока в Амперах  для максимальных оборотов вентилятора ШИМ. (до 25,5А)
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
    tQ1 = tr_Q1.getTempInt();  // чтение температуры с датчика
#endif
// если подключен датчик температуры и разрешено уменьшать ток при превышении температуры транзистора
#if (SENSTEMP1 and GUARDTEMP)
      // если превышена температура
    if (tQ1 > vkr.service[DEGCUR]) bitSet(flag_global, TR_Q1_ERR);                                                 // превышена температура Q1
    else if (bitRead(flag_global, TR_Q1_ERR) and tQ1 < vkr.service[DEGCUR] - 5) bitClear(flag_global, TR_Q1_ERR);  // температура Q1 снизилась на 5 гр.
#endif

#if (FAN)
    if (KULDISCHAR == 2 and bitRead(pam.MyFlag, CHARGE) and pam.Mode == 4) {      
      gio::high(PWMKUL);  // принудительное включение вентилятора при разряде
    } else {
      int16_t amp = abs(ina.ampersec);  // чтение тока
      switch (vkr.service[FANMODE]) {
        case 1:
          // управление кулером ШИМ
          if (tQ1 >= vkr.service[DEGMAX]) _kul = 255;
          else _kul = tQ1 ? ((tQ1 >= vkr.service[DEGON]) ? (_kul ? map(tQ1, vkr.service[DEGOFF], vkr.service[DEGMAX], 70, 255) : 150) : _kul) : ((amp >= _curfanOn) ? (_kul ? map(amp, _curfanOff, _curfanMax, 70, 255) : 150) : _kul);
          // если кулер включен
          if (_kul) {
            _kul = tQ1 ? ((tQ1 <= vkr.service[DEGOFF]) ? 0 : _kul) : ((amp <= _curfanOff) ? 0 : _kul);
          }
          analogWrite_my(PWMKUL, _kul);
          break;

        case 2:
          // управление кулером вкл/откл
          // если кулер включен
          if (_kulb) {
            _kulb = tQ1 ? ((tQ1 <= vkr.service[DEGOFF]) ? false : _kulb) : ((amp <= _curfanOff) ? false : _kulb);
          } else {
            // если кулер отключен
            _kulb = tQ1 ? ((tQ1 >= vkr.service[DEGON]) ? true : _kulb) : ((amp >= _curfanOn) ? true : _kulb);
          }          
          gio::write(PWMKUL, _kulb);
          break;
      }
    }
#endif
  }

private:
  // список членов для использования внутри класса
#if (FAN)
  uint8_t _kul;        // значение ШИМ кулера
  bool _kulb;          // кулер вкл/откл
  int16_t _curfanOn;   // значение тока в Амперах при котором включается вентилятор. (до 25,5А) define в 2_menu.h
  int16_t _curfanOff;  // значение тока в Амперах при котором отключается вентилятор. (до 25,5А)
  int16_t _curfanMax;  // значение тока в Амперах  для максимальных оборотов вентилятора ШИМ. (до 25,5А)
#endif
};
extern KULER kul;
KULER kul = KULER();
