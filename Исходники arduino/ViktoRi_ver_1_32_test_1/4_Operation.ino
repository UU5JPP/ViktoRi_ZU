void Operations() {
  const uint8_t mode = pam.Mode;  // сохранить режим работы
  // если флаг заряда установлен true то выполнять работу
  while (bitRead(pam.MyFlag, CHARGE)) {
    uint8_t roundi;
    EEPROM.get((MEM_KTC - 1), roundi);  // считать из памяти
    lcd.clear();
    print_mode(0);  //вывод режима
    setCursory();
    Saved();  // сохранить настройки
    Speaker(1000);
    // Выбор режима заряда
    switch (pam.Mode) {
      case 0:
        // Заряд
        ChargeAkb(pam.Volt_charge, pam.Current_charge, pam.Current_charge_min, (uint32_t)pam.Time_charge_h, false);  // заряд, дозаряд откл
        if (bitRead(pam.MyFlag, CHARGE)) {
          roundi--;
          if (!roundi) pam.Mode = 6;  // если счетчик циклов равен 0 то завершить заряд // иначе повторить заряд
        }
        break;
      case 1:
        // Заряд - Дозаряд
        switch (pam.Number) {
          case 1:
            // Заряд
            ChargeAkb(pam.Volt_charge, pam.Current_charge, pam.Current_charge_min, (uint32_t)pam.Time_charge_h, false);  // заряд дозаряд откл
            pam.Number++;
            break;
          case 2:
            // Дозаряд
            ChargeAkb(pam.Volt_add_charge, pam.Current_add_charge, pam.Current_add_charge_min, (uint32_t)pam.Time_add_charge_h, true);  // Включить дозаряд
            if (bitRead(pam.MyFlag, CHARGE)) {
              roundi--;
              if (!roundi) pam.Mode = 6;  // если счетчик циклов равен 0 то завершить заряд
              else pam.Number = 1;        // иначе повторить заряд - дозаряд
            }
            break;
        }
        break;
      case 2:
        // Заряд по Бранимиру
#if (BRANIMIR == 1)
        switch (pam.Number) {
          case 1:
            // Бранимир Заряд 20 часов после достижения максимального напряжения
            ChargeAkb(pam.Volt_charge, constrain((int16_t)pam.Capacity << 4, 150, 3500), constrain((int16_t)(1.414f * pam.Capacity), CURMIN, 250), 72, false);  // заряд (Напр. заряда, ток заряда, ток завершения заряда, время заряда, дозар. откл.

            if (bitRead(pam.MyFlag, BRANIM)) pam.Number = 3;         // если ток заряда снизился до установленного то переходим в дозаряд
            else if (bitRead(pam.MyFlag, BRANIMBOIL)) pam.Number++;  // иначе если время заряда превысило 20 часов
            if (!roundi) pam.Number = 3;                             //или если счетчик циклов равен 0 то перейти в дозаряд
            break;
          case 2:
            bitClear(pam.MyFlag, BRANIMBOIL);
            // Бранимир перемешивание
            ChargeAkb(pam.Volt_add_charge + 1000, constrain((int16_t)pam.Capacity * 20, 150, 5000), constrain(((int16_t)pam.Capacity << 3), CURMIN, 1500), 1, false);  // перемешивание (Напр. заряда, ток заряда, ток окончания заряда, время заряда, дозар. откл.
            roundi--;
            break;
          case 3:
            // Бранимир дозаряд
            if (pam.typeAkb < 5) ChargeAkb(pam.Volt_add_charge, constrain(bat.Cur(1), CUR_CHARGE_MIN, CURRMAXINT), pam.Current_add_charge / 3, 10, true);  // дозаряд (Напр. заряда, ток заряда, ток окончания заряда, время заряда, дозар. вкл.
            pam.Mode = 6;
            break;
        }
#else
        bitClear(pam.MyFlag, CHARGE);
#endif
        break;
      case 3:
        // Дозаряд
        ChargeAkb(pam.Volt_add_charge, pam.Current_add_charge, pam.Current_add_charge_min, (uint32_t)pam.Time_add_charge_h, true);  // Включить дозаряд
        if (bitRead(pam.MyFlag, CHARGE)) {
          roundi--;
          if (!roundi) pam.Mode = 6;  // если счетчик циклов равен 0 то завершить дозаряд
        }
        break;
      case 4:
        // Разряд
#if (DISCHAR == 1)
        Discharge();  //разряд аккумулятора
        if (bitRead(pam.MyFlag, CHARGE)) {
          roundi--;
          if (!roundi) bitClear(pam.MyFlag, CHARGE);  // если счетчик циклов равен 0 то завершить разряд// иначе повторить разряд
        }
#else
        bitClear(pam.MyFlag, CHARGE);
#endif
        break;
      case 5:
        //  Котнтрольно-тренировочный цикл
#if (DISCHAR == 1)
        {
          int32_t KTC[4];  //  Ah_discharge, Wh_discharge, Ah_charge, Wh_charge,
          EEPROM.get((((uint16_t)roundi << 4) - 16 + MEM_KTC), KTC);  // считать из памяти значения разряда, заряда roundi-цикла КТЦ
          switch (pam.Number) {
            case 1:
              // предварительный заряд
              ChargeAkb(pam.Volt_charge, pam.Current_charge, pam.Current_charge_min, (uint32_t)pam.Time_charge_h, false);  // заряд, дозаряд откл
              pam.Number++;
              Res_mem_char();     // сброс значений заряда
              break;
            case 2:
              // дать отстояться
              Wait(((uint16_t)pam.Capacity << 1));  // Функция ожидания - минут
              pam.Number++;
              break;
            case 3:
              // Разряд
             // Res_mem_dischar();  // сброс значений разряда
              Discharge();        //разряд аккумулятора
              pam.Number++;
              KTC[0] = pam.Ah_discharge;  //  Ah_discharge, Wh_discharge, Ah_charge, Wh_charge,
              KTC[1] = pam.Wh_discharge;
              EEPROM.put((((uint16_t)roundi << 4) - 16 + MEM_KTC), KTC);  // сохранить в памяти значения разряда roundi-цикла КТЦ
              break;
            case 4:
              // заряд
             // Res_mem_char();     // сброс значений заряда
              ChargeAkb(pam.Volt_charge, pam.Current_charge, pam.Current_charge_min, (uint32_t)pam.Time_charge_h, false);  // заряд, дозаряд откл
              KTC[2] = pam.Ah_charge;                                                                                      //  Ah_discharge, Wh_discharge, Ah_charge, Wh_charge,
              KTC[3] = pam.Wh_charge;
              EEPROM.put((((uint16_t)roundi << 4) - 16 + MEM_KTC), KTC);  // сохранить в памяти значения разряда roundi-цикла КТЦ
              if (bitRead(pam.MyFlag, CHARGE)) {
                roundi--;
                if (!roundi) pam.Number++;  // если счетчик циклов равен 0 то перейти в Дозаряд
                else {
                  // иначе повторить КТЦ
                  pam.Number = 2;
                  Res_mem_dischar();  // сброс значений разряда
                  Res_mem_char();     // сброс значений заряда
                }
              }
              break;
            case 5:
              // Дозаряд
              if (pam.typeAkb < 5) ChargeAkb(pam.Volt_add_charge, pam.Current_add_charge, pam.Current_add_charge_min, (uint32_t)pam.Time_add_charge_h, true);  // Включить дозаряд
              pam.Mode = 6;                                                                                                                                    // перейти в Хранение
              break;
          }
        }
#else
        bitClear(pam.MyFlag, CHARGE);
#endif
        break;
      case 6:  // хранение
        Storage(true);
        pam.Mode = mode;
        bitClear(pam.MyFlag, CHARGE);
        break;
      case 7:  // буферный режим
        Storage(false);
        pam.Mode = mode;
        bitClear(pam.MyFlag, CHARGE);
        break;
    }                                   // выбор режима
    EEPROM.put((MEM_KTC - 1), roundi);  // сохранить количество циклов

    if (BitIsClear(pam.MyFlag, CHARGE)) {
      Saved();  // сохранить настройки
      End();    // Завершение работы
    }
  }
  // цикл работы
}
