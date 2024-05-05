// Меню версии 4.0 для дисплея LCD1602
void Menu1602() {
  if (bitRead(pam.MyFlag, CHARGE)) return;  // если флаг заряда установлен true то выйти иначе выполнять Menu
#if (TIME_LIGHT)
  disp.Light_high();  // включение подсветки
#endif
#if (POWPIN == 1)
  bitSet(flag_global, RELEY_OFF);  // разрешено отключать реле
#endif
  pam.Number = 1;
  Saved();                    // сохранить настройки
  uint8_t urovenmenu = 0;     // три уровня меню
  uint8_t main_menu = 0;      // номер пункта меню
  uint8_t point_setting = 0;  // номер пункта настроек
  bool printx = true;
  int8_t x = 0;
  sekd.start();
  ina.start(1);  // старт замеров INA
  do {
    // цикл меню
    switch (urovenmenu) {
      // основное меню urovenmenu
      case 0:
        point_setting = 0;
        // вывод меню
        if (printx) {
          printx = false;
          //if (++mode >= MODES) mode = 0;     // на увеличение
          //if (--mode < 0) mode = MODES - 1;  // на уменьшение
          if (x) main_menu = (x > 0) ? ((main_menu == POINTMENU) ? 0 : main_menu + x) : ((main_menu == 0) ? POINTMENU : main_menu + x);  // реализация кругового меню
          lcd.clear();
          disp.printFromPGM((int)(&menuTxt[main_menu]));
          setCursory();
          switch (main_menu) {
            case 0:
              // профиль
              print_Capacity();
              Vout();                 // *Pr:0 Pb CA/Ca   *
              lcd.print(vkr.profil);  // *60Ah 12.60V(6)  *
              lcd.setCursor(5, 0);
              print_mode(1);  // вывод типа акб
              break;
            case 1:
              print_Capacity();  // емкость акб
              break;
            case 2:
              print_mode(1);  // вывод типа акб
              break;
            case 3:
              Vout();  // напряжение
              break;
            case 4:
              print_mode(0);  //  режим
              break;
          }
        }
        if (main_menu < POINTMENU) {
          if (butt.tick == ENCCLICK or butt.tick == OKCLICK) {
            urovenmenu = (main_menu < 5) ? 2 : 1;
            point_setting = 0;
            printx = true;
            break;
          }
          // удержание Пуск или Энкодера запускает заряд

          if (butt.tick == ENCHELD or butt.tick == OKHELD) {
            // Бранимир, Дозаряд, КТЦ
            if (pam.Mode == ADDCRb and pam.typeAkb > 4) {
              lcd.clear();
              print_mode(1);
              printSimb();  // пробел
              print_mode(0);
              setCursory();
              print_mode(2);  // "error"
              Speaker(1000);  // функция звукового сигнала (время в миллисекундах)
              Delay(3000);
            } else {
              bitSet(pam.MyFlag, CHARGE);  // запустить работу режима, выйти из цикла меню
            }
            printx = true;
          }
        }
        break;  // основное меню
      case 1:
        // urovenmenu 1 - подменю
        switch (main_menu) {
          case 0:
            Saved();  // сохранить настройки
            urovenmenu = 0;
            printx = true;
            break;
          case 1:
            // емкость акб
          case 2:
            // тип  акб
            // расчет при выборе емкости, типа и количества ячеек(напряжения) Акб
            calculate(1);  // расчет токов
          case 3:
            calculate(2);  // расчет напряжений
            urovenmenu = 0;
            printx = true;
            break;
          case 4:
            urovenmenu = 0;
            printx = true;
            break;
          case 5:
            // настройки
            if (printx) {
              printx = false;
              if (x) point_setting = (x > 0) ? ((point_setting == POINTSETTINGS) ? 0 : point_setting + x) : ((point_setting == 0) ? POINTSETTINGS : point_setting + x);
              lcd.clear();
              disp.printFromPGM((int)(&settings[point_setting]));
              setCursory();
              Settings_point(point_setting, false, 0);  // вывод/изменение пунктов настроек
            }
            switch (butt.tick) {
              case STOPCLICK:
              case ENCHELD:
                urovenmenu = 0;
                point_setting = 0;
                printx = true;
                Saved();  // сохранить настройки
                lcd.clear();
                print_mode(3);       // "Settings"
                lcd.print(F(txt3));  // saved
                Delay(1000);
                break;
              case ENCCLICK:
              case OKCLICK:
                urovenmenu = 2;
                printx = true;
                break;
            }
            break;
          case 6:
            // измерение сопротивления
#if (RESIST == 1)
            Resist();
            Saved();  // сохранить настройки
#endif
            urovenmenu = 0;
            printx = true;
            break;
          case 7:
            // статистика
            if (printx) {
              printx = false;
              if (x) point_setting = constrain(point_setting + x, 0, 3 + pam.Round);
              lcd.clear();
              Statistics_point(point_setting);  // вывод статистики
            }
            switch (butt.tick) {
              case ENCCLICK:
              case OKCLICK:
              case STOPCLICK:
                urovenmenu = 0;
                point_setting = 0;
                printx = true;
                break;
            }
            break;
          case 8:
            // калибровка
            if (printx) {
              printx = false;
              lcd.clear();
              lcd.setCursor(1, 0);
              disp.printFromPGM((int)(&calibr[0]));
              lcd.setCursor(1, 1);
              disp.printFromPGM((int)(&calibr[1]));
              if (x) point_setting = constrain(point_setting + x, 0, POINTCALIBRS);
              lcd.setCursor(0, point_setting);
              lcd.write(RIGHTs);  // >
            }
            switch (butt.tick) {
              case STOPCLICK:
              case ENCHELD:
                urovenmenu = 0;
                point_setting = 0;
                printx = true;
                break;
              case ENCCLICK:
              case OKCLICK:
                urovenmenu = 2;
                printx = true;
                break;
            }
            break;
        }
        break;
      case 2:
        //urovenmenu -  изменение параметров
        if (printx) {
          printx = false;
          setCursory();
          if (main_menu) lcd.write(LEFTs);  // <
          switch (main_menu) {
            case 0:
              // выбор профиля
              vkr.profil = constrain(vkr.profil + x, 0, PROFIL);
              EEPROM.get((int)(sizeof(pam) * vkr.profil + MEM), pam);  // читаю из памяти значения
              print_Capacity();
              Vout();
              lcd.write(LEFTs);  // <            // *Pr:<0>Pb CA/Ca  *
              lcd.print(vkr.profil);
              lcd.write(RIGHTs);  // >
              print_mode(1);      // вывод типа акб
              ClearStr(4);        // очистить поле
              break;
            case 1:
              // емкость
              pam.Capacity = constrain(pam.Capacity + x, 1, 255);
              lcd.print(pam.Capacity);
              ClearStr(4);  // очистить поле
              break;
            case 2:
              // выбор типа аккумулятора
              pam.typeAkb = constrain(pam.typeAkb + x, 0, 8);
              print_mode(1);  // вывод типа акб
              ClearStr(4);    // очистить поле
              break;
            case 3:
              // напряжение
              pam.Voltage = constrain(pam.Voltage + x, 1, 100);  // 100 - максимальное количество ячеек акб
              if (bat.Volt(0) > POWERINT) pam.Voltage -= x;
              print_mode(4);  // 12.6V
              lcd.write(40);  // (
              lcd.print(pam.Voltage);
              lcd.write(41);  // )
              ClearStr(2);    // очистить поле
              break;
            case 4:
              // выбор режима
              pam.Mode = constrain(pam.Mode + x, 0, POINTMODE);
              print_mode(0);
              ClearStr(10);  // очистить поле
              break;
            case 5:
              // настройки
              Settings_point(point_setting, true, x);  // вывод/изменение пунктов настроек
              if (point_setting == POINTSETTINGS) {    // сброс настроек по умолчанию
                urovenmenu = 0;
                main_menu = 0;
                printx = true;
              }
              if (butt.tick == OKCLICK or butt.tick == ENCCLICK or butt.tick == STOPCLICK or point_setting == SYSPARAM) {
                urovenmenu = 1;
                printx = true;
              }
              break;
            case 8:
              // калибровка
              Korrect(point_setting);
              //Saved();  // сохранить настройки
              urovenmenu = 1;
              printx = true;
              break;
          }
          if (main_menu and main_menu < POINTSETTINGS - 2) {
            lcd.setCursor(DISPLAYx - 1, 1);
            lcd.write(RIGHTs);  // >
          }
        }

        if (main_menu < 5 and (butt.tick == ENCCLICK or butt.tick == OKCLICK or butt.tick == STOPCLICK)) {
          urovenmenu = 1;
          printx = true;
        }
        break;  // изменение параметров urovenmenu = 2
    }
    // -- меню
    x = 0;
    // ожидание действий пользователя
    do {
      sensor_survey();
      if (sekd.tick()) {
        // если прошла секунда
        // вывод на дисплей напряжения, тока, температуры раз в одну секунду
        if (main_menu == POINTMENU) {
          // *24.12V 25C 20C   *
          // *12.652V 0.061A  *
#if (VOLTIN == 1)  // напряжение от БП
          setCursorx();
          PrintVA(vin.volt(), 0, 0, 0, 2);
#endif
          lcd.setCursor(7, 0);
#if (SENSTEMP1)
            print_tr(kul.tQ1);  // температура
#endif  
#if (SENSTEMP2 == 2)
          print_tr(ntc.akb);  // чтение температуры с датчика 2 акб
#endif
          setCursory();
          PrintVA(ina.voltsec, ina.ampersec, 0, 0, 3);  // *12.361V -0.253A *
          // print_memoryFree(); // вывод свободной оперативки
        }
#if (LOGGER)
        Serial_out(0);
#endif
      }  // --- выполнить раз в секунду

    } while (!butt.tick and !printx);
    // --- ожидание действий пользователя
    switch (butt.tick) {
      case RIGHT:
      case LEFT:
        x = butt.tick;
        break;
    }
    printx = true;
  } while (BitIsClear(pam.MyFlag, CHARGE));  // цикл меню

  if (pam.Mode != 4) Res_mem_char();                      // сброс значений заряда
  if (pam.Mode == 4 or pam.Mode == 5) Res_mem_dischar();  // сброс значений разряда
  if (pam.Mode == 5) Res_ktc();                           // очистить память КТЦ
  if (pam.Mode == 2) pam.Round = 3;                       // для Бранимира минимум 3 раунда
  regim_end = 0;
  if (pam.Round < 1) pam.Round = 1;
  EEPROM.put((MEM_KTC - 1), pam.Round);  // сохранить количество циклов
}
