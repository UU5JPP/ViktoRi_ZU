#include "Arduino.h"
#include "FunctionLite.h"

#define DCDC_CHARGE true      // заряд
#define DCDC_DISCHARGE false  // разряд
// класс управления DC-DC модулем
class DCDC {
public:
  // список членов, доступных в программе

  DCDC(void){};

  /// задать максимальные значения напряжения и тока заряда
  void begin(uint16_t volt_charge, int16_t cur_charge) {
    _volt_charge = volt_charge;
    _cur_charge_max = cur_charge;
    if (BitIsClear(_flags, Smooth)) _cur_charge = cur_charge;  // если отключен плавный пуск
  }

  void start(bool mods) {
    _cur_max = CURRMAXINT;
    _err_pred = 0;
    _Integral = 0.0;
    _cur_charge = 0;
    _tok = 0;
    _flags = 0b00000011;           // флаги // 0 пауза, 1 плавный пуск, 2 режим (заряд/ разряд)
    bitWrite(_flags, Mode, mods);  // установить режим работы заряд/разряд
    _time_sec = millis();
  }

  // регулировка напряжения и тока заряда
  void Control(void) {
    if (bitRead(pam.MyFlag, CHARGE) and bitRead(_flags, Pause) and bitRead(flag_global, POWERHIGH) and bitRead(flag_global, TEMP_AKB)) {
      if (bitRead(_flags, Mode) and BitIsClear(flag_global, POWERON)) return;  // если включен заряд а напряжения от БП нет
      if (abs(ina.amperms) >= _cur_max and _tok > 0) _tok--;
      else {
        if (millis() - _time_sec > 1000) {
          _time_sec = millis();
          // плавный пуск - плавное увеличение тока заряда/разряда
          if (bitRead(_flags, Smooth)) {
            _cur_charge = constrain(_cur_charge + 300, 0, _cur_charge_max);  // 300 мА/сек
            if (_cur_charge == _cur_charge_max) bitClear(_flags, Smooth);    // отключить плавный пуск
          } else {
#if (SENSTEMP1 and GUARDTEMP)
            // если превышена температура то уменьшаем ток иначе увеличиваем
            _cur_charge = constrain(_cur_charge + (bitRead(flag_global, TR_Q1_ERR) ? -20 : 20), _cur_charge_max >> 1, _cur_charge_max);
#endif
          }
        }

        int16_t volt_err = bitRead(_flags, Mode) ? (int16_t)_volt_charge - (int16_t)ina.voltms : (int16_t)ina.voltms - (int16_t)_volt_charge;  // величина ошибки по напряжению
        int16_t amp_err = _cur_charge - abs(ina.amperms);                                                                                      // величина ошибки по току
        int16_t err = (volt_err < amp_err) ? volt_err : amp_err;                                                                               // регулируем напряжение или ток

        if (bitRead(_flags, Mode)) {
          // заряд
          float P = 0.001f * err;
          _Integral += P;  // 0.001f * err;
          float D = (float)(err - _err_pred) * 5 * 0.0001;
          _err_pred = err;
#if (MCP4725DAC)
          _tok = constrain(trunc(P + _Integral - D), 0, 4095);  // регулировка тока
#else
          _tok = constrain(trunc(P + _Integral - D), 0, 255);  // регулировка тока
#endif
        } else {
          // разряд
          _Integral += err * 0.001f;
          _tok = constrain(trunc(_Integral), 0, 255);
        }
      }
#if (MCP4725DAC)
      bitRead(_flags, Mode) ? dac.setVoltage(_tok) : analogWrite_my(PWMDCH, _tok);
#else
      analogWrite_my((bitRead(_flags, Mode) ? PWMCH : PWMDCH), _tok);
#endif
    }
  }

  void stop(void) {
    #if (MCP4725DAC)
    dac.setVoltage(0);  // отключаем заряд
#else
    bitClear(TCCR2A, COM2B1);  // Pin 3 PWM disable
    gio::low(PWMCH);           // отключаем заряд
#endif
#if (DISCHAR == 1)
    bitClear(TCCR2A, COM2A1);  // 11 PWM disable
    gio::low(PWMDCH);          // отключаем разряд
#endif
  }

  void pause(bool x) {
    bitWrite(_flags, Pause, x);
    if (!x) stop();
  }

  void Off(void) {
    start(DCDC_CHARGE);
    pause(false);
  }

  void setVolt_charge(uint16_t v) {
    _volt_charge = v;
  }

  void setCur_charge(int16_t v) {
    _cur_charge_max = v;
    _cur_charge = v;
  }

  uint16_t getVolt_charge(void) {
    return _volt_charge;
  }

  int16_t getCur_charge(void) {
    return _cur_charge_max;
  }


#if (MCP4725DAC)
  void setTok(uint16_t v) {
    _tok = v;
  }

  uint16_t getTok(void) {
    return _tok;
  }
#else
  void setTok(uint8_t v) {
    _tok = v;
  }

  uint8_t getTok(void) {
    return _tok;
  }
#endif

private:
  // список членов для использования внутри класса
  uint16_t _volt_charge;    // напряжение заряда
  int16_t _cur_max;         // предельный ток зарядника
  int16_t _cur_charge_max;  // максимальный ток заряда
  int16_t _cur_charge;      // текущий ток заряда
#if (MCP4725DAC)
  uint16_t _tok = 0;  // значение ШИМ
#else
  uint8_t _tok = 0;  // значение ШИМ
#endif
  int16_t _err_pred;
  float _Integral;
  uint32_t _time_sec;

  uint8_t _flags = 0b00000000;  // флаги
  enum { Pause = 0, Smooth, Mode };  // пауза, плавный пуск, режим (заряд/ разряд)
};

extern DCDC dcdc;
DCDC dcdc = DCDC();
