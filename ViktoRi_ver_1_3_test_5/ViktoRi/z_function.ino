void sensor_survey(void) {
  butt.Tick();  // опрос кнопок и энкодера
  if (ina.sample()) {
    if (bitRead(flag_global, DCDCMODE)) dcdc.Control();  // регулировка тока и напряжения
    else dcdc.Control_dich();
#if (VOLTIN == 1)
    vin.sample();  // измерение напряжения БП
#endif
  }
  static uint32_t sec = millis();
  if (millis() - sec >= 1000) {
    sec = millis();
    kul.fan();  // чтение температуры и управение кулером
#if (POWPIN == 1)
    Relay();  // функция управления реле подключения к сети 220В
#endif
  }
}


// функция хранения акб и буферного режима
void Storage(bool regim) {
  bitSet(flag_global, DCDCMODE);  // режим dcdc - Заряд
  bool ext = true;
  bool vvmin = true;                       // флаг отключения нагрузки
  uint32_t t_st = 0;                       // время хранения
  uint8_t sig = 0;                         // время отстчета сигнала буферного режима
  uint16_t vbc = bat.Volt(3) / 100 * 100;  // расчет напряжения - снижение до 12В
#if (GUARDA0)
  Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
#if (POWPIN == 1)
  gio::high(RELAY220);               // включить сеть 220В
  bitClear(flag_global, RELEY_OFF);  // запрещено отключать реле
#endif
  Freq(vkr.service[FREQCHARGE]);  // установить частоту работы силового модуля

  {
    int16_t amp = (regim ? (constrain(pam.Capacity << 2, 50, 1000)) : pam.Current_charge);  // максимальный ток заряда
    uint16_t vlt = regim ? pam.Volt_storage : pam.Volt_buffer;
    PrintVA(vlt, amp, 0, 0, 1);  // вывод напряжения и тока заряда
    Delay(3000);
    dcdc.begin(vlt, amp);  // задать максимальные значения напряжения и тока заряда
  }
  if (!regim) gio::high(PIN_13);  // включить пин 13. Включить реле нагрузки
#if (VOLTIN == 1)
  vin.start();  // старт замеров напряжения от БП
#endif

  sekd.start();  // старт отсчета времени одна секунда
  ina.start(1);  // старт замеров INA
  dcdc.start();
  while (ext) {
    // цикл работы
    sensor_survey();                                                 // опрос кнопок, INA226, напря. от БП., контроль dcdc.
    if (butt.tick == ENCHELD or butt.tick == STOPHELD) ext = false;  // завершить заряд

    // если прошла секунда
    if (sekd.tick()) {
#if (GUARDA0)
      Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
      Print_time(t_st++, DISPLAYx - 8, 0);          // *Storage 12:10:36*
      setCursory();                                 // *13.10V 6.12A 25C*
      PrintVA(ina.voltsec, ina.ampersec, 0, 0, 2);  // вывод на дисплей во float - напряжение, ток, ампер-часы, ватт-часы, кол. разрядов после запятой.
      print_tr(kul.tQ1);                            // чтение температуры
#if (SENSTEMP2 == 2 and DISPLAYx == 20)
      print_tr(tr_bat.getTempInt());  // чтение температуры с датчика 2 акб
#endif

      // сигнализация о снижении напряжения акб ниже предела (12V)
      if (ina.voltsec < vbc) {
        if (!sig) {
#if (TIME_LIGHT)
          disp.Light_high();  // включение подсветки
#endif
          Speaker(300);  // функция звукового сигнала (время в миллисекундах)
          sig = 60;      // сигнал раз в 60 секунд
        } else sig--;

        if (!regim) {
          // включен буферный режим
          if (vvmin and ina.voltsec < bat.Volt(3)) {
            // если нагрузка подключена и напряжение снизилось менее напряжения разряда то
            Speaker(300);  // функция звукового сигнала (время в миллисекундах)
            vvmin = false;
            gio::low(PIN_13);  // отключить пин 13. отключить реле нагрузки
          }
        }
      }
      if (!vvmin and ina.voltsec > pam.Volt_buffer - 200) {
        // когда реле нагрузки отключено и напряжение увеличилось до максимального то
        Speaker(300);  // функция звукового сигнала (время в миллисекундах)
        vvmin = true;
        gio::high(PIN_13);  // включить пин 13. отключить реле нагрузки
      }
#if (TIME_LIGHT)
      if (!regim) disp.Light_low();  // отключение подсветки через 3 минуты в буферном режиме, если разрешено отключение
#endif
    }
    // если прошла секунда
  }
  // цикл работы
  gio::low(PIN_13);  // отключить пин 13. отключить реле нагрузки
  dcdc.Off();  // отключить заряд, разряд.
#if (GUARDA0)
  gio::low(PINA0);  // отключить блокировку модуля защиты
#endif
}

// расчет токов и напряжений
void calculate(uint8_t i) {
  switch (i) {
    case 1:
      pam.Current_charge = constrain(bat.Cur(0), CUR_CHARGE_MIN, CURRMAXINT);       // ток заряда
      pam.Current_charge_min = constrain((int16_t)pam.Capacity << 1, CURMIN, 200);  // ток завершения заряда
      pam.Current_discharge = constrain(bat.Cur(2), 50, CURRMAXINT);                // ток разряда
      pam.Current_discharge_min = pam.Current_discharge;                            // ток завершения разряда
      pam.Current_add_charge = constrain(bat.Cur(1), CUR_CHARGE_MIN, CURRMAXINT);   // ток дозаряда
      pam.Current_add_charge_min = pam.Current_add_charge / 3;                      // минимальный ток дозаряда
      pam.Current_pre_charge = constrain(pam.Current_charge / 3, 200, CURRMAXINT);  // ток предзаряда
      ChargeTimeCalc();                                                             // расчет времени заряда
      pam.Round = 1;                                                                // 1 раунд
      break;
    case 2:
      // расчет напряжений
      {
        pam.Volt_charge = bat.Volt(1);     // напряжение заряда, напряжение заряда ячейки на количество ячеек
        pam.Volt_decrCur = Volt_DCur();    // напряжение при котором начинается снижение тока заряда
        pam.Volt_discharge = bat.Volt(3);  // напряжение разряда
        uint16_t vbc = bat.Volt(0);
        pam.Volt_storage = (uint16_t)(vbc * 1.04f);      // напряжение хранения
        pam.Volt_buffer = (uint16_t)(vbc * 1.1f);        // напряжение буферного режима
        if (pam.typeAkb < 5) {                           // расчет для свинцово кислотных Акб
          pam.Volt_add_charge = bat.Volt(2);             // напряжение дозаряда
          pam.Volt_pre_charge = (uint16_t)(vbc * 1.1f);  // напряжение предзаряда
        }
      }
      break;
  }
}

#if (POWPIN == 1)
// функция управления реле подключения к сети 220В
void Relay(void) {
  static uint32_t tvin = 0;
  if (ina.voltsec < 9000) {
    // если напряжение подключенной АКБ ниже 9 вольт включается реле 220В.
    gio::high(RELAY220);                                                                          // включить питание реле (подкл. питание от сети 220В)
    tvin = millis();                                                                              // запомнить время
  } else if (bitRead(flag_global, RELEY_OFF) and (millis() - tvin > 120000)) gio::low(RELAY220);  // если разрешено отключать реле и прошло 2 минуты то отключить питание реле (откл. питание от сети 220В)
}
#endif

// функция расчета Ач и Втч
int32_t aw_ch(int32_t i) {
  return (i * 100 / 3600);
}

// функция включения блокировки модуля защиты в зависимости от напряжения акб
// включает блокировку при напряжении акб менее 5 Вольт и отключает при напр. более 6 Вольт
#if (GUARDA0)
void Guard(void) {
  if (ina.voltsec < 5000) gio::high(PINA0);
  else if (ina.voltsec > 6000) gio::low(PINA0);
}
#endif

// функция сброса значений заряда
void Res_mem_char(void) {
  pam.Ah_charge = 1;  // единица для того чтобы Ач и Вт сразу отображались на дисплее
  pam.Wh_charge = 1;
  pam.Time_charge = 0;
  pam.Time_addcharge = 0;
  pam.Ah_addcharge = 0;
  pam.Wh_addcharge = 0;
#if (BRANIMIR == 1)
  bitClear(pam.MyFlag, BRANIM);
  bitClear(pam.MyFlag, BRANIMBOIL);
#endif
}

// функция сброса значений разряда
void Res_mem_dischar(void) {
  pam.Ah_discharge = 1;    // емкость разряда Ah  // единица для того чтобы Ач и Вт сразу отображались на дисплее
  pam.Wh_discharge = 1;    // емкость разряда Вт/ч
  pam.Time_discharge = 0;  // время разряда
  pam.Ah_addcharge = 0;    // емкость разряда до напряжения аккумулятора
  pam.Wh_addcharge = 0;    // емкость до напряжения аккумулятора Вт/ч
}

// очистить память значений КТЦ
void Res_ktc(void) {
  EEPROM.put((MEM_KTC - 1), pam.Round);                         // сохранить количество циклов
  for (uint16_t i = MEM_KTC; i <= 1019; i++) EEPROM.put(i, 0);  // очистить память КТЦ
}

// функция ожидания (t - милисекунды)
void Delay(uint16_t t) {
  uint32_t r = millis();
  while (millis() - r < t) sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.  // and !butt.tick
}

void pauses(void) {
  do {
    sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.
  } while (!butt.tick);
}

// напряжение при котором начинается снижение тока заряда
uint16_t Volt_DCur(void) {
  return (pam.Volt_charge + bat.Volt(0)) >> 1;
}

// включение разряда и фиксация на определенном токе (amp - миллиампер)
void Fixcurrent(int16_t amp) {
  setCursory();
  PrintVA(0, amp, 0, 0, 2);
  float Integ = 0.0;
  uint32_t t = millis();
  ina.start(2);  // старт замеров INA
  do {
    // увеличение тока разряда
    butt.Tick();
    if (ina.sample()) {
      Integ += (amp - abs(ina.amperms)) * 0.001f;
      analogWrite_my(PWMDCH, trunc(constrain(Integ, 0, 255)));
    }
  } while (((millis() - t) < 3000) and !butt.tick);
}

// калибровка падения напряжения при токе нагрузки, напряжения от БП
void Korrect(const uint8_t kof) {
  if (kof and !VOLTIN) return;  // если выбрана калибровка напряжения от БП и не установлен делитель напряжения на входе то выйти из функции

#if (TIME_LIGHT)
  disp.Light_high();  // включение подсветки
#endif
#if (GUARDA0)
  Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
  lcd.clear();
  if (!kof) {
    Freq(vkr.service[FREQDISCHAR]);  // установить частоту работы разрядного модуля (4 кГц по умолчанию)
    lcd.print(F(txt11));
    Fixcurrent(2100);  // установить ток разряда 1000мА
    lcd.clear();
  } else {
#if (POWPIN == 1)
    gio::high(RELAY220);               // включить сеть 220В
    bitClear(flag_global, RELEY_OFF);  // запрещено отключать реле
#endif
  }
  bool x = false;
  bool ext = true;
  sekd.start();  // старт отсчета времени
  ina.start(2);  // старт замеров INA
#if (VOLTIN == 1)
  vin.start();  // старт замеров напряжения от БП
#endif
  do {
    // установка напряжения
    do {
      // ожидание действий пользователя
      sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.
      if (butt.tick or sekd.tick()) {
        // вывод на дисплей
        switch (kof) {
          case 0:
            lcd.clear();
            PrintVA(ina.voltsec, ina.ampersec, 0, 0, 3);
            setCursory();  // *12.365V 3.254А  *
            Write_x(x);    // > or  пробел  // *>80     >55    *
            lcd.print(vkr.Ohms);
            lcd.setCursor(8, 1);
            Write_x(!x);  // > or  пробел
            lcd.print(((float)vkr.shunt / 100));
            lcd.print(F("mOm"));
            break;
#if (VOLTIN == 1)
          case 1:  // замер напряжения блока питания
            setCursorx();
            PrintVA(vin.volt(), 0, 0, 0, 2);
            lcd.write(LEFTs);  // <
            lcd.print(vkr.Vref);
            lcd.write(RIGHTs);  // >
            break;
#endif
        }
      }
    } while (!butt.tick);

    switch (butt.tick) {
      case ENCCLICK:
      case OKCLICK:
        x = !x;
        break;
      case RIGHT:
      case LEFT:
        switch (kof) {
          case 0:
            if (x) vkr.Ohms = constrain(vkr.Ohms + butt.tick, 0, 255);
            else vkr.shunt = constrain(vkr.shunt + butt.tick, 10, 10000);  // 1 - 0,1 мОм, 10000 - 100мОм
            ina.start(1);                                                  // старт замеров INA
            break;
            // замер напряжения блока питания
#if (VOLTIN == 1)
          case 1:
            vkr.Vref = constrain(vkr.Vref + butt.tick, 1000, 6000);
            break;
#endif
        }
        break;
      case ENCHELD:
      case STOPCLICK:
        ext = false;
        break;
    }
  } while (ext);     // установка напряжения
  dcdc.Off();  // отключить заряд, разряд.
#if (POWPIN == 1)
  bitSet(flag_global, RELEY_OFF);  // разрешено отключать реле
#endif
}

// Функция ожидания time_charge - минут
void Wait(uint16_t time_charge) {
#if (POWPIN == 1)
  bitSet(flag_global, RELEY_OFF);  // разрешено отключать реле
#endif
  lcd.clear();
  lcd.print(F(txt16));  // Pause
  uint8_t tm = 60;
  sekd.start();  // старт отсчета времени одна секунда
  ina.start(1);  // старт замеров INA
  do {
    sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.

    // если прошла секунда вывод на дисплей
    if (sekd.tick()) {
      setCursory();
      PrintVA(ina.voltsec, 0, 0, 0, 2);
      lcd.print(time_charge);
      lcd.print(F(txt20));  // "минут"
      tm--;
      // когда прошла минута
      if (!tm) {
        tm = 60;
        time_charge--;
      }
    }
  } while (time_charge and butt.tick != STOPHELD and butt.tick != ENCHELD);  //  цикл в минутах или долгово нажатия OK
}

// Функция завершения заряда
void End(void) {
  lcd.clear();
  lcd.print(regim_end);  // вывод причины завершения работы.
  printSimb();           // пробел
  print_mode(0);         // вывод режима
  setCursory();
  int32_t Ah;
  uint32_t tm;
  switch (pam.Mode) {
    case 0:
      // Заряд
      Ah = pam.Ah_charge;
      tm = pam.Time_charge;
      break;
    case 1:
    case 2:
    case 5:
      // Заряд-Дозаряд , Бранимир, КТЦ
      Ah = pam.Ah_charge + pam.Ah_addcharge;
      tm = pam.Time_charge + pam.Time_addcharge;
      break;
    case 3:
      // Дозаряд
      Ah = pam.Ah_addcharge;
      tm = pam.Time_addcharge;
      break;
    case 4:
      // Разряд
      Ah = pam.Ah_discharge;
      tm = pam.Time_discharge;
      break;
    case 6:
    case 7:
      // Хранение, Буфер.
      Ah = 0;
      tm = 0;
      break;
  }
  PrintVA(0, 0, Ah, 0, 2);
#if (DISPLAYx == 16)
  Print_time(tm, DISPLAYx - 5, 1);
#elif (DISPLAYx == 20)
  Print_time(tm, DISPLAYx - 11, 1);
#endif
#if (DISPLAYy == 4)
  lcd.setCursor(0, 2);
  ClearStr(3);
#endif
  regim_end = 0;
  Speaker(1000);  // функция звукового сигнала (время в миллисекундах)
  Delay(1000);    // подождать
  pauses();       // пауза до нажатия
  lcd.clear();    // очистить дисплей
  Delay(500);     // подождать
}

// функция выбора Stop/OK. (timer - секунд, время через которое возвращается false. Если timer = 0 то возврат из функции по нажатию кнопок/енкодера)
bool Choice(uint8_t secund, bool rez) {
  sekd.start();
  bool ext = true;
  do {
    switch (butt.tick) {
      case LEFT:
      case RIGHT:
        rez = !rez;
        break;
      case ENCCLICK:
      case OKCLICK:
      case STOPCLICK:
        ext = false;
        break;
    }
    lcd.setCursor(4, 1);
    lcd.print((rez ? F(txt17) : F(txt19)));  // OK / Stop

    do {
      // цикл ожидания действий пользователя
      sensor_survey();  // опрос кнопок, INA226, напря. от БП., контроль dcdc.
      if (sekd.tick()) {
        // если прошла секунда вывод на дисплей
        lcd.setCursor(DISPLAYx - 2, 1);
        lcd.print(secund);
        printSimb();
        secund--;
      }
    } while (!butt.tick and secund and ext);  // цикл ожидания действий пользователя
  } while (ext and secund);                   // ожидание нажатия или истечения времени
  return rez;
}


// функция звукового сигнала (n - время в миллисекундах)
void Speaker(uint16_t n) {
// активный звукового сигнала
#if (SPEAKER == 2)
  gio::high(BUZER);  // включить сигнал
  Delay(n);          // подождать n - миллисекунд
  gio::low(BUZER);   // отключить сигнал

#elif (SPEAKER == 1)
  // пассивный звукового сигнала
  uint32_t time2 = millis();
  // звуковой сигнал в течении n миллисекунд
  while (millis() - time2 < n) {
    gio::high(BUZER);        // HIGH
    delayMicroseconds(771);  // 2314 - частота 432Гц; 1157 - частота 864Гц; 771 - частота 1296Гц
    gio::low(BUZER);         // LOW
    delayMicroseconds(771);
  }
#endif
  return;
}

// расчет времени заряда
void ChargeTimeCalc(void) {
  uint8_t v = (uint8_t)((uint32_t)pam.Capacity * 1000 / pam.Current_charge);
  pam.Time_charge_h = constrain(v + ((v + 2 - 1) >> 1), 1, 100);  // расчет времени заряда
}

// вычисление процента заряда (максим напряж заряда,текущее напряжение заряда, текущий ток заряда, минимальный ток заряда)
uint8_t Percentage(uint16_t volt_max, uint16_t volt, int16_t current, int16_t curr_min) {
  uint16_t vmin = bat.Volt(3);                       // миним напряжение (напряжение разряда)
  if (volt < vmin) return 0;                         // если текущее напряжение акб меньше напряжения разряда то вернуть 0%
  uint8_t vperc = map(volt, vmin, volt_max, 1, 50);  // процент заряда по напряжению 50%
  vperc += (uint8_t)(curr_min * 50 / current);       // прибавляем процент заряда по току 50%
  return constrain(vperc, 1, 100);
}

#if (SENSTEMP2 == 2)
// контроль температуры Акб с датчика NTC подключенного к пин А6 Ардуино
bool Control_trAkb() {
  bool tr = true;                                   // вернуть если температура акб в норме
  int16_t tr_akb = tr_bat.getTempInt();             // чтение температуры с датчика 2 акб
  int16_t curr_charge_init = dcdc.getCur_charge();  // ток заряда
  // если акб свинцово-кислотный то производится корректировка напряжения заряда 30 мВ на каждый градус Цельсия при отличии температуры от +25°С.
  if (pam.typeAkb <= 4) dcdc.setVolt_charge(dcdc.getVolt_charge() - (tr_akb - 25) * 30);  // Pb
  else if (pam.typeAkb <= 6 or pam.typeAkb == 8) {                                        // "Li-ion" "LiFePo4" "NiCd/Mh"
    // если температура акб меньше 10 или больше 40 гр то отключить заряд
    if (tr_akb < 10 or tr_akb > 40) {
      tr = false;
      dcdc.Off();
    } else if (tr_akb > 35) dcdc.setCur_charge(curr_charge_init - ((tr_akb - 35) * 10 * curr_charge_init / 100));                     // если темпратура акб больше 35 гр то уменьшать ток заряда на 10% на градус.
    else if (tr_akb < 15) dcdc.setCur_charge(curr_charge_init - ((5 - (tr_akb - 10)) * 10 * curr_charge_init / 100));                 // если темпратура акб меньше 15 гр то уменьшать ток заряда на 10% на градус.
    else tr = true;                                                                                                                   // t_akb = true;                                                                                                               // иначе продолжать заряд
  } else if (pam.typeAkb == 7 and tr_akb > 40) dcdc.setCur_charge(curr_charge_init - ((tr_akb - 40) * 10 * curr_charge_init / 100));  // "LiTit" // если темпратура акб больше 40 гр то уменьшать ток заряда на 10% на градус.
  return tr;
}
#endif

// функция отправки значений в сетевой порт
// время, напряжение, ток, А/ч, Вт/ч, темп Акб, темп Q1, напряжение БП
#if (LOGGER == 1)
void Serial_out(float volt, float amperSred, uint8_t tQ1, float volt_in, float Achas, uint8_t tAkb) {  //
  //const char* simb = ";";
  Serial.print(volt / 1000, 3);       //1 напряжение на АКБ (милливольт)/1000_________________
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(amperSred / 1000, 3);  //2 ток АКБ (миллиампер)/1000___________________________
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(tQ1, 1);               //3 Температура на Q1 (градусы)
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(volt_in / 1000, 3);    //4 напряжение от БП (милливольт)/1000__________________   vin
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(Achas / 100000, 3);    //5 Полученная_отданная емкость АКБ (А/ч)) /100000______
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(1);                    //6 Температура АКБ (градусы)
  //Serial.print(";");
  //Serial.print(7);      //7  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере. нужна коррекция скетча WiFi переходника
  //Serial.print(";");
  //Serial.print(8);      //8  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(9);      //9  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(10);    //10  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(11);    //11  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(12);    //12  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(13);    //13  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(14);    //14  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(15);    //15  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(16);    //16  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  //Serial.print(";");
  //Serial.print(17);    //17  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  Serial.println(";");  // Знак переноса строки.
}
#endif

// время, напряжение, ток
#if (LOGGER == 2)
void Serial_out(float volt, float amperSred, uint8_t tQ1, float Achas) {
  //const char* simb = ";";
  Serial.print(volt / 1000, 3);       //1 напряжение на АКБ (милливольт)/1000_________________
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(amperSred / 1000, 3);  //2 ток АКБ (миллиампер)/1000___________________________
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(tQ1, 1);               //3 Температура на Q1 (градусы)
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(1);                    //4 напряжение от БП (милливольт)/1000__________________
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(Achas / 100000, 3);    //5 Полученная_отданная емкость АКБ (А/ч)) /100000______
  Serial.print(";");                  // символ между значениями___________________________________
  Serial.print(1);                    //6 Температура АКБ (градусы)
                                      //Serial.print(";");
                                      //Serial.print(7);      //7  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере. нужна коррекция скетча WiFi переходника
                                      //Serial.print(";");
                                      //Serial.print(8);      //8  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(9);      //9  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(10);    //10  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(11);    //11  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(12);    //12  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(13);    //13  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(14);    //14  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(15);    //15  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(16);    //16  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
                                      //Serial.print(";");
                                      //Serial.print(17);    //17  подключен:значения на главной странице WiFi переходника и на логгере, отключен: значения только на логгере.
  Serial.println(";");                // Знак переноса строки.00
}
#endif

/*
// ===================== FLAGS Button======================
#define EB_PRESS (1 << 0)       1 // нажатие на кнопку
#define EB_HOLD (1 << 1)        2 // кнопка удержана
#define EB_STEP (1 << 2)        4 // импульсное удержание
#define EB_RELEASE (1 << 3)     8 // кнопка отпущена
#define EB_CLICK (1 << 4)       16 // одиночный клик
#define EB_CLICKS (1 << 5)      32 // сигнал о нескольких кликах
#define EB_TURN (1 << 6)        64 // поворот энкодера
#define EB_REL_HOLD (1 << 7)    128 // кнопка отпущена после удержания
#define EB_REL_HOLD_C (1 << 8)  256 // кнопка отпущена после удержания с предв. кликами
#define EB_REL_STEP (1 << 9)    512 // кнопка отпущена после степа
#define EB_REL_STEP_C (1 << 10) 1024 // кнопка отпущена после степа с предв. кликами

// ===================== FLAGS Encoder======================
#define EB_TYPE (1 << 0)
#define EB_REV (1 << 2)
#define EB_FAST (1 << 3)
#define EB_EHLD_M (1 << 4)
#define EB_DIR (1 << 5)
#define EB_ETRN_R (1 << 6)
#define EB_ISR_F (1 << 7)
*/
