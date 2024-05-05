/*
    Библиотека для работы с NTC термисторами по закону Стейнхарта-Харта
    Документация: 
    GitHub: https://github.com/GyverLibs/GyverNTC

*/
// подключение: GND --- Rt --- ADC --- R --- VCC
//GyverNTC tr_Q1(PINTERM1, NTCR1, NTCB1, 25, NTCRS1, BIT);  // пин, R термистора, B термистора, базовая температура, R резистора - датчик температуры транзистора Q1
//GyverNTC tr_bat; //(PINTERM2, NTCR2, NTCB2, 25, NTCRS2, BIT);  // пин, R термистора, B термистора, базовая температура, R резистора - датчик температуры аккумулятора
#include <Arduino.h>

class NTCv {
public:

  NTCv() {}

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
