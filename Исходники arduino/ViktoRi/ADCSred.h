#include "Arduino.h"

#if (VOLTIN == 1)

// данный класс нужен для считывания напряжения от БП и вычисления среднего значения
class ADCSred {
public:
  // список членов, доступных в программе

  // инициализация
  ADCSred() {}

  // запуск замеров.
  void start(void) {
    _timer = 0;
    _k = 0;
    _vmax = (uint16_t)(((uint16_t)EEPROM.read(8 + POWERMAX) * 1000) / ((((float)DIV_R1 + DIV_R2) / DIV_R2 / BITRATE) * vkr.Vref));  // максимальное напряжение от БП
    _vinmcrs = (int32_t)analogRead_my(POWIN) << 4;
  }

  // считывает напряжение и суммирует скользящее среднее. Вызывается совместно с замерами INA226 (ina.sample())
  void sample(void) {
    _vinmcrs += ((((int32_t)analogRead_my(POWIN) << 4) - _vinmcrs) >> 4);  // скользящее среднее каждые _prd мкс
    // проверить напряжение на превышение раз в 170 мс
    if (++_timer == 255) {
      _timer = 0;

      bitWrite(flag_global, POWERON, (volt() > ina.voltsec + 1000));  // если есть питание от БП (если напряжение от БП превышает напряжение на АКБ)
      if (bitRead(flag_global, POWERON)) {
        if ((_vinmcrs >> 4) > _vmax) {
          // если напряжение превысит максимальное проводим 10 проверок
          if (++_k > 10) {
            _k = 0;
            bitClear(flag_global, POWERHIGH);
          }
        } else {
          _k = 0;
          bitSet(flag_global, POWERHIGH);
        }
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

#endif