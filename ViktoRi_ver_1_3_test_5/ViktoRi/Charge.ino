// флаги управления зарядом
#define CHARGEZ 0  // заряд включен
#define OSCILZ 1   // осциляция
#define TAKBZ 2    // температура акб
#define KORR 3     // корректировка напряжения и тока заряда
#define KORRVA 4   // изменение вольт или ампер
#define POWOFF 5   // питание вкл/откл
#define VMAXM 7    // напряжение достигло максимума

struct VA_var {
  uint16_t vmax[13] = { 0 };  // 13 значения напряжения. Сохраняются каждые 5 минут.
  int16_t amin[13] = { 0 };   // 13 значения тока

  void sred_v(void) {
    vmax[12] += (((int16_t)ina.voltsec - (int16_t)vmax[12]) >> 2);
  }
  void sred_a(void) {
    amin[12] += ((ina.ampersec - amin[12]) >> 2);
  }

  void sred_va(void) {
    sred_v();
    sred_a();
  }

  void offset(void) {
    // сохранить 12 значений напряжения и тока за 1 час
    // смещение значений на одно влево
    for (uint8_t n = 1; n < 12; n++) {
      vmax[n] = vmax[n + 1];
      amin[n] = amin[n + 1];
    }
  }
#if (GRAFIK == 1)
  void grafik(void) {
    // 1 - найти мин и макс
    int16_t vmin = vmax[1];
    int16_t vmks = vmax[1];
    int16_t amnm = amin[1];
    int16_t amks = amin[1];
    for (uint8_t n = 2; n <= 12; n++) {
      if (vmax[n] and (vmin > (int16_t)vmax[n])) vmin = (int16_t)vmax[n];  // миним напр
      if (vmax[n] and (vmks < (int16_t)vmax[n])) vmks = (int16_t)vmax[n];  // максим напр
      if (amin[n] and (amnm > amin[n])) amnm = amin[n];                    // миним ток
      if (amin[n] and (amks < amin[n])) amks = amin[n];                    // макс ток
    }
    // найти разницу
    int16_t vraznica = vmks - vmin;
    int16_t araznica = amks - amnm;
    // 2 - найти разрешение
    int16_t vrazr = (vraznica > 700) ? vraznica : 700;
    int16_t arazr = (araznica > 700) ? araznica : ((amks <= 200 and araznica <= 200) ? 400 : 700);

    // 3 - найти минимальное значение
    vmin -= ((vrazr - vraznica) >> 1);
    amnm -= ((arazr - araznica) >> 1);
    if (vmin < 0) vmin = 0;
    if (amnm < 0) amnm = 0;
    // 4 - найти максимальное значение
    vmks = (vmin) ? vmks + ((vrazr - vraznica) >> 1) : vrazr;
    amks = (amnm) ? amks + ((arazr - araznica) >> 1) : arazr;

    for (uint8_t n = 1; n <= 12; n++) {  // вывод на дисплей
      if (vmax[n]) {
        lcd.setCursor(n - 1, 0);
        lcd.write(map(vmax[n], vmin, vmks, 0, 6));
        lcd.setCursor(n - 1, 1);
        lcd.write(map(amin[n], amnm, amks, 0, 6));
      }
    }
    lcd.setCursor(12, 0);
    lcd.print(F(txt22));  // Volt
    lcd.setCursor(12, 1);
    lcd.print(F(txt23));  // Amper
  }
#endif
};

struct Tyme {
  uint32_t local;  // сюда сохраняем время заряда из памяти в сек (при запуске заряда)
  uint32_t real;   // сюда сохраняем начальный таймер заряда в мс (при запуске заряда)
  uint32_t real_tm; // сюда рассчитывается полное время заряда (каждую секунду)

  void cal_real_tm(void) {
    real_tm = local + (millis() - real) / 1000;  // рассчитать количество всех секунд заряда (каждую секунду)
  }
};

// Функция заряда. volt_charge_init - напряжение заряда, curr_charge_init - ток заряда, curr_min - минимальный ток заряда, time_charge - время заряда в часах, add_charge - дозаряд вкл/откл
void ChargeAkb(uint16_t volt_charge_init, int16_t curr_charge_init, int16_t curr_min, uint32_t time_charge, bool add_charge) {
  bitSet(flag_global, DCDCMODE);  // режим dcdc - Заряд
  //вывод режима, напряжения и тока заряда/разряда
#if (GUARDA0)
  Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
  // вывод значений заряда
  PrintVA(volt_charge_init, curr_charge_init, 0, 0, 1);               // вывод напряжения и тока заряда
  lcd.print(time_charge);  // вывод времени заряда/дозаряда
  lcd.write(104);                                                     // h
  lcd.setCursor(DISPLAYx - 2, 0);
  printCykl();  // вывод количества циклов

  Freq(vkr.service[FREQCHARGE]);  // установить частоту работы силового модуля
  time_charge *= 3600;            // перевел часы в секунды 

  VA_var va;
  Tyme tyme;

  tyme.local = add_charge ? pam.Time_addcharge : pam.Time_charge;

  const uint8_t VoltinKooff = bat.Volt(1) >> 8;                             // 14700/256 = 57 // pam.Voltage * 15; // количество банок * 15мВ;  // 4*15=60
  uint8_t typez = add_charge ? 3 : (bitRead(pam.MyFlag, PRECHAR) ? 1 : 2);  // 1 - Предзаряд, 2 - Заряд, 3 - Дозаряд (4 - осциляция(качели)), 5 - заряд по Бранимиру
#if (BRANIMIR == 1)
  if (pam.Mode == 2) typez = 5;  // 5 - заряд по Бранимиру
  uint32_t tyme_branim = 0;
#endif
  uint8_t ap[2] = { 4, 15 };  // коэффициент тока для предзаряда
  // задает напряжение и ток заряда
  dcdc.begin(volt_charge_init, (add_charge ? curr_charge_init : (bitRead(pam.MyFlag, PRECHAR) ? pam.Current_pre_charge : curr_charge_init)));

  uint8_t n_disp = 0, ns_disp = 0;  // номер 'экрана, время отображения
  uint16_t Timezar = 0;             // время до завершения заряда секунды
  uint16_t v1 = 0;                  // переменная для сохранения напряжения
  uint8_t tsw = 10;                 // Дозаряд переменная времени переключения, сек.
  uint8_t timeaut10 = 2;            // время 10 минут
  uint16_t timeaut5 = 300;          // время 5 минут
  uint8_t time_millis = 0;
  uint16_t volt_init;
  const int16_t cur_max = curr_charge_init;

#if (POWPIN == 1)
  gio::high(RELAY220);  // включить сеть 220В
  bitClear(flag_global, RELEY_OFF);    // запрещено отключать реле
#endif
#if (LOGGER > 0)
  uint8_t logg = LOGGTIME;  // период отправки данных в сетевой порт
#endif
  const int x[] = { (int)(&settings[14]), (int)(&modeTxt[0]), (int)(&modeTxt[3]), (int)(&settings[17]), (int)(&modeTxt[2]) };  // {предварительный заряд, заряд, дозаряд, Качели, Бранимир}
  uint8_t flagsz = 0b00110111;
  // #define  CHARGEZ  0    // 1 заряд включен
  // #define  OSCILZ   1    // 1 осциляция
  // #define  TAKBZ    2    // 1 температура акб
  // #define  KORR     3    // 0 корректировка напряжения и тока заряда
  // #define  KORRVA   4    // 1 изменение вольт или ампер
  // #define  POWOFF   5    // 1 питание вкл/откл
  // #define  VMAXM    7    // напряжение достигло максимума
  uint8_t q1 = 0;  // количество проверок силового транзистора 2
  Delay(3000);
  lcd.clear();
#if (VOLTIN == 1)
  uint16_t vin_volt;
  vin.start();  // старт замеров напряжения от БП
#endif
  sekd.start();  // старт отсчета времени одна секунда
  ina.start(1);  // старт замеров INA
  dcdc.start();
  tyme.real = millis(); // запоминаем текущее время заряда
  // цикл заряда
  while (bitRead(flagsz, CHARGEZ) and bitRead(pam.MyFlag, CHARGE)) {
    sensor_survey();  // опрос кнопок, INA226, напряж. от БП., контроль dcdc.

    if (butt.tick) {
      // если кнопка нажата
      if (bitRead(flagsz, KORR)) {
        // производится корректировка напряжения и тока заряда        
        switch (butt.tick) {
          case RIGHT:  // энкодер вправо
          case LEFT:   // энкодер влево
            // изменение напряжения или тока
            if (bitRead(flagsz, KORRVA)) volt_charge_init = constrain(volt_charge_init + butt.tick * 100, 1000, POWERINT);
            else curr_charge_init = constrain(curr_charge_init + butt.tick * 100, CUR_CHARGE_MIN, CURRMAXINT);
            dcdc.begin(volt_charge_init, curr_charge_init);
            break;
          case ENCCLICK:  // клик энкодер
          case OKCLICK:   // нажать Пуск
            InvBit(flagsz, KORRVA);
            break;
          case ENCHELD:              // удержать энкодер
          case STOPCLICK:            // нажать Стоп
            bitClear(flagsz, KORR);  //korr = false;  // выйти из изменения
            setCursory();
            ClearStr(15);  // очистить вторую строку
            break;
        }
      } else {        
        switch (butt.tick) {
          case STOPHELD:
          case ENCHELD:            
            if (n_disp == 0) bitClear(pam.MyFlag, CHARGE); // завершить заряд
            if (butt.tick == STOPHELD) break;
          case OKHELD:
            if (n_disp == WINDC) bitSet(flagsz, KORR);
            break;            
          case RIGHT:
          case LEFT:
            if (bitRead(flag_global, POWERON)) {              
              // если питание от БП приходит то выбрать следующее окно
              lcd.clear();
              n_disp = (n_disp == 0 and butt.tick < 0) ? WINDC : n_disp + butt.tick;  // окно отображения
              ns_disp = TIME_DISP;                                    // время отображения сек
            }
            break;
        }
      }
    }

    (bitRead(flagsz, OSCILZ) and bitRead(flagsz, TAKBZ)) ? bitSet(flag_global, DCDC_PAUSE) : bitClear(flag_global, DCDC_PAUSE);

    switch (time_millis) {
      case 0:
        if (sekd.tick()) {
          // если прошла секунда
          time_millis = 13;                                                                                // раз в секунду запускаем 13 шагов
          volt_init = (ina.voltsec >= (volt_charge_init - VoltinKooff)) ? volt_charge_init : ina.voltsec;  // стабилизация максимального напряжения
#if (VOLTIN == 1)
          vin_volt = vin.volt();
#endif
        }
        break;
      case 13:
#if (GUARDA0)
        Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
        break;
      case 12:
        // прибавить секунду ко времени, если время больше установленного завершить работу
        tyme.cal_real_tm(); // рассчитать количество всех секунд заряда 
        if (tyme.real_tm > time_charge) {
          bitClear(flagsz, CHARGEZ);
          regim_end = 1;  // закончилось время
        }
        if (add_charge) {
          pam.Time_addcharge = tyme.real_tm;
          pam.Ah_addcharge += aw_ch((int32_t)ina.ampersec);  // расчет ампер/часов
          pam.Wh_addcharge += aw_ch(ina.getPower());         // расчет ватт/часов
        } else {
          pam.Time_charge = tyme.real_tm;
          pam.Ah_charge += aw_ch((int32_t)ina.ampersec);  // расчет ампер/часов
          pam.Wh_charge += aw_ch(ina.getPower());         // расчет ватт/часов
        }
        
        // выполнить если включен таймер завершения заряда
        if (Timezar) {         
          if (--Timezar == 0) bitClear(flagsz, CHARGEZ);  // если таймер вышел то завершить заряд
        }
        break;
      case 11:
        switch (typez) {
            // выбор типа заряда
          case 1:
            //  включен предварительный заряд
            dcdc.setCur_charge(map(timeaut5, 300, 1, constrain((int16_t)pam.Capacity * ap[0], 100, curr_charge_init), constrain((int16_t)pam.Capacity * ap[1], 300, curr_charge_init)));  //  в течении 300сек увеличиваем ток заряда от ap[0]С до ap[1]С
            if (timeaut5 > 250 and (va.vmax[12] < volt_init)) va.sred_va();
            break;
          case 2:
            // включен заряд
            va.sred_va();  // скользящее среднее - колебания напряжения и тока уменьшены в 4 раза
            // начать снижение тока заряда при достижении величины напряжения половины максимального
            // начинает работать когда напряжение достигнет установленного и ток заряда более 1% от емкости
            if (va.vmax[12] > pam.Volt_decrCur and va.amin[12] > (int16_t)pam.Capacity * 10) {
              curr_charge_init = map(va.vmax[12], pam.Volt_decrCur, volt_charge_init, cur_max, (int16_t)pam.Capacity * 10);
              dcdc.begin(volt_charge_init, curr_charge_init);
            }
            break;
          case 3:
            // Дозаряд
            if (dcdc.getCur_charge() == curr_charge_init) va.sred_va();  // скользящее среднее - колебания напряжения и тока уменьшены в 4 раза
            if (pam.Current_add_charge_min < pam.Current_add_charge) {
              if (tsw) tsw--;
              if (!tsw and (abs((int16_t)volt_init - (int16_t)v1) < 20)) {
                // если напряжение стабилизировалось, время завершилось
                dcdc.setCur_charge((dcdc.getCur_charge() == curr_charge_init) ? pam.Current_add_charge_min : curr_charge_init);  // если ток максимальный
                tsw = 10;                                                                                                        // ожидание 10 сек
              } else v1 = volt_init;
            }
            break;
          case 4:
            // если в настройках включена Осциляция(Качели) то при снижении тока менее curr_min*2 включается Режим осциляции - включение и отключение тока
            if (abs((int16_t)volt_init - (int16_t)v1) < 10) {
              // если ток включен
              if (bitRead(flagsz, OSCILZ)) {
                // скользящее среднее - колебания напряжения и тока уменьшены в 4 раза
                if (va.vmax[12] < volt_init) va.sred_v();                     // сохраняем максимальное напряжение vmax[12] = volt_init
                if (va.amin[12] > ina.ampersec or !va.amin[12]) va.sred_a();  // сохраняем минимальный ток amin[12] = ina.ampersec
              }
              InvBit(flagsz, OSCILZ);  // инвертируем флаг
#if (MCP4725DAC)
              if (BitIsClear(flagsz, OSCILZ)) dac.setVoltage(0);
#else
              if (BitIsClear(flagsz, OSCILZ)) dcdc.Off();  // отключить заряд, разряд.
              else bitSet(flag_global, DCDC_PAUSE);
#endif
            } else v1 = volt_init;
            break;
          case 5:
            va.sred_va();  // скользящее среднее - колебания напряжения и тока уменьшены в 4 раза
            break;
        }
        break;
      case 10:
        // вывод на дисплей 1602
#if (DISPLAYy == 2)
        switch (n_disp) {          
          case 0:
            // экран 0
            Display_print((add_charge ? pam.Ah_addcharge : pam.Ah_charge), tyme.real_tm, Percentage(volt_charge_init, va.vmax[12], va.amin[12], curr_min), add_charge);  // вывод текущих значений напряжения, тока. Вывод Ватт-часов.
            break;
          case 1:
            // экран 1
            setCursorx();
            print_mode(0);
            setCursory();
            disp.printFromPGM(x[typez - 1]);
            printSimb();                                                                  // пробел
            lcd.print(Percentage(volt_charge_init, va.vmax[12], va.amin[12], curr_min));  // процент заряда Акб
            lcd.print(F(txt_PRC));
            //lcd.print(dcdc.getTok());  // вывод значения ШИМ заряда
            break;
            // *Charge>AddChar  *
            // *Charge 45% 125  *
          case 2:
            // экран 2
            setCursorx();
            PrintVA(0, 0, 0, (add_charge ? pam.Wh_addcharge : pam.Wh_charge), 2);
            print_tr(kul.tQ1);  // температура
            lcd.setCursor(DISPLAYx - 1, 0);
            lcd.print(regim_end);  // причина завершения заряда
#if (SENSTEMP2 == 2)
            print_tr(tr_bat.getTempInt());  // температура
#endif
            setCursory();
#if (VOLTIN == 1)
            PrintVA(vin_volt, 0, 0, 0, 1);
#endif
            printCykl();
            printSimb();
            lcd.print(dcdc.getTok());  // вывод значения ШИМ заряда
            if (Timezar) {
              printSimb();
              lcd.print(Timezar);  // выполнить если включен таймер завершения заряда
            }
            break;
            // *58.32Wh 45C 25C *
            // *24.1V C1 N1     *
          case 3:
            // экран 3
#if (GRAFIK == 1)
            if (ns_disp == TIME_DISP) va.grafik();
#else
            setCursorx();
            lcd.print((float)(ina.getPower() / 1000), 2);
            lcd.print(F("W"));
#endif
            break;
          case 4:
            // экран 4           // изменение напряжения и тока заряда
            setCursorx();
            PrintVA(ina.voltsec, ina.ampersec, 0, 0, 2);  // текущие напряжение и ток
            lcd.setCursor(DISPLAYx - 2, 0);
            lcd.print(F(txt_OK));
            if (bitRead(flagsz, KORR)) {
              setCursory();
              Write_x(bitRead(flagsz, KORRVA));       // > or  пробел
              PrintVA(volt_charge_init, 0, 0, 0, 1);  // Напряжение и ток заряда
              Write_x(!bitRead(flagsz, KORRVA));      // > or  пробел
              PrintVA(0, curr_charge_init, 0, 0, 1);  // Напряжение и ток заряда
            }
            break;
          default:
            n_disp = 0;
            lcd.clear();  // очистить дисплей
            break;
        }
#endif
// вывод на дисплей 2004
#if (DISPLAYy == 4)
        switch (n_disp) {
          // экран 0
          case 0:
            Display_print((add_charge ? pam.Ah_addcharge : pam.Ah_charge), (add_charge ? pam.Wh_addcharge : pam.Wh_charge), tyme.real_tm, kul.tQ1, Percentage(volt_charge_init, va.vmax[12], va.amin[12], curr_min));
            break;
          case 1:
            setCursorx();
            print_mode(0);
            setCursory();
            disp.printFromPGM(x[typez - 1]);
            lcd.setCursor(0, 2);
            lcd.print(regim_end);  // причина завершения заряда
            if (Timezar) {
              printSimb();
              lcd.print(Timezar);  // выполнить если включен таймер завершения заряда
            }
            printCykl();
            break;
          // изменение напряжения и тока заряда
          case 2:
            setCursorx();
            PrintVA(ina.voltsec, ina.ampersec, 0, 0, 2);  // текущие напряжение и ток
            lcd.setCursor(DISPLAYx - 2, 0);
            lcd.print(F(txt_OK));
            if (bitRead(flagsz, KORR)) {
              setCursory();
              Write_x(bitRead(flagsz, KORRVA));
              PrintVA(volt_charge_init, 0, 0, 0, 1);  // Напряжение и ток заряда
              Write_x(!bitRead(flagsz, KORRVA));
              PrintVA(0, curr_charge_init, 0, 0, 1);  // Напряжение и ток заряда
            }
            break;
#if (GRAFIK == 1)
          case 3:
            // вывод графика
            if (ns_disp == TIME_DISP) va.grafik();
            break;
#endif
          default:
            n_disp = 0;
            break;
        }
#endif
        if (BitIsClear(flagsz, KORR) and n_disp) {
          if (ns_disp) ns_disp--;
          else {
            n_disp = 0;
            lcd.clear();
          }
        }
        break;
      case 9:
        if (volt_init > va.vmax[0]) va.vmax[0] = volt_init;        // сохранить в vmax[0] максимальное напряжение
        if (ina.ampersec > va.amin[0]) va.amin[0] = ina.ampersec;  // сохранить вmeter.voltn[0] максимальный ток
        // если флаг макс напр не установлен и напряжение достигло максимального то установить этот флаг
        if (BitIsClear(flagsz, VMAXM) and (va.vmax[0] >= volt_charge_init - 50)) bitSet(flagsz, VMAXM);
        // иначе если флаг макс напр установлени и напряжение стало ниже максимального то снять этот флаг
        else if (bitRead(flagsz, VMAXM) and (volt_init < volt_charge_init - 500)) {
          bitClear(flagsz, VMAXM);
          va.vmax[0] = 0;
          Timezar = 0;
        }
        break;
      case 8:
        // выполнить каждые 5 минут
        if (--timeaut5 == 0) {
          timeaut5 = 300;
          // сохранить 12 значений напряжения и тока за 1 час
          va.offset();  // смещение значений на одно влево

          switch (typez) {            
            case 1:
              // включен предварительный заряд
              ap[1] += 3;  // каждые 5 минут увеличиваем ток заряда.
              ap[0] = ap[1] / 3;
              // если напряжение больше напряжения предзаряда то отключить предварительный заряд
              if (va.vmax[12] > pam.Volt_pre_charge) {
                typez++;  // отключить предварительный заряд - вкл. обычный заряд
                dcdc.setCur_charge(curr_charge_init);
              }
              break;
            case 2:
              // включен заряд
              // включение режима Осчиляция(Качели) при снижении тока заряда до curr_min*3, не выполняется заряд по Бранимиру
              if (bitRead(pam.MyFlag, OSCIL) and (va.amin[12] < constrain(curr_min << 1, curr_min, (int16_t)pam.Capacity * 7))) typez = 4;
            case 3:
              // включен дозаряд,
              //если напряжение достигло максимального
              if (bitRead(flagsz, VMAXM)) {
                // нужно следить за током
                // если он не уменьшается то включить таймер на 1 час

                // если прошло время
                uint8_t ag = 1;
                //ищем заполненую ячейку
                while (ag < 11 and va.amin[ag] == 0) ag++;
                // чем ближе ток к минимальному тем больше промежуток времени
                uint8_t an = map(va.amin[12], curr_charge_init, curr_min, 11, ag);
                uint8_t cm = map(va.amin[12], curr_charge_init, curr_min, 50, 10);

                if (va.amin[12] <= curr_min) {
                  // если ток снизился менее тока отключения то завершить заряд, дозаряд
                  if (regim_end != 2 or !Timezar) Timezar = 600;  // включаем таймер 10 мин
                  // если ток продолжает уменьшаться то
                  if ((va.amin[11] - va.amin[12]) > cm) Timezar = 600;  // продлеваем время
                  regim_end = 2;                                        //  напряжение достигло максимума а ток минимума
                  //bitClear(flagsz, CHARGEZ);
                } else {
                  if (va.amin[an] - va.amin[12] < cm) {
                    regim_end = 5;  // напряжение достигло максимума, ток перестал уменьшаться
                    if (!Timezar) Timezar = 36000;
                  } else if (Timezar) Timezar = 36000;

                  if ((va.amin[an] - va.amin[12]) < -cm) {
                    regim_end = 4;  // напряжение достигло максимума, ток начал увеличиваться
                    if (!Timezar) Timezar = 3600;
                  } else if (regim_end == 4 and Timezar) {
                    Timezar = 0;
                    regim_end = 0;
                  }
                }
              }
              break;
            case 4:
              // включен режим Качели
              // если минимальный ток меньше тока завершения заряда и напряжение достигло максимального то уменьшить ток завершения заряда
              if ((va.amin[12] < curr_min) and bitRead(flagsz, VMAXM)) {
                // если ток уменьшился на 30мА
                if (curr_min - va.amin[12] > 30) {
                  curr_min = va.amin[12];  // уменьшить ток завершения заряда
                  Timezar = 1200;          // и засечь время
                  regim_end = 6;
                }
                // если время завершения заряда не включено то засечь время
                if (!Timezar) Timezar = 1200;  // и засечь время
              }
              break;
            case 5:
              // заряд по Бранимиру
#if (BRANIMIR == 1)
              // если напряжение достигло максимального начинаем отсчет времени
              if (bitRead(flagsz, VMAXM)) {
                tyme_branim += 300;
                if (tyme_branim >= 72000) {
                  bitClear(flagsz, CHARGEZ);       // если прошло более 20 часов останавливаем заряд и перемешиваем
                  bitSet(pam.MyFlag, BRANIMBOIL);  // установить флаг перемешивания электролита
                }
                if (va.amin[12] <= curr_min) {
                  // если ток заряда снизился до установленного то устанавливаем флаг об успешном заряде Бранимира и переходим в дозаряд
                  regim_end = 2;               //  напряжение достигло максимума ток минимума;
                  bitSet(pam.MyFlag, BRANIM);  // установить флаг успешного завершения заряда по Бранимиру
                  bitClear(flagsz, CHARGEZ);   // отключить заряд
                }
              }
#endif
              break;
          }
          // выполнить раз в 10 минут.
          if (--timeaut10 == 0) {
            timeaut10 = 2;  //timeaut10 = 2;
            Saved();        // сохранить настройки
          }
          // выполнить раз в 10 минут.
        }
        // выполнить раз в 5 минуты.
        break;
      case 7:
        // защита от повышенного тока и напряжения
        if (ina.ampersec > (int16_t)(curr_charge_init * 1.2f) or (ina.voltsec > (uint16_t)(volt_charge_init * 1.03f) and ina.ampersec > 20)) {
#if (TIME_LIGHT)
          disp.Light_high();  // включение подсветки
#endif
          if (dcdc.getTok()) {
            dcdc.Off();
            Speaker(200);  // функция звукового сигнала (время в миллисекундах)
          } else {
            Speaker(1000);  // функция звукового сигнала (время в миллисекундах)
            resetFunc();    // если ток  или напряжение заряда превысит установленный  значит не работает блок регулировки и защиты и вызываем reset (перезагрузка Ардуино). После презагрузки работа продолжится.
          }
        }
#if (TIME_LIGHT)
        disp.Light_low();  // отключение подсветки через 3 минуты, если разрешено отключение
#endif
        break;
      case 6:
        if (BitIsClear(flag_global, POWERON)) {
          // если питание от БП не приходит
#if (TIME_LIGHT)
          disp.Light_high();  // включение подсветки
#endif
          lcd.clear();
          lcd.print(F(txt4));  // "Power "
          print_mode(2);       // "error"
          setCursory();
          PrintVA(ina.voltsec, ina.ampersec, 0, 0, 2);
          //Speaker(100);  // функция звукового сигнала (время в миллисекундах)
        }
        break;
      case 5:
        //проверка на то что силовой транзистор пробит
        q1 = (dcdc.getTok() == 0 and ina.ampersec > 100) ? q1 + 1 : 0;
        if (q1 > 2) bitWrite(pam.MyFlag, CHARGE, Q1_broken());  // если количество проверок превысило 2 то запустить функцию
        break;
        //     case 4:

        //        break;
        //     case 3:
        // резерв
        //       break;
      case 2:
#if (SENSTEMP2 == 2)
        bitWrite(flagsz, TAKBZ, Control_trAkb());  // контроль температуры Акб с датчика NTC подключенного к пин А6 Ардуино
#endif
        break;
      case 1:
#if (LOGGER == 1)
        logg--;
        if (!logg) {
          logg = LOGGTIME;
          Serial_out(ina.voltsec, ina.ampersec, kul.tQ1, vin_volt, (add_charge ? pam.Ah_addcharge : pam.Ah_charge), 0);  //------------------------------------//------------------------------------------//Serial_out(tyme.real_tm, ina.voltsec, ina.ampersec, (add_charge ? pam.Ah_addcharge : pam.Ah_charge), (add_charge ? pam.Wh_addcharge : pam.Wh_charge), 0, kul.tQ1, vin_volt);  // функция отправки значений в сетевой порт
        }
#elif (LOGGER == 2)
        logg--;
        if (!logg) {
          logg = LOGGTIME;
          Serial_out(ina.voltsec, ina.ampersec, kul.tQ1, (add_charge ? pam.Ah_addcharge : pam.Ah_charge));  //-------------------------------//--------------------------------------//Serial_out(tyme.real_tm, ina.voltsec, ina.ampersec);  // функция отправки значений в сетевой порт
        }
#endif
        break;
    }
    if (time_millis) {
      time_millis--;
      if (BitIsClear(flag_global, POWERON) and time_millis > 6) time_millis = 6;
    }
  }
  // цикл заряда
  dcdc.Off();  // отключить заряд, разряд.
#if (GUARDA0)
  gio::low(PINA0);
#endif
  Saved();  // сохранить настройки
}
