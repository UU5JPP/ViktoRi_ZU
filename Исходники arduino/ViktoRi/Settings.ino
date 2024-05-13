// вывод пунктов настроек
void Settings_point(uint8_t point, bool edit, int8_t x) {
  if (!edit) {
    // отображение настроек
    switch (point) {
      case 0:
        PrintVA(pam.Volt_charge, 0, 0, 0, 2);  // напряжение заряда
        break;
      case 1:
        PrintVA(0, pam.Current_charge, 0, 0, 2);  // ток заряда
        break;
      case 2:
        PrintVA(pam.Volt_decrCur, 0, 0, 0, 2);  //     Напряжение при котором начинает снижаться ток
        break;
      case 3:
        PrintVA(0, pam.Current_charge_min, 0, 0, 2);  // минимальный ток заряда
        break;
      case 4:
        lcd.print(pam.Time_charge_h);  // время заряда
        lcd.write(104);                // h
        break;
      case 9:
        PrintVA(pam.Volt_discharge, 0, 0, 0, 2);  // напряжение разряда
        break;
      case 10:
        PrintVA(0, pam.Current_discharge, 0, 0, 2);  // ток разряда
        break;
      case 11:
        PrintVA(0, pam.Current_discharge_min, 0, 0, 2);  // ток завершения разряда
        break;
      case 12:
        PrintVA(pam.Volt_buffer, 0, 0, 0, 2);  // напряжение буферный режим
        break;
      case 13:
        PrintVA(pam.Volt_storage, 0, 0, 0, 2);  // напряжение режима хранения
        break;
      case 14:  // Предзаряд
        PrintOnOff(bitRead(pam.MyFlag, PRECHAR));
        break;
      case 15:
        PrintVA(pam.Volt_pre_charge, 0, 0, 0, 2);  // напряжение предзаряда
        break;
      case 16:
        PrintVA(0, pam.Current_pre_charge, 0, 0, 2);  // ток предзаряда
        break;
      case 18:
        lcd.print(pam.Round);  // колицество циклов
        break;

        if (pam.typeAkb < 5) {
            //switch (point) {
          case 5:
            PrintVA(pam.Volt_add_charge, 0, 0, 0, 2);  // напряжение дозаряда
            break;
          case 6:
            PrintVA(0, pam.Current_add_charge, 0, 0, 2);  // ток дозаряда
            break;
          case 7:
            PrintVA(0, pam.Current_add_charge_min, 0, 0, 2);  // минимальный ток дозаряда
            break;
          case 8:
            lcd.print(pam.Time_add_charge_h);  // время дозаряда
            lcd.write(104);                    // h
            break;
          case 17:
            PrintOnOff(bitRead(pam.MyFlag, OSCIL));  // режим качели
            break;
        }
    }
  } else {
    // изменение настроек
    switch (point) {
      case 0:
        // напряжение заряда
        pam.Volt_charge = constrain(pam.Volt_charge + x * STEP_VOLT, 1000, POWERINT);
        PrintVA(pam.Volt_charge, 0, 0, 0, 2);
        break;
      case 1:
        // ток заряда
        pam.Current_charge = constrain(pam.Current_charge + x * STEP_CURR, CUR_CHARGE_MIN, CURRMAXINT);
        PrintVA(0, pam.Current_charge, 0, 0, 2);
        ChargeTimeCalc();  // расчет времени заряда
        break;
      case 2:
        //  Напряжение при котором начинает снижаться ток
        pam.Volt_decrCur = constrain(pam.Volt_decrCur + x * STEP_VOLT, 1000, pam.Volt_charge);
        PrintVA(pam.Volt_decrCur, 0, 0, 0, 2);
        break;
      case 3:
        // минимальный ток заряда
        pam.Current_charge_min = constrain(pam.Current_charge_min + x * 10, CURMIN, pam.Current_charge);
        PrintVA(0, pam.Current_charge_min, 0, 0, 2);
        break;
      case 4:
        // время заряда
        pam.Time_charge_h = constrain(pam.Time_charge_h + x, 1, 255);
        lcd.print(pam.Time_charge_h);
        break;
      case 9:
        // напряжение разряда
        pam.Volt_discharge = constrain(pam.Volt_discharge + x * STEP_VOLT, 1000, POWERINT);
        PrintVA(pam.Volt_discharge, 0, 0, 0, 2);
        break;
      case 10:
        // ток разряда
        pam.Current_discharge = constrain(pam.Current_discharge + x * STEP_CURR, 50, CURRMAXINT);
        PrintVA(0, pam.Current_discharge, 0, 0, 2);
        pam.Current_discharge_min = pam.Current_discharge;  // ток завершения разряда
        break;
      case 11:
        // ток завершения разряда
        pam.Current_discharge_min = constrain(pam.Current_discharge_min + x * STEP_CURR, 50, pam.Current_discharge);
        PrintVA(0, pam.Current_discharge_min, 0, 0, 2);
        break;
      case 12:
        // напряжение буферного режима
        pam.Volt_buffer = constrain(pam.Volt_buffer + x * STEP_VOLT, 1000, POWERINT);
        PrintVA(pam.Volt_buffer, 0, 0, 0, 2);
        break;
      case 13:
        // напряжение режима Хранение
        pam.Volt_storage = constrain(pam.Volt_storage + x * STEP_VOLT, 1000, POWERINT);
        PrintVA(pam.Volt_storage, 0, 0, 0, 2);
        break;
      case 14:
        // Предзаряд
        if (x) InvBit(pam.MyFlag, PRECHAR);
        PrintOnOff(bitRead(pam.MyFlag, PRECHAR));
        break;
      case 15:
        // напряжение предзаряда
        pam.Volt_pre_charge = constrain(pam.Volt_pre_charge + x * STEP_VOLT, 1000, POWERINT);
        PrintVA(pam.Volt_pre_charge, 0, 0, 0, 2);
        break;
      case 16:
        // ток предзаряда
        pam.Current_pre_charge = constrain(pam.Current_pre_charge + x * 10, CUR_CHARGE_MIN, CURRMAXINT);
        PrintVA(0, pam.Current_pre_charge, 0, 0, 2);
        break;
      case 18:
        // цикл
        pam.Round = constrain(pam.Round + x, 1, 10);
        lcd.print(pam.Round);
        break;
#if (SERVICE == 1)
      case 19:
        // системные пaраметры на 3 нажатия
#if (ENCBUTT == 0)  // 0 - только энкодер
        if (enc.clicks == 3) {
          enc.clicks = 0;
#elif (ENCBUTT == 1)  // 1 - только кнопки
        if (OK.clicks == 3) {
          OK.clicks = 0;
#else                 // иначе - энкодер + OK + Stop или все
        if (enc.clicks == 3 or OK.clicks == 3) {
          enc.clicks = 0;
          OK.clicks = 0;
#endif
          serviceparam();  // изменение системныx параметров
          lcd.clear();
        }
#endif
        break;
      case 20:
        // сброс всех настроек
        Reset_settings(0);  // сброс настроек по умолчанию
        lcd.clear();
        break;
    //}

    if (pam.typeAkb < 5) {  
        case 5:
          // напряжение дозаряда
          pam.Volt_add_charge = constrain(pam.Volt_add_charge + x * STEP_VOLT, 1000, POWERINT);
          PrintVA(pam.Volt_add_charge, 0, 0, 0, 2);
          break;
        case 6:
          // ток дозаряда
          pam.Current_add_charge = constrain(pam.Current_add_charge + x * STEP_CURR, CUR_CHARGE_MIN, CURRMAXINT);
          PrintVA(0, pam.Current_add_charge, 0, 0, 2);
          break;
        case 7:
          // минимальный ток дозаряда
          pam.Current_add_charge_min = constrain(pam.Current_add_charge_min + x * 10, CURMIN, pam.Current_add_charge);
          PrintVA(0, pam.Current_add_charge_min, 0, 0, 2);
          break;
        case 8:
          // время дозаряда
          pam.Time_add_charge_h = constrain(pam.Time_add_charge_h + x, 1, 252);
          lcd.print(pam.Time_add_charge_h);
          break;
        case 17:
          // режим качели
          if (x) InvBit(pam.MyFlag, OSCIL);  // инвертировать бит
          PrintOnOff(bitRead(pam.MyFlag, OSCIL));
          break;
      }
    }
    ClearStr(10);  // очистить поле
  }
}

// печать статистики
void Prstatistic(int i, uint32_t time_i, int32_t Ah, int32_t Wh) {
  disp.printFromPGM(i);
  Print_time(time_i, DISPLAYx - 5, 0);
  setCursory();
  PrintVA(0, 0, Ah, Wh, 2);
}
// вывод статистики
void Statistics_point(uint8_t point) {
  switch (point) {
    case 0:
      // Заряд
      // *Zaryad     24:42*
      // *55.36Ah 366.35Wh*
      Prstatistic((int)(&modeTxt[0]), pam.Time_charge, pam.Ah_charge, pam.Wh_charge);
      break;
    case 1:
      //  Дозаряд
      // *Add charge 24:42*
      // *55.36Ah 366.35Wh*
      Prstatistic((int)(&modeTxt[3]), pam.Time_addcharge, pam.Ah_addcharge, pam.Wh_addcharge);
      break;
    case 2:
      // Разряд
      // *Razryad    24:42*
      // *55.36Ah 366.35Wh*
      Prstatistic((int)(&modeTxt[4]), pam.Time_discharge, pam.Ah_discharge, pam.Wh_discharge);
      break;
    case 3:
      // Внутреннее сопротивление
      // *Resist       *
      // *20.3mOm 520A *
      disp.printFromPGM((int)(&menuTxt[6]));  // *Resist       *
      setCursory();
      lcd.print(((float)pam.Resist_1 / 100), 1);
      lcd.print(F("mOm "));
      lcd.print((float)pam.Resist_2 / 10, 1);  // вывод пускового тока - напряжение делить на сопротивление
      lcd.write(65);            // А
      break;
    default:
      // вывод значений емкости КТЦ
      {
        int32_t KTC[4] = { 0 };  //  Ah_discharge, Wh_discharge, Ah_charge, Wh_charge,
        uint16_t m = pam.Round;  // считать количество циклов в переменную
        m -= (point - 4);
        EEPROM.get(((m << 4) - 16 + MEM_KTC), KTC);  // считать из памяти значения разряда, заряда m-цикла КТЦ
        lcd.print(point - 3);                        // 1
        lcd.print(F("D:"));                          // 1D:
        PrintVA(0, 0, KTC[0], 0, 1);                 // 1D:65.3Ah
        PrintVA(0, 0, 0, KTC[1], 0);                 // 1D:65.3Ah 255Wh
        setCursory();
        lcd.print(point - 3);         // 1
        lcd.print(F("C:"));           // 1C:
        PrintVA(0, 0, KTC[2], 0, 1);  // 1C:65.3Ah
        PrintVA(0, 0, 0, KTC[3], 0);  // 1C:65.3Ah 255Wh
        break;
      }
  }
}

/*
*                *
------------------
*1D:65.3Ah 255Wh *
*1C:66.4AH 257Wh *
------------------
*/
