#include "Arduino.h"
#include "FunctionLite.h"

// класс управления DC-DC модулем
class DCDC {
public:
  // список членов, доступных в программе

  DCDC(void){};

  /// задать максимальные значения напряжения и тока заряда
  void begin(uint16_t volt_charge, int16_t cur_charge) {
    _volt_charge = volt_charge;
    _cur_charge_max = cur_charge;
    if (BitIsClear(flag_global, DCDC_SMOOTH)) _cur_charge = cur_charge;
  }

  void start(void) {
    _cur_max = CURRMAXINT;
    _err_pred = 0;
    _Integral = 0.0;
    bitSet(flag_global, DCDC_SMOOTH);
    bitSet(flag_global, DCDC_PAUSE);
    _cur_charge = 0;
    _tok = 0;
    _time_sec = millis();
  }

#if (MCP4725DAC)
  // регулировка напряжения и тока
  void Control() {
    if (bitRead(pam.MyFlag, CHARGE) and bitRead(flag_global, DCDC_PAUSE) and bitRead(flag_global, POWERON) and BitIsClear(flag_global, POWERHIGH)) {
      if (ina.isAlert() or abs(ina.amperms) >= _cur_max) _tok = constrain(_tok - 10, 0, 4095);
      else {
        // плавный пуск - плавное увеличение тока заряда
        if (millis() - _time_sec > 1000) {
          _time_sec = millis();
          if (bitRead(flag_global, DCDC_SMOOTH)) {
            _cur_charge = constrain(_cur_charge + 300, 0, _cur_charge_max);  // 300 мА/сек
            if (_cur_charge == _cur_charge_max) bitClear(flag_global, DCDC_SMOOTH);
          }
        }
        int16_t volt_err = (int16_t)_volt_charge - (int16_t)ina.voltms;
        int16_t amp_err = _cur_charge - ina.amperms;  // величина ошибки по току
        int16_t err = (volt_err < amp_err) ? volt_err : amp_err;

        float P = 0.001f * err;
        _Integral += P;                                   // 0.001f * err;
        float D = (float)(err - _err_pred) * 5 * 0.0001;  //  / 0.2 сек
        _err_pred = err;

        _tok = constrain(trunc(P + _Integral - D), 0, 4095);  // регулировка тока   0.012
      }
      dac.setVoltage(_tok);
    }
  }
#else

  // регулировка напряжения и тока заряда
  void Control(void) {
    if (bitRead(pam.MyFlag, CHARGE) and bitRead(flag_global, DCDC_PAUSE) and bitRead(flag_global, POWERON) and BitIsClear(flag_global, POWERHIGH)) {
      if (abs(ina.amperms) >= _cur_max) _tok = constrain(_tok - 1, 0, 255);
      else {
        if (millis() - _time_sec > 1000) {
          _time_sec = millis();
          // плавный пуск - плавное увеличение тока заряда
          if (bitRead(flag_global, DCDC_SMOOTH)) {
            _cur_charge = constrain(_cur_charge + 300, 0, _cur_charge_max);  // 300 мА/сек
            if (_cur_charge == _cur_charge_max) bitClear(flag_global, DCDC_SMOOTH);
          } else {
#if (SENSTEMP1 and GUARDTEMP)
            // если превышена температура то уменьшаем ток иначе увеличиваем
            _cur_charge = constrain(_cur_charge + (bitRead(flag_global, TR_Q1_ERR) ? -20 : 20), _cur_charge_max >> 1, _cur_charge_max);
#endif
          }
        }
        int16_t volt_err = (int16_t)_volt_charge - (int16_t)ina.voltms;  // величина ошибки по напряжению
        int16_t amp_err = _cur_charge - ina.amperms;                     // величина ошибки по току
        int16_t err = (volt_err < amp_err) ? volt_err : amp_err;         // регулируем напряжение или ток

        float P = 0.001f * err;
        _Integral += P;  // 0.001f * err;
        float D = (float)(err - _err_pred) * 5 * 0.0001;
        _err_pred = err;

        _tok = constrain(trunc(P + _Integral - D), 0, 255);  // регулировка тока
      }
#if (VOLTIN == 1)
      analogWrite_my(PWMCH, _tok);
#endif
    }
#if (VOLTIN == 0)
    // проверяет если есть питание от БП
    if (ina.amperms < 0 and _tok > 20) {
      bitClear(flag_global, POWERON);
      _tok = 30;
    } else bitSet(flag_global, POWERON);
#if (POWPIN == 2)
    gio::write(RELAY220, bitRead(flag_global, POWERON));
#endif
    analogWrite_my(PWMCH, _tok);
#endif
  }

  void control_alert(void) {
    if (gio::read(4)) _tok = constrain(_tok - 1, 0, 255);
    else _tok = constrain(_tok + 1, 0, 255);
    analogWrite_my(PWMCH, _tok);
  }

#endif

// регулировка напряжения и тока разряда
#if (DISCHAR == 1)
  void Control_dich(void) {
    // разряд
    if (bitRead(pam.MyFlag, CHARGE) and bitRead(flag_global, DCDC_PAUSE) and BitIsClear(flag_global, POWERHIGH)) {
      int16_t volt_err = (int16_t)ina.voltms - (int16_t)pam.Volt_discharge;  // величина ошибки по напряжению
      int16_t amp_err = pam.Current_discharge - abs(ina.amperms);            // величина ошибки по току
      int16_t err = (volt_err < amp_err) ? volt_err : amp_err;               // регулируем напряжение или ток
      _Integral += err * 0.001f;
      analogWrite_my(PWMDCH, (constrain(trunc(_Integral), 0, 255)));
    }
  }
#endif

  void Off(void) {
    start();
    bitClear(flag_global, DCDC_PAUSE);
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
};

extern DCDC dcdc;
DCDC dcdc = DCDC();
