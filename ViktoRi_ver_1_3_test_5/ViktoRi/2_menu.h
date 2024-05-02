// названия аккумуляторов
const char name1[] PROGMEM = "Pb Ca/Ca";
const char name2[] PROGMEM = "Pb Ca+";
const char name3[] PROGMEM = "Pb Sur";
const char name4[] PROGMEM = "Pb AGM";
const char name5[] PROGMEM = "Pb Gel";
const char name6[] PROGMEM = "Li-ion";
const char name7[] PROGMEM = "LiFePo4";
const char name8[] PROGMEM = "LiTit";
const char name9[] PROGMEM = "NiCd/Mh";

// объявляем таблицу ссылок
const char* const typeAkb[] PROGMEM = {
  name1, name2, name3, name4, name5,
  name6, name7, name8, name9,
};

#define POINTMENU 9       // число пунктов Меню
#define POINTMODE 7       // число пунктов Режимов работы
#define POINTSETTINGS 20  // число пунктов Настроек
#define POINTCALIBRS 1    // число пунктов Калибровок
#define SYSPARAM 19       // номер системных параметров

// русский (!для дисплеев с поддержкой русского языка!)
#if (INTERFACE == 0)
const char namm1[] PROGMEM = "Pr:";                                                       //  0   Профиль
const char namm2[] PROGMEM = "\x45\xBC\xBA\x6F\x63\xBF\xC4 \x41\x4B\xA0";                 //  1   Емкость  // АКБ
const char namm3[] PROGMEM = "\x54\xB8\xBE \x41\x4B\xA0";                                 //  2   Тип АКБ
const char namm4[] PROGMEM = "\x48\x61\xBE\x70\xC7\xB6\x65\xBD\xB8\x65 \x41\x4B\xA0";     //  3   Напряжение АКБ
const char namm5[] PROGMEM = "\x50\x65\xB6\xB8\xBC \x70\x61\xB2\x6F\xBF\xC3";             //  4   Режим работы
const char namm6[] PROGMEM = "\x48\x61\x63\xBF\x70\x6F\xB9\xBA\xB8";                      //  5   Настройки
const char namm7[] PROGMEM = "\x43\x6F\xBE\x70\x6F\xBF\x2E\x41\x4B\xA0";                  //  6   Сопрот. АКБ
const char namm8[] PROGMEM = "\x43\xBF\x61\xBF\xB8\x63\xBF\xB8\xBA\x61";                  //  7   Статистика
const char namm9[] PROGMEM = "\x4B\x61\xBB\xB8\xB2\x70\x6F\xB3\xBA\x61";                  //  8   Калибровка
const char namm10[] PROGMEM = " ";

const char* const menuTxt[] PROGMEM = {
  namm1, namm2, namm3, namm4, namm5,
  namm6, namm7, namm8, namm9, namm10,
};

const char namd1[] PROGMEM = "\xA4\x61\x70\xC7\xE3";                                       //  0   Заряд
const char namd2[] PROGMEM = "\xA4\x61\x70\xC7\xE3>\xE0\x6F\xB7\x61\x70\xC7\xE3";          //  1   Заряд>Дозаряд
const char namd3[] PROGMEM = "\xA4\x61\x70\xC7\xE3 \xA0\x70\x61\xBD\xB8\xBC\xB8\x70\x61";  //  2   Заряд Бранимир
const char namd4[] PROGMEM = "\xE0\x6F\xB7\x61\x70\xC7\xE3";                               //  3   Дозаряд
const char namd5[] PROGMEM = "\x50\x61\xB7\x70\xC7\xE3";                                   //  4   Разряд
const char namd6[] PROGMEM = "KT\xE5";                                                     //  5   КТЦ (Разр>Зар>Дозар)
const char namd7[] PROGMEM = "\x58\x70\x61\xBD\x65\xBD\xB8\x65";                           //  6   Хранение АКБ
const char namd8[] PROGMEM = "\xA0\x79\xE4.\x70\x65\xB6\xB8\xBC";                          //  7   Буф.режим  // Буферный режим "\xA0\x79\xE4\x65\x70\xBD\xc3\xB9 \x70\x65\xB6\xB8\xBC";

const char* const modeTxt[] PROGMEM = {
  namd1, namd2, namd3, namd4,
  namd5, namd6, namd7, namd8,
};

#if (DISPLAYy == 2)
const char regimr[] PROGMEM = "\xA4\xA4\xA0\xE0\x50K";  // символы вывода режима работы - Заряд, Заряд, Бранимир, Дозаряд, Разряд, КТЦ
#endif

const char nami1[] PROGMEM = "\x48\x61\xBE\x70\xC7\xB6 \xB7\x61\x70\xC7\xE3\x61";              //  0   Напряж. зарядa  *
const char nami2[] PROGMEM = "\x54\x6F\xBA \xB7\x61\x70\xC7\xE3\x61";                          //  1   Ток заряда      *
const char nami3[] PROGMEM = "\x48\x61\xBE\x70\xC7\xB6 \x43\xBD\xB8\xB6.";                     //  2   Напряжение при котором начинает снижаться ток
const char nami4[] PROGMEM = "\x4D\xB8\xBD. \xBF\x6F\xBA \xB7\x61\x70\xC7\xE3\x61";            //  3   Мин. ток заряда *
const char nami5[] PROGMEM = "\x42\x70\x65\xBC\xC7 \xB7\x61\x70\xC7\xE3\x61";                  //  4   Время заряда    *
const char nami6[] PROGMEM = "\x48\x61\xBE\x70. \xE3\x6F\xB7\x61\x70\xC7\xE3\x61";             //  5   Напр. дозаряда  *
const char nami7[] PROGMEM = "\x54\x6F\xBA \xE3\x6F\xB7\x61\x70\xC7\xE3\x61";                  //  6   Ток дозаряда    *
const char nami8[] PROGMEM = "\x4D\xB8\xBD. \xBF\x6F\xBA \xE3\x6F\xB7\x61\x70\xC7\xE3\x61";    //  7   Мин. ток дозар. *
const char nami9[] PROGMEM = "\x42\x70\x65\xBC\xC7 \xE3\x6F\xB7\x61\x70\xC7\xE3\x61";          //  8  Время дозаряда  *
const char nami10[] PROGMEM = "\x48\x61\xBE\x70. \x70\x61\xB7\x70\xC7\xE3\x61";                 //  9  Наряж. разряда  *
const char nami11[] PROGMEM = "\x54\x6F\xBA \x70\x61\xB7\x70\xC7\xE3\x61";                     //  10  Ток разряда     *
const char nami12[] PROGMEM = "\x4D\xB8\xBD. \xBF\x6F\xBA \x70\x61\xB7\x70\xC7\xE3\x61";       //  11  Мин. ток разряда*
const char nami13[] PROGMEM = "\xA0\x79\xE4\x65\x70\xBD\xc3\xB9 \x70\x65\xB6\xB8\xBC";         //  12  Буферный режим  *
const char nami14[] PROGMEM = "\x58\x70\x61\xBD\x65\xBD\xB8\x65";                              //  13  Хранение
const char nami15[] PROGMEM = "\xA8\x70\x65\xE3\xB7\x61\x70\xC7\xE3";                          //  14  Предзаряд       *
const char nami16[] PROGMEM = "\x48\x61\xBE\x70. \xBE\x70\x65\xE3\xB7\x61\x70\xC7\xE3\x61";    //  15  Напряж. предзар.*
const char nami17[] PROGMEM = "\x54\x6F\xBA \xBE\x70\x65\xE3\xB7\x61\x70\xC7\xE3\x61";         //  16  Ток предзаряда  *
const char nami18[] PROGMEM = "\x4B\x61\xC0\x65\xBB\xB8";                                      //  17  Качели          *
const char nami19[] PROGMEM = "\xE1\xB8\xBA\xBB";                                              //  18  Цикл
const char nami20[] PROGMEM = "\x21\x43\xB8\x63\xBF\x65\xBC\x2E\x20\xBE\x61\x70\x61\xBC\x21";  //  19  !Cистем. парам! *
const char nami21[] PROGMEM = "\x43\xB2\x70\x6F\x63 \xBD\x61\x63\xBF\x70\x6F\x65\xBA";         //  20  Сброс настроек  *

const char* const settings[] PROGMEM = {
  nami1, nami2, nami3, nami4, nami5, nami6, nami7, nami8, nami9, nami10, nami11,
  nami12, nami13, nami14, nami15,  nami16, nami17, nami18, nami19, nami20, nami21,
};

const char namc1[] PROGMEM = "\x48\x61\xBE\x70\xC7\xB6/\x54\x6F\xBA \x41\xBA\xB2";  // Напряж/ток акб
const char namc2[] PROGMEM = "\x48\x61\xBE\x70\xC7\xB6 \xA0\xA8";                   // Напряж. БП

const char* const calibr[] PROGMEM = { namc1, namc2 };

#define txt2 " \x63\xB2\x70\x6F\x63"                           //  сброc
#define txt3 " \x63\x6F\x78\x70\x21"                           //  сохр!
#define txt4 "\x42\x78\x2E\x20\xA0\xA8 "                       //  "Вх. БП "
#define txt5 "\x42\xC3\x78\x6F\xE3"                            //  Выход
#define txt6 "INA226 \x6F\xC1\xB8\xB2\xBA\x61"                 //  INA226
#define txt7 "DS18B20 "                                        //  DS18B20
//#define txt8 "!ALERT!"                                         //  
#define txt9 "Temp akb:"                                       //  температура акб 
#define txt10 "\x42\x63\xA2"                                   //  Всё
#define txt11 "\xA8\x6F\xE3\xBA\xBB\xC6\xC0\xB8 \x41\x4B\xA0"  //  Подключи АКБ
#define txt12 "\x43\xB2\x70\x6F\x63"                           //  Сброс
#define txt13 "\x42\xBA\xBB\x2E "                              //  Вкл.
#define txt14 "\x4F\xBF\xBA\xBB\x2E"                           //  Откл.
//#define txt15 "\xAB\x61\x63"                                   //  Час
#define txt16 "\xA8\x61\x79\xB7\x61"                           //  Пауза
#define txt17 "\x3C\x4F\x4B\x3E"                               // "< OK >"
//#define txt18 "V Bpe\xBC\xC7 "                              //  V Время
#define txt19 "\x3C\x43\xBF\x6F\xBE\x3e"                       //  "<Стоп>"
#define txt20 "\xBC\xB8\xBD"                                   //  мин
#define txt21 "\x6F\xC1\xB8\xB2\xBA\x61"                              //  ошибка
#define txt22 "Volt"                                                  // Вольт
#define txt23 "Amper"                                                 // Ампер
#define txt24 "!-\xA8\x50\x4F\xA0\xA5\x54 Q1-!"                       //  !-ПРОБИТ Q1-!
#define txt25 "\x56\xB2\xBE "                                         //  Vбп
//#define txt26 "\x41\x4B\xA0\x20\x70\x61\xB7\x70\xC7\xB6\x65\xBD\x21"  //  АКБ разряжен!
//const char err[] PROGMEM = "\x6F\xC1\xB8\xB2\xBA\x61";

// английский
#elif (INTERFACE == 1)
const char namm1[] PROGMEM = "Pr:";
const char namm2[] PROGMEM = "Capacity Akb";
const char namm3[] PROGMEM = "Type Akb";
const char namm4[] PROGMEM = "Voltage Akb";
const char namm5[] PROGMEM = "Operating mode";
const char namm6[] PROGMEM = "Settings";
const char namm7[] PROGMEM = "Resistance Akb";
const char namm8[] PROGMEM = "Statistics";
const char namm9[] PROGMEM = "Calibration";
const char namm10[] PROGMEM = " ";

const char* const menuTxt[] PROGMEM = {
  namm1, namm2, namm3, namm4, namm5,
  namm6, namm7, namm8, namm9, namm10,
};

const char namd1[] PROGMEM = "Charge";
const char namd2[] PROGMEM = "Charge>AddChar";
const char namd3[] PROGMEM = "Charge Branim";
const char namd4[] PROGMEM = "Add charge";
const char namd5[] PROGMEM = "Discharge";
const char namd6[] PROGMEM = "Contr.trai.cycle";
const char namd7[] PROGMEM = "Storage";
const char namd8[] PROGMEM = "Buffer mode";  // "Buffer mode"

const char* const modeTxt[] PROGMEM = {
  namd1, namd2, namd3, namd4,
  namd5, namd6, namd7, namd8,
};

#if (DISPLAYy == 2)
const char regimr[] PROGMEM = "CCBADK";  // символы вывода режима работы - Заряд, Заряд, Бранимир, Дозаряд, Разряд, КТЦ
#endif

const char nami1[] PROGMEM = "Volt charge";        //  0   Напряж. зарядa
const char nami2[] PROGMEM = "Curr. charge";       //  1   Ток заряда
const char nami3[] PROGMEM = "Volt decr.Cur";      //  2   Напряжение при котором начинает снижаться ток
const char nami4[] PROGMEM = "Curr. charge off";   //  3   ток завершения заряда
const char nami5[] PROGMEM = "Time charge";        //  4   Время заряда
const char nami6[] PROGMEM = "Volt add charge";    //  5   Напряж. дозаряда
const char nami7[] PROGMEM = "Curr. add charge";   //  6   Ток дозаряда
const char nami8[] PROGMEM = "Curr.addChar min";   //  7   Мин. ток дозар.
const char nami9[] PROGMEM = "Time add charge";    //  8   Время дозаряда
const char nami10[] PROGMEM = "Volt dischar";      //  9   Наряж. разряда
const char nami11[] PROGMEM = "Curr. dischar";     //  10   Ток разряда
const char nami12[] PROGMEM = "Curr.disch. off";   //  11  Мин. ток разряда
const char nami13[] PROGMEM = "Bufer mode";        //  12  Буферный режим
const char nami14[] PROGMEM = "Storage mode";      //  13  напряжение режима Хранение
const char nami15[] PROGMEM = "Precharge";         //  14  Предзаряд
const char nami16[] PROGMEM = "Volt pre charge";   //  15  Напряж. предзар.
const char nami17[] PROGMEM = "Curr. pre charge";  //  16  Ток предзаряда
const char nami18[] PROGMEM = "Oscillation";       //  17  Качели
const char nami19[] PROGMEM = "Cycle";             //  18  Цикл
const char nami20[] PROGMEM = "!system param!";    //  19  #define SYSPARAM 18       // номер системных параметров
const char nami21[] PROGMEM = "Reset settings";    //  20   Сброс настроек

const char* const settings[] PROGMEM = {
  nami1, nami2, nami3, nami4, nami5, nami6, nami7, nami8, nami9, nami10, nami11,
  nami12, nami13, nami14, nami15,  nami16, nami17, nami18, nami19, nami20, nami21,
};

const char namc1[] PROGMEM = "Volt/Curr akb";
const char namc2[] PROGMEM = "Volt Power";

const char* const calibr[] PROGMEM = {
  namc1,
  namc2,
};

#define txt2 " reset"
#define txt3 " saved"  // 2 раза
#define txt4 "Power "
#define txt5 "Exit"
#define txt6 "INA226 error"
#define txt7 "DS18B20 "
//#define txt8 "!ALERT!"
#define txt9 "Temp akb:"                                       //  температура акб 
#define txt10 "All"
#define txt11 "Connect Batt"
#define txt12 "Sbros"
#define txt13 "On "
#define txt14 "Off "
//#define txt15 " hour" // 2 раза
#define txt16 "Pause"
#define txt17 "< OK >"
//#define txt18 "V  Time "
#define txt19 "<Stop>"
#define txt20 " min "
#define txt21 "error"  // 6 раз
#define txt22 "Volt" 
#define txt23 "Amper"
#define txt24 "!-BROKEN Q1-!"
#define txt25 "Vin "  //  Напряж БП превыш
//#define txt26 "Akb razryazen!"
//const char err[] PROGMEM = "error";

// транстит
#elif (INTERFACE == 2)
const char namm1[] PROGMEM = "Pr:";
const char namm2[] PROGMEM = "Emkost akb";
const char namm3[] PROGMEM = "Tip Akb";
const char namm4[] PROGMEM = "Napryagenie akb";
const char namm5[] PROGMEM = "Regim raboti";
const char namm6[] PROGMEM = "Nastroiki";
const char namm7[] PROGMEM = "Soprotivl. akb";
const char namm8[] PROGMEM = "Statistika";
const char namm9[] PROGMEM = "Kalibrovka";
const char namm10[] PROGMEM = " ";

const char* const menuTxt[] PROGMEM = {
  namm1, namm2, namm3, namm4, namm5,
  namm6, namm7, namm8, namm9, namm10,
};

const char namd1[] PROGMEM = "Zaryad";
const char namd2[] PROGMEM = "Zar.>Dozaryad";
const char namd3[] PROGMEM = "Zar. Branimir";
const char namd4[] PROGMEM = "Dozaryad";
const char namd5[] PROGMEM = "Razryad";
const char namd6[] PROGMEM = "KTC";
const char namd7[] PROGMEM = "Hranenie";
const char namd8[] PROGMEM = "Buf. regim";  // "Bufer regim"

const char* const modeTxt[] PROGMEM = {
  namd1, namd2, namd3, namd4,
  namd5, namd6, namd7, namd8,
};

#if (DISPLAYy == 2)
const char regimr[] PROGMEM = "ZZBDRK";  // символы вывода режима работы - Заряд, Заряд, Бранимир, Дозаряд, Разряд, КТЦ
#endif

const char nami1[] PROGMEM = "Napr. zaryada";
const char nami2[] PROGMEM = "Tok zaryada";
const char nami3[] PROGMEM = "Napr. snigen";      //  2   Напряжение при котором начинает снижаться ток
const char nami4[] PROGMEM = "Min. tok zaryda";
const char nami5[] PROGMEM = "Vremya zaryada";
const char nami6[] PROGMEM = "Napr. dozaryada";
const char nami7[] PROGMEM = "Tok dozaryada";
const char nami8[] PROGMEM = "Min. tok dozar.";
const char nami9[] PROGMEM = "Vremya dozaryada";
const char nami10[] PROGMEM = "Napr. razryada";
const char nami11[] PROGMEM = "Tok razryada";
const char nami12[] PROGMEM = "Min. tok razr.";
const char nami13[] PROGMEM = "Bufernii regim";
const char nami14[] PROGMEM = "Xranenie Akb";
const char nami15[] PROGMEM = "Predzaryad";
const char nami16[] PROGMEM = "Napr. predzaryda";
const char nami17[] PROGMEM = "Tok predzaryda";
const char nami18[] PROGMEM = "Regim kacheli";
const char nami19[] PROGMEM = "Cycle";
const char nami20[] PROGMEM = "!system param!";
const char nami21[] PROGMEM = "Sbros nastroek";

const char* const settings[] PROGMEM = {
  nami1, nami2, nami3, nami4, nami5, nami6, nami7, nami8, nami9, nami10, nami11,
  nami12, nami13, nami14, nami15,  nami16, nami17, nami18, nami19, nami20, nami21,
};

const char namc1[] PROGMEM = "Volt/Curr akb";
const char namc2[] PROGMEM = "Volt Power";

const char* const calibr[] PROGMEM = { namc1, namc2 };

#define txt2 " sbros"
#define txt3 " sohr."
#define txt4 "Power "
#define txt5 "Exit"
#define txt6 "INA226 error"
#define txt7 "DS18B20 "
//#define txt8 "!ALERT!"
#define txt9 "Temp akb:"                                       //  температура акб 
#define txt10 "Vse"
#define txt11 "Podkluchi Akb"
#define txt12 "Sbros"
#define txt13 "Vkl."
#define txt14 "Otkl."
//#define txt15 " chas"
#define txt16 "Pause"
#define txt17 "< OK >"
//#define txt18 "V  Time "
#define txt19 "<Stop>"
#define txt20 "min"
#define txt21 "error"
#define txt22 "Volt"
#define txt23 "Amper"
#define txt24 "!-PROBIT Q1-!"
#define txt25 "Vin "  //  Напряж БП превыш
//#define txt26 "Akb razryazen!"
//const char err[] PROGMEM = "error";
#endif


// ------сервисные параметры-----------
#define SETTINGS_AMOUNT 15  // количество настроек
const char service0[] PROGMEM = "POWER";
const char service1[] PROGMEM = "POWER_MAX";
const char service2[] PROGMEM = "INAVKOOF";
const char service3[] PROGMEM = "CURR_MAX";
const char service4[] PROGMEM = "FAN";
const char service5[] PROGMEM = "DEG_ON";
const char service6[] PROGMEM = "DEG_OFF";
const char service7[] PROGMEM = "DEG_MAX";
const char service8[] PROGMEM = "DEG_CUR";
const char service9[] PROGMEM = "CURFAN_ON";
const char service10[] PROGMEM = "CURFAN_OFF";
const char service11[] PROGMEM = "CURFAN_MAX";
const char service12[] PROGMEM = "TIME_LIGHT";
const char service13[] PROGMEM = "FREQ_CHARGE";
const char service14[] PROGMEM = "FREQ_DISCHAR";


#define POWERINT ((uint16_t)EEPROM.read(8+0) * 1000)   // напряжение от БП
#define POWERMAX 1                                   // максимальное напряжение от БП
#define INAVKOOFV  ((float)EEPROM.read(8+2) / 1000 + 1.0f)     // коэффициент напряжения INA226 
#define CURRMAXINT ((int16_t)EEPROM.read(8+3) * 100)   // максимальный ток
#define FANMODE 4                                    // режим работы кулера
#define DEGON 5                                      // температура включения вентилятора в градусах
#define DEGOFF 6                                     // температура отключения вентилятора в градусах
#define DEGMAX 7                                     // значение температуры для максимальных оборотов вентилятора ШИМ.
#define DEGCUR 8                                     // температура при которой уменьшается ток заряда
#define CURFANON 9                                   // значение тока в Амперах при котором включается вентилятор. (до 25,5А)
#define CURFANOFF 10                                 // значение тока в Амперах при котором отключается вентилятор. (до 25,5А)
#define CURFANMAX 11                                 // значение тока в Амперах  для максимальных оборотов вентилятора ШИМ. (до 25,5А)
#define TIMELIGHT 12                                 // время подсветки дисплея в минутах
#define FREQCHARGE 13                                // частота работы силовогомодуля
#define FREQDISCHAR 14                               // частота работы разрядного модуля


const char* const servicc[] PROGMEM = {
  service0, service1, service2, service3, service4, service5, service6, service7,
  service8, service9, service10, service11, service12, service13, service14,
};

// символы аккумулятора и заряда
const char simb_array[8][8] PROGMEM = {
  { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F },  // akb 0
  { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x1F },  // akb 1
  { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x1F, 0x1F, 0x1F },  // akb 2
  { 0x0E, 0x11, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x1F },  // akb 3
  { 0x0E, 0x11, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },  // akb 4
  { 0x0E, 0x11, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },  // akb 5
  { 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },  // akb 6
  { 0x03, 0x06, 0x0C, 0x18, 0x1F, 0x06, 0x0C, 0x18 },  // заряд 7
};

//const char simbols[] PROGMEM = "VAWCNh%()<>:; ";

#define PROBEL 32  // пробел
#define LEFTs  60  // <
#define RIGHTs 62  // >

#define txt_C "C "
#define txt_PRC "% "
#define txt_OK "OK"


