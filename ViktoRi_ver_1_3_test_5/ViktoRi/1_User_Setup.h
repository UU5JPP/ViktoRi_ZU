#define MEMVER 102   // версия прошивки в EPPROM 
#if defined(__LGT8FX8P__)
#define VER "Ver 1.3-lgt8"  // версия прошивки и тип контроллера
#else
#define VER "Ver 1.3-328p"  // версия прошивки и тип контроллера
#endif
#define txt_ViktoRi "ViktoRi"


#define INTERFACE 1  // выбор языка интерфейса: 0 - русский(!для дисплеев с поддержкой русского языка!), 1 - английский, 2 - транслит
// параметры дисплея
#define ADDRDISP 0x27  // адрес дисплея 0x27 или 0x3F
#define DISPLAYx 16    // количество столбцов дисплея 16 или 20
#define DISPLAYy 2     // количество строк дисплея     2 или 4
// параметры управления
#define ENCBUTT 2  // 0 - только энкодер, 1 - только кнопки, 2 - энкодер + Пуск + Stop, 3 - энкодер + все кнопки
// тип спикера
#define SPEAKER 1  // 1 - пассивный спикер, 2 - активный спикер
// выбор програмных модулей
#define VOLTIN   1  // замер напряжения от БП:  1 - используется, 0 - не используется
#define FAN      1  // управление вентилятором: 0 - не используется, 1 - вентилятор c плавной регулировкой оборотов ШИМ, 2 - режим работы вентилятора вкл/откл
#define PROT     1  // управления модулем защиты: 1 - используется, 0 - не используется
#define DISCHAR  1  // разряд: 1 - используется разрядный модуль, 0 - не используется.
#define RESIST   1  // замер сопротивления: 1 - используется, 0 - не используется
#define BRANIMIR 1  // методика заряда Акб по инструкции Бранимира: 1 - используется, 0 - не используется
// реле 220В или светодиод БП для пин 10
#define POWPIN 0   // 0 - не используется, 1 - реле 220 Вольт, 2 - индикатор включения БП в сеть(не работает без #define VOLTIN 1)

// >>> при изменении остальных параметров нужно сделать "Сброс системных настроек", что бы данные параметры применились и сохранились в память.
// сопротивление шунта
#define SHUNT 0.01085                       // сопротивление шунта, Ом
#define CURR_MAX (0.08192f / (float)SHUNT)  // Максимальный ток зарядника, 25.5 ампер. расчитывается автом.(8.1A при шунте 0,01 Ом). Можно выставить вручную. Максимальный ток зависит от сопротивления шунта (0.08192 / SHUNT) 0.08192V / 0.01 = 8.192А (Макс напр на шунте / сопрот шунта)
//#define CURR_MAX 8.0                       // Максимальный ток ЗУ 8.0 Ампер. При необходимоси раскомментируй строку, впиши максимальный ток вручную. Строку выше закомментируй.

#define OHMS 61  // сопротивление линии до акб - миллиОм,  0-255. Установи 0 если хочешь чтобы INA замеряла напряжение без коррекции по току.
// Чтобы значения применились сбрось калибровки.

// параметры для функциии замера напряжения от БП. Опорное напряжение и значения сопротивлений делителя напряжения.
#define VREF 5000     // точное напряжение на пине 5V (в мВ). Замерь напряжение и впиши сюда значение. Влияет на замер напряжения от БП. 4292  4304
#define DIV_R1 10000  // точное значение верхнего резистора Ом. до 65kOm. До 35В от БП рекомендуется на 6000 Ом. до 55В - 10000 Ом.
#define DIV_R2 1000   // точное значение нижнего резистора Ом.

// Выбор датчика температуры 1 для контроля температуры силового транзистора.
#define SENSTEMP1 1  // датчик температуры транзистора Q1: 1 - DS18B20, 2 - NTC, 0 - не используется.  Схема 1.6.3
// параметры для  датчика температуры NTC
#define NTCB1 3435    // B термистора - берется из характеристик термистора
#define NTCR1 10000   // Ом - сопротивление термистора при 25 градусах.
#define NTCRS1 10000  // Ом - сопротивление подтягивающего резистора (тем точнее тем лучше)
#define TEMPBASE1 25  // базовая температура термистора. Подкорректируй если замеряет не правильно.

// датчик температуры 2 для контроля температуры аккумулятора.
#define SENSTEMP2 0  // датчик температуры аккумулятора: (1 - DS18B20 пока нет), 2 - NTC, 0 - не используется.  Схема 1.6.3
// параметры для  датчика температуры NTC
#define NTCB2 3435    // B термистора - берется из характеристик термистора
#define NTCR2 10000   // Ом - сопротивление термистора при 25 градусах.
#define NTCRS2 10063  // Ом - сопротивление подтягивающего резистора (тем точнее тем лучше)
#define TEMPBASE2 57  // базовая температура термистора. Подкорректируй если замеряет не правильно.

#define CORRECTCUR 1  // 1 - вкл, 0 - откл. работает совместно с SENSTEMP2. (в доработке!) корректировать напряжение заряда в зависимости от температуры акб.
                      // пока что контролирует минимальную и максимальную температуру акб. 

// параметры управления вентилятором
#define DEG_ON 40        // температура включения вентилятора в градусах
#define DEG_OFF 36       // температура отключения вентилятора в градусах
#define DEG_MAX 60       // температура максимальных оборотов вентилятора
#define DEG_CUR 70       // температура при которой уменьшается ток заряда
#define CURFAN_ON 1500   // значение тока в миллиАмперах при котором включается вентилятор. (до 32А)
#define CURFAN_OFF 1200  // значение тока в миллиАмперах при котором отключается вентилятор. (до 32А)
#define CURFAN_MAX 7000  // значение тока в миллиАмперах  для максимальных оборотов вентилятора. (до 32А)

// значение напряжения БП
#define POWER 24      // 24 - напряжение от блока питания, вольт(целое число). Если установлен делитель напряжения на входе то замеряется в программе. (Цифровое ограничение 65 Вольт)
#define POWER_MAX 26  // максимально допустимое напряжение от БП (17-32 Вольт). Если оно превысит то отключится  реле 220В (пин 10) на 10 секунд (если был просто скачек напряжения). Утитывай напряжение С12 и С10 и стабилизатора LM7812.
// время дисплея
#define TIME_LIGHT 3  // время подсветки дисплея в минутах (от 0 до 255 минут). При 0 подсветка отключаться не будет. 
#define TIME_DISP 10  // время возврата на 1 экран в секундах
// профили
#define PROFIL 9  // количество профилей пользователя 0 - 9.


// считывание температуры с датчика SENSTEMP1 при разряде и управление кулером
#define KULDISCHAR 1  // 0 - откл. 1 - управление кулером по датчику темпрературы SENSTEMP1. 2 - принудительное включение вентилятора при разряде

//график напряжения и тока при заряде за последний час. В LGT8 не влезает.
#if defined(__LGT8FX8P__)
#define GRAFIK 0  // 1 - вкл, 0 - откл.  Данные фиксируются каждые 5 минут. Это для LGT8
#else
#define GRAFIK 1  // 1 - вкл, 0 - откл.  Данные фиксируются каждые 5 минут. Это для 328р
#endif

// изменение сервисных параметров
#define SERVICE 1  // 1 - вкл, 0 - откл.
// уменьшение тока заряда при превышении температуры силового транзистора.
#define GUARDTEMP 1  // 1 - вкл, 0 - откл.
// принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт. Включение пин А0.
#define GUARDA0 0  // 1 - вкл, 0 - откл.

// ЦАП MCP4725
#define MCP4725DAC 0   // 1 - вывод значений заряда в ЦАП MCP4725, 0 - отключен
#define ADDR4725 0x60  // адрес MCP4725
// Логгер. Отправка в сетевой порт. ПРИ ВКЛЮЧЕНИИ ОТКЛЮЧИ -  GRAFIK !!!
#define LOGGER 0    // функция отправки значений в сетевой порт (скорость 115200).  Занимает много памяти. Если хочешь ее использовать отключи другие функции. 
                    //  1 - набор логов: напряжение, ток, А/ч, (напряжение БП), (темп Q1), (темп Акб) - в скобках вывод при наличии датчиков
                    //  0 - не используется.
#define LOGGTIME 1  // период, в секундах, 1 - 255. С этим периодом будут отправляться данные в сетевой порт.

// настройка пинов на свое усмотрение. Название оставляй номер пина меняй.
#define PUSK      2  // кнопка - Пуск
#define PWMCH     3  // ШИМ выход заряда          (3 выход ШИМ)
#define MINUS     4  // кнопка - Минус
#define PLUS      5  // кнопка - Плюс           //(5 выход ШИМ)
#define STOP      6  // кнопка - Стоп           //(6 выход ШИМ)
#define ENC_S1    7  // энкодер S1
#define ENC_S2    8  // энкодер S2
#define PWMKUL    9  // ШИМ - вентилятор (кулер)  (9 выход ШИМ)
#define RELAY220 10  // управление реле 220В    //(10 выход ШИМ)
#define PWMDCH   11  // ШИМ нагрузка. Разряд.     (11 выход ШИМ)
#define ENC_KL   12  // энкодер klick
#define PIN_13   13  // управление реле откючение нагрузки от акб в буферном режиме
#define PINA0    A0  // управление блокировкой модуля защиты
#define BUZER    A1  // выхорд на динамик(пищалка). При вкл. генерируются импульсы с частотой около 1200Гц. Можно использовать пищалку от компьютера подключив напрямую к пину и GND.
#define PROTECT  A2  // управление защитным транзистором Q4
#define PINTERM1 A3  // вход датчика температуры DS18B20 или NTC
#define PINTERM2 A6  // вход датчика температуры акб NTC 
#define POWIN    A7  // замер напряжения БП
//               A4       // SDA
//               A5       // SCL

// ------- конец настроек --------

// ****можно изменить при желании***
// минимальный ток завершения заряда
#define CURMIN 20
// минимальный ток заряда, мА
#define CUR_CHARGE_MIN 100
// шаг изменения напряжения в настройках, мВ
#define STEP_VOLT 100
// шаг изменения тока в настройках, мА
#define STEP_CURR 100
// Частота шины I2C
#define WIRECLOCK 800000

/* Цифровые контакты:
   D2 - кнопка1 ОК/Старт
   D3 - ШИМ выход заряда
   D4 - кнопка  минус-
   D5 - кнопка плюс+
   D6 - Стоп/Назад
   D7 - энкодер S1
   D8 - энкодер S2
   D9 - ШИМ - вентилятор (кулер)
   D10 - управление реле 220В либо светодиод отображающий наличие питания
   D11 - ШИМ нагрузка. Разряд.
   D12 - энкодер klick
   D13 - управление реле откючение нагрузки от акб в буферном режиме

   Аналоговые контакты:
   A0 - управление блокировкой модуля защиты
   A1 - выход звук
   A2 - выход управления защитой
   A3 - вход датчика температуры силового транзистора
   A4 - SDA
   A5 - SDL
   A6 - вход датчика температуры NTC аккумулятора
   А7 - замер напряжения на входе зарядника
*/

/*
  const char* TypeAkb[] = {
  "Pb Ca/Ca",      //  0        2,1В  напр. отсечки при заряде - 2,45В  напр. отсечки при разряде - 1,75В
  "Pb Ca+",        //  1
  "Pb Sur",        //  2
  "Pb AGM",        //  3
  "Pb Gel",        //  4
  "Li-ion",        //  5  литий-ионные           3,65В  напр. отсечки при заряде - 4,1  напр. отсечки при разряде - 2,5-3В(2,75В)  ток заряда - 1С  напр хранения- 3,75
  "LiFePo4",       //  6  литий-железо-фосфатные 3,2В   напр. отсечки при заряде - 3,3  напр. отсечки при разряде - 2,8В(2В)  напр хранения- 3,3 ток заряда - <4С
  "LiTit",         //  7  литий-титанатные       2,4В
  "NiCd/Mh",       //  8  никель-кадмиевые/металлгидридные 1,25В/1,37В  ток заряда - 1С  напр. отсечки при заряде - 1,6В(1,37В)      (0,85В/1В)
  };
  // Li-Pol  литий-полимерные 3,7В напр. отсечки при заряде - 4,2 напр хранения- 3,85  напр. отсечки при разряде - 3В
*/




//******** НЕ ТРОГАТЬ **********
#if defined(__LGT8FX8P__)
#define BITRATE 4096  // разрядность АЦП
#define BIT 12        // разрядность АЦП бит
#else
#define BITRATE 1024  // разрядность АЦП
#define BIT 10        // разрядность АЦП бит
#endif

// частота силового модуля {0.25, 0.5, 1, 2, 4, 8, 31, 62} кГц
#define FREQ_CHARGE 7
// частота разрядного модуля {0.25, 0.5, 1, 2, 4, 8, 31, 62} кГц
#define FREQ_DISCHARGE 4
// коэффициент напряжения INA226 в микровольтах + 1
#define INAVOLTKOOF 250
// адрес начальной ячейки памяти для pam
#define MEM 30
// адрес начальной ячейки памяти для результатов замера емкости КТЦ. Максимум 10 замеров по 16 байт
#define MEM_KTC 850


// название бит для pam.MyFlag
#define CHARGE 0
#define OSCIL 1
#define BRANIM 2 
#define PRECHAR 3
#define BRANIMBOIL 4   // флаг перемешивания электролита

// название бит для vkr.GlobFlag
#define TRQ1 0
#define INAERR 1

// номера режимов работы
#define CHARGb  0
#define CHADCRb 1
#define BRANb   2
#define ADDCRb  3
#define DISCHRb 4
#define KTCb    5
#define STORb   6
#define BUFb    7

// количество экранов при заряде на дисплей 1602
#if (DISPLAYy == 2)
#define WINDC 4
// количество экранов на дисплей 2004
#elif (DISPLAYy == 4)
#define WINDC 2
#endif

#define InvBit(reg, bit) reg ^= (1 << bit)  //  nvBit(tmp,6);    //инвертировать шестой бит переменной tmp
#define BitIsClear(reg, bit) ((reg & (1 << bit)) == 0)
//if  (BitIsClear(PIND, 0)) {   //если очищен нулевой бит в регистре PIND    //выполнить блок



/*
Карта организации памяти EPRROM
0 - пусто
1 - 7 - struct vkr 7 байт
8 - 23 - архив 15 байт// uint8_t service[SETTINGS_AMOUNT];  // основные параметры зарядника
30 - 105 - профиль 0: struct pam 75 байт
106 - 181 - профиль 1: struct pam 75 байт
181 - 256 - профиль 2: struct pam 75 байт
257 - 332 - профиль 3: struct pam 75 байт
333 - 408 - профиль 4: struct pam 75 байт
409 - 484 - профиль 5: struct pam 75 байт
485 - 560 - профиль 6: struct pam 75 байт
561 - 636 - профиль 7: struct pam 75 байт
637 - 712 - профиль 8: struct pam 75 байт
713 - 788 - профиль 9: struct pam 75 байт
789 - 848 - пусто 59 байт
849 - текущее количество циклов
850 - 866 - КТЦ-1, архив 16 байт int32_t KTC[4] = { 0 };  //  Ah_discharge, Wh_discharge, Ah_charge, Wh_charge, КТЦ
867 - 883 - КТЦ-2
884 - 900 - КТЦ-3
901 - 917 - КТЦ-4
918 - 934 - КТЦ-5
935 - 951 - КТЦ-6
952 - 968 - КТЦ-7
967 - 985 - КТЦ-8
986 - 1002 - КТЦ-9
1003 - 1019 - КТЦ-10
*/