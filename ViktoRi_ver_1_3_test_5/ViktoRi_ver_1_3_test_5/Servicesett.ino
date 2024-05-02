#if (SERVICE == 1)
// изменение сервисных параметров
void serviceparam(void) {
  lcd.clear();
  uint8_t arrowPos = 0;   // номер пункта меню
  bool controlState = 0;  // клик
  int8_t screenPos = 0;   // номер "экрана"
  int8_t lastScreen = 0;  // предыдущий номер "экрана"
  bool ext = true;
  do {
    screenPos = arrowPos / DISPLAYy;           // ищем номер экрана (0..3 - 0, 4..7 - 1)
    if (lastScreen != screenPos) lcd.clear();  // если экран сменился - очищаем
    lastScreen = screenPos;
    // для всех строк
    for (byte i = 0; i < DISPLAYy; i++) {
      lcd.setCursor(0, i);  // курсор в начало
      // если курсор находится на выбранной строке
      lcd.write((arrowPos == DISPLAYy * screenPos + i) ? (controlState ? 62 : 126) : 32);  // рисуем стрелку или пробел      
      if (DISPLAYy * screenPos + i == SETTINGS_AMOUNT) break; // если пункты меню закончились, покидаем цикл for
      // выводим имя и значение пункта меню
      disp.printFromPGM((int)(&servicc[DISPLAYy * screenPos + i]));
      lcd.print(F(": "));
      lcd.print(vkr.service[DISPLAYy * screenPos + i]);
      printSimb();  // пробелы для очистки
    }
    pauses();  // ожидание действий пользователя

    switch (butt.tick) {
      case RIGHT:  // поворот вправо
      case LEFT:   // поворот влево
        if (!controlState) {
          arrowPos = constrain(arrowPos + butt.tick, 0, SETTINGS_AMOUNT - 1);  // двигаем курсор // ограничиваем
        } else {
          switch (arrowPos) {
            case FANMODE:                                                        // выбор режима работы кулера
              vkr.service[arrowPos] = constrain(vkr.service[arrowPos] + butt.tick, 0, 2);  // меняем параметры
              break;
            case FREQCHARGE:                                                       // изменение частоты заряда
            case FREQDISCHAR:                                                      // изменение частоты разряда
              vkr.service[arrowPos] = constrain(vkr.service[arrowPos] + butt.tick, 0, 7);  // меняем параметры частоты
              break;
            default:
              vkr.service[arrowPos] = constrain(vkr.service[arrowPos] + butt.tick, 0, 255);  // меняем параметры
              break;
          }
        }
        break;
      case ENCCLICK:
      case OKCLICK:
        controlState = !controlState;  // двигать курсор / изменять данныые
        break;
      case STOPCLICK:
      case ENCHELD:
        ext = false;  // выйти из цикла
        break;
    }
  } while (ext);
  lcd.clear();
  print_mode(3);       // "Settings"
  lcd.print(F(txt3));  // saved
  lcd.write(63);       // ?
  if (Choice(30, false)) {
    Saved();      // сохранить настройки
    resetFunc();  // перезагрузить Ардуино
  }
}
#endif

// установка частоты ШИМ на пинах 3 и 11
void Freq(const uint8_t frq) {
  switch (frq) {
    case 0:
      // Пины D3 и D11 - 245 Гц
      TCCR2B = 0b00000101;  // x128
      TCCR2A = 0b00000001;  // phase correct
      break;
    case 1:
      // Пины D3 и D11 - 490 Гц
      TCCR2B = 0b00000100;  // x64
      TCCR2A = 0b00000001;  // phase correct
      break;
    case 2:
      // Пины D3 и D11 - 980 Гц
      TCCR2B = 0b00000011;  // x32
      TCCR2A = 0b00000001;  // phase correct
      break;
    case 3:
      // Пины D3 и D11 - 2 кГц
      TCCR2B = 0b00000011;  // x32
      TCCR2A = 0b00000011;  // fast pwm
      break;
    case 4:
      // Пины D3 и D11 - 4 кГц
      TCCR2B = 0b00000010;  // x8
      TCCR2A = 0b00000001;  // phase correct
      break;
    case 5:
      // Пины D3 и D11 - 8 кГц
      TCCR2B = 0b00000010;  // x8
      TCCR2A = 0b00000011;  // fast pwm
      break;
    case 6:
      // Пины D3 и D11 - 31.4 кГц
      TCCR2B = 0b00000001;  // x1
      TCCR2A = 0b00000001;  // phase correct
      break;
    case 7:
      // Пины D3 и D11 - 62.5 кГц
      TCCR2B = 0b00000001;  // x1
      TCCR2A = 0b00000011;  // fast pwm
      break;
  }
}


/*
ШИМ на выводах 9 и 10
9 бит, 31 250 Гц
TCCR1A = TCCR1A & 0xe0 | 2;
TCCR1B = TCCR1B & 0xe0 | 0x09;

*/