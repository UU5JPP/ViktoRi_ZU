// функция сохранения настроек
void Saved() {
  EEPROM.put(1, vkr);  
  EEPROM.put((int)(sizeof(pam) * vkr.profil + MEM), pam);  // обновить значения в памяти
}

const uint8_t data_service[] PROGMEM = {
  POWER,                        // *напряжение от БП
  POWER_MAX,                    // *максимально допустимое напряжение от БП
  INAVOLTKOOF,                  // коэффициент напряжения INA226
  (uint8_t)(CURR_MAX * 10),     // Максимальный ток зарядника 25.5А
  FAN,                          // *тип управления вентилятором
  DEG_ON,                       // *температура включения вентилятора в градусах
  DEG_OFF,                      // *температура отключения вентилятора в градусах
  DEG_MAX,                      // *значение температуры для максимальных оборотов вентилятора ШИМ.
  DEG_CUR,                      // *температура при которой уменьшается ток заряда
  (uint8_t)(CURFAN_ON / 100),   // *значение тока в Амперах/100 при котором включается вентилятор. (до 25,5А)
  (uint8_t)(CURFAN_OFF / 100),  // *значение тока в Амперах/100 при котором отключается вентилятор. (до 25,5А)
  (uint8_t)(CURFAN_MAX / 100),  // *значение тока в Амперах/100  для максимальных оборотов вентилятора ШИМ. (до 25,5А)
  TIME_LIGHT,                   // *время подсветки дисплея в минутах TIME_LIGHT (до 255 минут)
  FREQ_CHARGE,                  // *частота силового модуля {0.25, 0.5, 1, 2, 4, 8, 30, 60} кГц
  FREQ_DISCHARGE,               // *частота разрядного модуля {0.25, 0.5, 1, 2, 4, 8, 30, 60} кГц
};

// сброс настроек по умолчанию
void Reset_settings(uint8_t sett) {
  lcd.clear();
  disp.printFromPGM((int)(&settings[POINTSETTINGS]));
  bool ext = true;
  do {
    setCursory();
    lcd.write(RIGHTs);  // >
    switch (sett) {
      case 0:
        lcd.print(F(txt5));  // "Exit"
        break;
      case 1:
        lcd.print(F(txt10));  // "All" Стереть все
        break;
      case 2:
        print_mode(3);  // "Settings"
        break;
      case 3:
        disp.printFromPGM((int)(&menuTxt[8]));  // "Calibration"
        break;
      case 4:
        disp.printFromPGM((int)(&settings[19]));  // "!system param!"
        break;
    }
    ClearStr(12);
    pauses();
    switch (butt.tick) {
      case LEFT: 
      case RIGHT:     
        sett = constrain(sett + butt.tick, 0, 4);
        break;
      case STOPCLICK:
      case ENCHELD:
        return;  // выйти
      case ENCCLICK:
      case OKCLICK:
        ext = false;  // выйти из цикла
        break;
    }
  } while (ext);
  switch (sett) {
    case 0: break;  // выйти
    case 1:
    // сбросить все
    case 2:
      // сбросить настройки по умолчанию
      pam.MyFlag = 0b00000000;           // - вкл/откл  MyFlag (o,o,o,o,o,precharge,Branimir,Oscillation,Charge) - 0b1000
      pam.typeAkb = 0;                   // - тип аккумулятора
      pam.Mode = 0;                      // - режм заряда
      pam.Capacity = 60;                 // - емкость аккумулятора
      pam.Voltage = 6;                   // - количество ячеек аккумулятора
      pam.Number = 1;                    // - номер
      pam.Round = 1;                     // - текущий круг
      pam.Volt_charge = 14700;           // - напряжение заряда - миливольт * 1000 (макс 65.535 Вольт)
      pam.Volt_decrCur = Volt_DCur();    // напряжение при котором начинается снижение тока заряда
      pam.Volt_add_charge = 16200;       // - напряжение дозаряда - миливольт * 1000 (макс 65.535 Вольт)
      pam.Current_charge = 6000;         // - ток заряда - миллиампер * 1000 (макс 65.535 Ампер)   0,01С
      pam.Current_add_charge = 1200;     // - ток дозаряда - миллиампер * 1000 (макс 65.535 Ампер) 0.02C
      pam.Current_charge_min = 120;      // - ток завершения заряда - миллиампер * 1000 (макс 65.535 Ампер) 0.003C
      pam.Current_add_charge_min = 600;  // - ток завершения дозаряда - миллиампер * 1000 (макс 65.535 Ампер)
      pam.Time_charge_h = 15;            // - время заряда, часы
      pam.Time_add_charge_h = 10;        // - время дозаряда, часы
      pam.Volt_discharge = 11600;        // - напряжение разряда - миливольт * 1000 (макс 65.535 Вольт)
      pam.Current_discharge = 1800;      // - ток разряда - миллиампер * 1000 (макс 65.535 Ампер)
      pam.Current_discharge_min = 1800;  // - ток завершения разряда - миллиампер * 1000 (макс 32,767 Ампер)
      pam.Volt_buffer = 13800;           // - напряжение буферного (выравнивающего режима) - миливольт * 1000 (макс 65.535 Вольт)
      pam.Volt_storage = 13100;          // - напряжение хранения - миливольт * 1000 (макс 65.535 Вольт)
      pam.Volt_pre_charge = 13800;       // - напряжение предзаряда (для сильно разр. аккумул) - миливольт * 1000 (макс 65.535 Вольт)
      pam.Current_pre_charge = 2000;     // - ток предзаряда (для сильно разр. аккумул) - миллиампер * 1000 (макс 65.535 Ампер)
      pam.Resist_1 = 0;                  // - внутренее сопротивление аккумулятора - миллиОм * 100 - до
      pam.Resist_2 = 0;                  // - внутренее сопротивление аккумулятора - миллиОм * 100 - после
      Res_mem_char();
      Res_mem_dischar();

      lcd.clear();
      lcd.print(F(txt12));  // "Sbros"
      for (uint8_t l = 0; l <= PROFIL; l++) {
        EEPROM.put((int)(sizeof(pam) * l + MEM), pam);  // обновить значения в памяти
        lcd.write(46);                                  // ........
      }
      Res_ktc();  // очистить память КТЦ
      if (sett != 1) break;
    case 3:
      // сбросить калибровки по умолчанию
      vkr.GlobFlag = 0;                               // Q1 - состояние силового транзистора, состояние INA226/
      vkr.Ohms = OHMS;                                // кооффицмент коррекции напряжения 0-255
      vkr.profil = 0;                                 // текущий профиль 0-6
      vkr.Vref = VREF;                                // референсное напряжение
      vkr.shunt = (uint16_t)((float)SHUNT * 100000);  // значение шунта
      EEPROM.put(1, vkr);
    case 4:
       // сбросить системные параметры по умолчанию
      for (byte i = 0; i < sizeof(data_service); i++) {
        EEPROM.update(8 + i, pgm_read_byte(&data_service[i]));
      }
      break;
  }
  lcd.clear();
  if (sett) {
    print_mode(3);       // "Settings"
    lcd.print(F(txt2));  // сброс
    setCursory();
    lcd.print((sizeof(pam)));
    lcd.print(F("B "));
    lcd.print((sizeof(vkr)));
    lcd.write(66);  // B
    EEPROM.update(0, MEMVER);
    Delay(3000);
    if (sett == 1 or sett >= 3) resetFunc();
    lcd.clear();
  }
}
