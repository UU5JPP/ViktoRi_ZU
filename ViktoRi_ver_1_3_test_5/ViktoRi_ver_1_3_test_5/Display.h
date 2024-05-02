#include "Arduino.h"

// класс управления дисплеем
class Display {
public:
  // список членов, доступных в программе
  Display(void) {}

    // Функции управления подсветкой
#if (TIME_LIGHT)
  void Light_setTime() {
    _light_tm = (uint32_t)vkr.service[TIMELIGHT] * 60000;  // установить время подсветки
  }
  #endif

  void Light_high(void) {
    lcd.setBacklight(true);  // включение подсветки
    #if (TIME_LIGHT)
    _light_time = millis();  // старт отсчета времени подсветки
    #endif
  }
#if (TIME_LIGHT)
  void Light_low(void) {
    if (vkr.service[TIMELIGHT] and (millis() - _light_time > _light_tm)) lcd.setBacklight(false);  // если разрешено отключение подсветки и время вышло отключение подсветки
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