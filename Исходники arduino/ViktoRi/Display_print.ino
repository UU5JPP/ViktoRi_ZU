#include "Arduino.h"
// вывод на дисплей во float - напряжение, ток, ампер-часы, ватт-часы, кол. разрядов после запятой.
void PrintVA(uint16_t volt, int16_t current, int32_t Ah, int32_t Wh, uint8_t x) {
  if (volt) {
    lcd.print(((float)volt / 1000), x);  // напряжение
    lcd.print(F("V "));
  }
  if (current) {
    lcd.print(((float)current / 1000), x);  // ток
    lcd.print(F("A "));
  }
  if (Ah) {
    lcd.print(((float)Ah / 100000), x);  // ампер-часы
    lcd.print(F("Ah "));                 // Ah
  }
  if (Wh) {
    lcd.print(((float)Wh / 100000), x);  // ватт-часы
    lcd.print(F("Wh "));                 // Wh
  }
}

void pr_tm(uint16_t tm) {
  if (tm < 10) lcd.print(0);
  lcd.print(tm);
}

// функция расчета текущего времени и вывод на дисплей
void Print_time(uint32_t time_real, uint8_t x, uint8_t y) {
  lcd.setCursor(x, y);
  pr_tm((uint16_t)(time_real / 3600ul));  // часы
  lcd.write(58);
  uint32_t t = (time_real % 3600ul);  // минуты
  pr_tm((uint16_t)(t / 60ul));
  lcd.write(58);
  pr_tm((uint16_t)(t % 60ul));  // секунды
}

void setCursorx(void) {
  lcd.setCursor(0, 0);
}
void setCursory(void) {
  lcd.setCursor(0, 1);
}
void printSimb(void) {
  lcd.write(PROBEL);
}

void print_tr(int8_t tr) {
  lcd.print(tr);
  lcd.print(F(txt_C));
}

bool akb = true;
// функция вывод на дисплей
#if ((DISPLAYx == 16 and DISPLAYy == 2) or (DISPLAYx == 20 and DISPLAYy == 2))
void Display_print(int32_t Ah, uint32_t time_real, uint8_t prc, bool add) {
  if (bitRead(flag_global, POWERHIGH)) {
#if (TIME_LIGHT)
    disp.Light_high();  // включение подсветки
#endif
    lcd.clear();
    lcd.print(F(txt4));  // "Power "
#if (VOLTIN == 1)
    PrintVA(vin.volt(), 0, 0, 0, 1);
#endif
    print_mode(2);  // error
    Speaker(500);   // функция звукового сигнала (время в миллисекундах)
    Delay(3000);    // ждать 3 секунды
  } else {
    // 0 строка
    setCursorx();
    if (ina.voltsec < 10000) lcd.print(0);
    PrintVA(ina.voltsec, 0, Ah, 0, ((Ah < 1000000) ? 2 : 1));  // вывод напряжения и Ач
    lcd.setCursor(DISPLAYx - 1, 0);
    lcd.print((char)pgm_read_byte(&regimr[(add ? 3 : pam.Mode)]));  // вывод режима работы
    // 1 строка
    setCursory();
    PrintVA(0, ((pam.Mode == 4) ? abs(ina.ampersec) : ina.ampersec), 0, 0, ((abs(ina.ampersec) < 10000) ? 3 : 2));
    Print_time(time_real, 7, 1);
    lcd.setCursor(DISPLAYx - 1, 1);
    lcd.write((akb ? map(prc, 0, 100, 0, 6) : ((pam.Mode == 4) ? 32 : 7)));  // знак акб:  пробел - значек заряда
    akb = !akb;
  }
}
// *14.81V 0.01Ah Ch*  0
// *10.55A 01:01:01 *  1

// 1 - используется диспплей 20*4
#elif (DISPLAYx == 20 and DISPLAYy == 4)
// функция вывод на дисплей 20*4
void Display_print(int32_t Ah, int32_t Wh, uint32_t time_real, int8_t tr, uint8_t prc) {
  if (bitRead(flag_global, POWERHIGH)) {
#if (TIME_LIGHT)
    disp.Light_high();  // включение подсветки
#endif
    lcd.clear();
    lcd.print(F(txt4));  // "Power "
#if (VOLTIN == 1)
    PrintVA(vin.volt(), 0, 0, 0, 2);
#else
    print_mode(2);  // error
#endif
    Speaker(500);  // функция звукового сигнала (время в миллисекундах)
    Delay(3000);   // ждать 3 секунды
  } else {
    // 0 строка
    setCursorx();
    print_mode(1);  // вывод типа акб                          // тип Акб
    printSimb();    // пробел
    print_Capacity();
    print_mode(4);  // 12.6V
    // 1 строка
    Print_time(time_real, 0, 1);  // время
    printSimb();                  // пробел
    print_mode(0);                // режим работы
    // 2 строка
    lcd.setCursor(0, 2);
    PrintVA(0, 0, Ah, Wh, 2);  // ампер-часы ватт-часы
    lcd.print(prc);            // процент заряда Акб
    lcd.print(F(txt_PRC));
    // 3 строка
    lcd.setCursor(0, 3);
    if (ina.voltsec < 10000) lcd.print(0);
    PrintVA(ina.voltsec, 0, 0, 0, 2);                                                                               // вывести Вольт
    PrintVA(0, ((pam.Mode == 4) ? abs(ina.ampersec) : ina.ampersec), 0, 0, ((abs(ina.ampersec) < 10000) ? 3 : 2));  // вывести Ампер
#if (SENSTEMP1)
    print_tr(kul.tQ1);                                                                                              // температура
#endif
#if (SENSTEMP2 == 2)
    print_tr(ntc.akb);                                                                                              // температура акб
#endif
    lcd.setCursor(DISPLAYx - 1, 3);
    lcd.write((akb ? map(prc, 0, 100, 0, 6) : ((pam.Mode == 4) ? 32 : 7)));  // знак акб - значек заряда -  пробел
    akb = !akb;
  }
}
// *PB Ca/Ca 60Ah 12.6V *  0
// *01:01:01 Zaryad>Doza*  1
// *60.56Ah 220.56Wh 56%*  2
// *14.81V 6.555A 35C  ~*  3


// 1 - используется диспплей 16*4
#elif (DISPLAYx == 16 and DISPLAYy == 4)
// функция вывод на дисплей 20*4
void Display_print(int32_t Ah, int32_t Wh, uint32_t time_real, int8_t tr, uint8_t prc) {
  if (bitRead(flag_global, POWERHIGH)) {
#if (TIME_LIGHT)
    disp.Light_high();  // включение подсветки
#endif
    lcd.clear();
    lcd.print(F(txt4));  // "Power "
#if (VOLTIN == 1)
    PrintVA(vin.volt(), 0, 0, 0, 2);
#else
    print_mode(2);  // error
#endif
    Speaker(500);  // функция звукового сигнала (время в миллисекундах)
    Delay(3000);   // ждать 3 секунды
  } else {
    // 0 строка
    setCursorx();
    print_Capacity();
    lcd.print(((float)bat.Volt(1) / 100), 1);  // напряжение
    lcd.print(F("V "));                        // V
    print_mode(0);                             // режим работы
    // 1 строка
    Print_time(time_real, 0, 1);  // время
    printSimb();                  // пробел
    lcd.print(prc);               // процент заряда Акб
    lcd.print(F(txt_PRC));
#if (SENSTEMP1)
    print_tr(kul.tQ1);  // температура
#endif
#if (SENSTEMP2 == 2)
    print_tr(ntc.akb);  // температура акб
#endif
    // 2 строка
    lcd.setCursor(0, 2);
    PrintVA(0, 0, Ah, 0, ((Ah < 1000000) ? 2 : 1));  // ампер-часы ватт-часы
    PrintVA(0, 0, 0, Wh, ((Wh < 1000000) ? 2 : 1));  // ампер-часы ватт-часы
    // 3 строка
    lcd.setCursor(0, 3);
    if (ina.voltsec < 10000) lcd.print(0);
    PrintVA(ina.voltsec, 0, 0, 0, 2);                                                                               // вывести Вольт
    PrintVA(0, ((pam.Mode == 4) ? abs(ina.ampersec) : ina.ampersec), 0, 0, ((abs(ina.ampersec) < 10000) ? 3 : 2));  // вывести Ампер
    lcd.setCursor(DISPLAYx - 1, 3);
    lcd.write((akb ? map(prc, 0, 100, 0, 6) : ((pam.Mode == 4) ? 32 : 7)));  // знак акб - значек заряда -  пробел
    akb = !akb;
  }
}
// *60Ah 12.6V Zarya*
// *01:01:01 52% 35C*
// *60.56Ah 220.56Wh*
// *14.81V 6.555A  ~*

#endif

// вывести на дисплей - вкл/откл
void PrintOnOff(bool i) {
  lcd.print((i ? F(txt13) : F(txt14)));  // вкл  / откл
}

// вывод на дисплей > или пробел
void Write_x(bool x) {
  lcd.write((x ? 62 : 32));  // > or  пробел
}

// вывод на дисплей: 0-режим работы, 1-тип акб
void print_mode(uint8_t i) {
  switch (i) {
    case 0: disp.printFromPGM((int)(&modeTxt[pam.Mode])); break;
    case 1: disp.printFromPGM((int)(&typeAkb[pam.typeAkb])); break;
    case 2: lcd.print(F(txt21)); break;                    // "error"
    case 3: disp.printFromPGM((int)(&menuTxt[5])); break;  // "Settings"
    case 4:
      lcd.print(((float)bat.Volt(0) / 1000), 1);  // напряжение
      lcd.write(86);                              // V
      break;
  }
}

void print_Capacity(void) {
  lcd.print(pam.Capacity);
  lcd.print(F("Ah "));  // Ah
}

void printCykl(void) {
  lcd.write(67);  // C
  lcd.print(pam.Round);
}

// Функция вывода напряжения акб и количество банок
void Vout(void) {
  print_mode(4);  // 12.6V
  lcd.write(40);  // (
  lcd.print(pam.Voltage);
  lcd.write(41);  // )
  ClearStr(4);    // очистить поле
  lcd.setCursor(3, 0);
}

// функция очистки строки дисплея (i символов)
void ClearStr(uint8_t i) {
  while (--i) printSimb();  // очистить поле
}

// вывод свободной оперативки
void print_memoryFree(void) {
  lcd.setCursor(0, 0);
  lcd.print(memoryFree());
  lcd.print(F(" "));
}

// Переменные, создаваемые процессом сборки,
// когда компилируется скетч
extern int __bss_end;
extern void *__brkval;

// Функция, возвращающая количество свободного ОЗУ (RAM)
int memoryFree() {
  int freeValue;
  if ((int)__brkval == 0)
    freeValue = ((int)&freeValue) - ((int)&__bss_end);
  else
    freeValue = ((int)&freeValue) - ((int)__brkval);
  return freeValue;
}
