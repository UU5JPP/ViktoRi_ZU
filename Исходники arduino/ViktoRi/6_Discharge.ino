#if (DISCHAR == 1)
//Функция разряда аккумулятора
void Discharge() {
  //вывод режима, напряжения и тока заряда/разряда
  PrintVA(pam.Volt_discharge, pam.Current_discharge, 0, 0, 1);  // напряжения и тока разряда
  printCykl();                                                  // вывод количества циклов
  Freq(EEPROM.read(8 + FREQDISCHAR));                           // установить частоту работы разрядного модуля (4 кГц по умолчанию)
  const uint16_t s1 = ina.voltsec;                              // текущее наряжение акб;
  uint8_t tok = 1, n_disp = 0, ns_disp = 0;
  bool dischar = true;  // разряд включен

  uint8_t time_millis = 0;
  Tyme tyme;
  tyme.local = pam.Time_discharge;
  dcdc.start(DCDC_DISCHARGE);
  dcdc.begin(pam.Volt_discharge, pam.Current_discharge);  // задает напряжение и ток разряда

#if (LOGGER)
  Sout.start();
#endif
  ina.start(2);  // старт замеров INA
  Delay(3000);

#if (POWPIN == 1)
  bitSet(flag_global, RELEY_OFF);  // разрешено отключать реле
#endif
  uint16_t time10 = 600;  // выполнить раз в 600 сек (10 мин)
  sekd.start();           // старт отсчета времени одна секунда
  tyme.real = millis();  // запоминаем текущее время
  // цикл разряда 35 часов максимум
  while (dischar and bitRead(pam.MyFlag, CHARGE) and pam.Time_discharge < 126000) {
    sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.

    if (butt.tick) {
      // если кнопка нажата
      switch (butt.tick) {
        case ENCHELD:                    // удержать энкодер
        case STOPHELD:                   // удержать Стоп
          bitClear(pam.MyFlag, CHARGE);  // завершить разряд
          break;
        case RIGHT:             // энкодер вправо
        case LEFT:              // энкодер влево
          n_disp++;             // окно отображения
          ns_disp = TIME_DISP;  // время отображения сек
          lcd.clear();
          break;
      }
    }

    if (sekd.tick()) time_millis = 5;

    // если прошла секунда
    if (time_millis) {
      switch (time_millis) {
        case 5:
          {
            int16_t ampsec = abs(ina.ampersec);
            pam.Ah_discharge += aw_ch((int32_t)ampsec);      // расчет ампер/часов
            pam.Wh_discharge += aw_ch(abs(ina.getPower()));  // расчет ватт/часов
            tyme.cal_real_tm();                              // рассчитать количество всех секунд разряда
            pam.Time_discharge = tyme.real_tm;

            if (ina.voltsec <= pam.Volt_discharge and ampsec <= pam.Current_discharge_min) dischar = false;  // напряжение уменьшилось до установленного и ток уменьшился до установленного то завершить разряд
          }
          break;
          //       case 4: break;
        case 3:
          // вывод на дисплей 1602
#if (DISPLAYy == 2)
          switch (n_disp) {
              // вывод на дисплей
            case 0:
              // экран 0 // вывод текущих значений напряжения, тока. Вывод Ватт-часов.
              Display_print(pam.Ah_discharge, pam.Time_discharge, map(ina.voltsec, pam.Volt_discharge, s1, 0, 100), false);  // вывод на дисплей
              break;
            case 1:
              // экран 1 // вывод средних значений напряжения, тока. Вывод Ватт-часов.
              setCursorx();
              print_mode(0);
              printSimb();
              printCykl();
              setCursory();
              PrintVA(0, 0, 0, pam.Wh_discharge, 2);
              lcd.print(map(ina.voltsec, pam.Volt_discharge, s1, 0, 100));
              lcd.print(F(txt_PRC));
#if (SENSTEMP1)
              print_tr(kul.tQ1);  // температура
#endif
#if (SENSTEMP2 == 2)
              print_tr(ntc.akb);  // температура акб
#endif
                // print_memoryFree(); // вывод свободной оперативки
              if (ns_disp) ns_disp--;
              else n_disp = 0;
              break;
              // *Discharge C1 N1 *
              // *45.25Wh 45%     *
            default:
              n_disp = 0;
              break;
          }
// вывод на дисплей 2004 или 1604
#elif (DISPLAYy == 4)
          // вывод на дисплей
          switch (n_disp) {
            case 0:
              // экран 0  вывод текущих значений
              Display_print(pam.Ah_discharge, pam.Wh_discharge, pam.Time_discharge, kul.tQ1, map(ina.voltsec, pam.Volt_discharge, s1, 0, 100));
              break;
            case 1:
              // экран 1
              setCursorx();
              print_mode(0);
              printSimb();
              printCykl();
              setCursory();
#if (SENSTEMP1)
              print_tr(kul.tQ1);  // температура
#endif
#if (SENSTEMP2 == 2)
              print_tr(ntc.akb);  // температура акб
#endif
              if (ns_disp) ns_disp--;
              else n_disp = 0;
              break;
            default:
              n_disp = 0;
              break;
          }
#endif
          break;

        case 2:
#if (TIME_LIGHT)
          disp.Light_low();  // отключение подсветки через 3 минуты, если разрешено отключение
#endif
          break;
        case 1:
#if (GUARDA0)
          Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
#if (LOGGER)
          Sout.Serial_out(pam.Ah_discharge);
#endif

          time10--;
          // выполнить раз в 10 мин
          if (!time10) {
            time10 = 600;
            Saved();  // сохранить настройки
          }
          break;
      }
      time_millis--;
    }
    // выполнить раз в секунду
  }
  // цикл разряда
  dcdc.Off();  // отключить заряд, разряд.
#if (GUARDA0)
  gio::low(PINA0);
#endif
  Saved();  // сохранить настройки
}
// Функция разряда аккумулятора
#endif

#if (RESIST == 1)
// Функция измерения сопротивления
void Resist() {
  if (ina.voltsec < 2000) {
    print_mode(2);  // "error"
    Delay(1000);
    return;
  }
  Freq(EEPROM.read(8 + FREQDISCHAR));   // установить частоту работы разрядного модуля (4 кГц по умолчанию)
  bitSet(pam.MyFlag, CHARGE);           // старт работы режима
  dcdc.start(DCDC_DISCHARGE);           // старт разряда
  dcdc.Smooth_off();                    // отключить плавный пуск
  dcdc.begin((ina.voltsec >> 1), 800);  // задает напряжение и ток разряда
  sekd.start();                         // старт отсчета времени одна секунда
  uint16_t vlt = 0;
  uint8_t timer = 60;                   // время первого замера сек.
  do {
    //выполнять 10 сек
    sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.
    if (butt.tick == ENCHELD or butt.tick == STOPCLICK) {
      dcdc.Off();  // отключить заряд, разряд.
      bitClear(pam.MyFlag, CHARGE);
      return;
    }
    if (sekd.tick()) {
      // если прошла секунда вывод на дисплей
      setCursory();
      PrintVA(ina.voltsec, abs(ina.ampersec), 0, 0, 3);  // *12.361V 3.555A  *
      lcd.print(--timer);
      if (ina.voltsec - vlt <= 1) break;  // стабилизация напряжения до 1мВ
      vlt = ina.voltsec;
    }
  } while (timer);
  int16_t I[2]{ abs(ina.ampersec), 0 };  // сохранить ток
  uint16_t U[2]{ ina.voltsec, 0 };       // сохранить напряжение
  dcdc.begin((ina.voltsec >> 1), 4000);  // задает напряжение и ток разряда // 2000
  Delay(1000);                           // ожидание // 3000
  I[1] = abs(ina.ampersec);              // сохранить ток
  U[1] = ina.voltsec;                    // сохранить напряжение
  dcdc.Off();                            // отключить заряд, разряд.
  bitClear(pam.MyFlag, CHARGE);
  setCursory();
  PrintVA(U[1], I[1], 0, 0, 3);  // *12.36V 3.55A    *
  ClearStr(4);
  Delay(4000);                                                                                              // ожидание 4 сек
  float r = ((float)(U[0] - U[1]) / (float)(I[1] - I[0])) / 4.9f * 1000;                                    // 4,2    // 3,6f // расчитать внутренее сопротивление аккумулятора
  float curr = ((float)U[0] - ((float)(U[0] - U[1]) * (200.0f / (float)(I[1] - I[0]) / 10))) / (r * 4.5f);  //4,2  // расчет пускового тока - напряжение делить на сопротивление
  setCursory();
  lcd.print(r, 1);
  lcd.print(F("mOm "));
  lcd.print(curr, 1);
  lcd.write(65);  // А
  ClearStr(4);
  pam.Resist_1 = (uint16_t)(r * 100);
  pam.Resist_2 = (uint16_t)(curr * 10);
  Delay(7000);  // ожидание
}
#endif