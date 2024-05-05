#include "1_User_Setup.h"
#include "2_menu.h"
#include <FunctionLite.h>
#include <EEPROM.h>

#if (VOLTIN == 1)
uint8_t flag_global = 0b00100000;
#else
uint8_t flag_global = 0b00100010;
#endif
#define POWERHIGH 0    // напряжение блока питания (true - напряжения питания в норме)
#define POWERON 1      // питание от БП поступает
#define TR_Q1_ERR 2    // превышена температура на силовом транзисторе
//#define DISP_LIGHT 3   // разрешено отключать подсветку дисплея
#define RELEY_OFF 4    // разрешено отключать реле
#define TEMP_AKB 5     // температура акб (true - в норме)

#if (SENSTEMP1 == 1)
#include <microDS18B20.h>      // Библиотека датчика температуры DS18B20.
MicroDS18B20<PINTERM1> tr_Q1;  // датчик температуры.
#endif
#if (SENSTEMP1 == 2 or SENSTEMP2 == 2)
#include "NTCv.h"  // Класс датчика температуры NTC
NTCv ntc; // создать обьект ntc
#endif

#define EB_DEB_TIME 50     // таймаут гашения дребезга кнопки (кнопка)
#define EB_CLICK_TIME 500  // таймаут ожидания кликов (кнопка)
#define EB_HOLD_TIME 1000  // таймаут удержания (кнопка)
#define EB_STEP_TIME 200   // таймаут импульсного удержания (кнопка)
#define EB_FAST_TIME 30    // таймаут быстрого поворота (энкодер)
#define EB_NO_CALLBACK     // отключить обработчик событий attach (экономит 2 байта оперативки)
#define EB_NO_COUNTER      // отключить счётчик энкодера [VirtEncoder, Encoder, EncButton] (экономит 4 байта оперативки)
//#define EB_NO_BUFFER     // отключить буферизацию энкодера (экономит 2 байта оперативки)
#include <EncButton.h>  // Библиотека энкодера и кнопок
#if (ENCBUTT == 0)
// 0 - только энкодер
EncButtonT<ENC_S2, ENC_S1, ENC_KL> enc(INPUT_PULLUP, INPUT_PULLUP);  // энкодер с кнопкой
#elif (ENCBUTT == 1)
// 1 - только кнопки
ButtonT<PUSK> OK(INPUT_PULLUP);      // кнопка - Пуск
ButtonT<MINUS> Minus(INPUT_PULLUP);  // кнопка - Минус
ButtonT<PLUS> Plus(INPUT_PULLUP);    // кнопка - Плюс
ButtonT<STOP> Stop(INPUT_PULLUP);    // кнопка - Стоп
#elif (ENCBUTT == 2)
// 2 - энкодер + OK + Stop
EncButtonT<ENC_S2, ENC_S1, ENC_KL> enc(INPUT_PULLUP, INPUT_PULLUP);
ButtonT<PUSK> OK(INPUT_PULLUP);
ButtonT<STOP> Stop(INPUT_PULLUP);
#elif (ENCBUTT == 3)
// 3 - энкодер + все кнопки
EncButtonT<ENC_S2, ENC_S1, ENC_KL> enc(INPUT_PULLUP, INPUT_PULLUP);
ButtonT<PUSK> OK(INPUT_PULLUP);
ButtonT<MINUS> Minus(INPUT_PULLUP);
ButtonT<PLUS> Plus(INPUT_PULLUP);
ButtonT<STOP> Stop(INPUT_PULLUP);
#endif

class TimerMs {
public:

  TimerMs(const uint16_t p) : _prd(p) {}

  // запустить/перезапустить таймер
  void start(void) {
    _tmr = millis();
  }

  bool tick(void) {
    if (millis() - _tmr >= _prd) {
      start();
      return true;
    }
    return false;
  }

private:
  uint32_t _tmr = 0; 
  const uint16_t _prd;
};
TimerMs sekd(1000); // таймер 1 секунда для вывода на дисплей и периодических функций
TimerMs sekv(1000); // таймер 1 секунда для постоянно работающих функций
#if (POWPIN == 1)
TimerMs tvin(60000); // таймер 1 минута для реле 220В
#endif

#if (MCP4725DAC)
#include "MCP4725my.h"
MCP4725 dac(ADDR4725);
#endif

// 7 байт
struct Intls {
  uint8_t GlobFlag;                  // Q1 - состояние силового транзистора Q1 - рабочий/пробит
  uint8_t Ohms;                      // кооффицмент коррекции напряжения 0-255 (сопротивление линии до акб)
  uint8_t profil;                    // текущий профиль 0-9
  uint16_t Vref;                     // референсное напряжение
  uint16_t shunt;                    // значение сопротивления шунта (1000 - 10мОм)  
} vkr;

// 77 байт
struct MyStrukt {
  uint8_t MyFlag;                  // - (Precharge,Branimir,Oscillation,Charge)     bool Oscillation;   // oscillation bool  1 бит - режим качели (ассиметричный заряд)  bool branimir = true;
  uint8_t typeAkb;                 // - тип аккумулятора
  uint8_t Mode;                    // - режм заряда
  uint8_t Capacity;                // - емкость аккумулятора
  uint8_t Voltage;                 // - количество ячеек аккумулятора
  uint8_t Number;                  // - номер
  uint8_t Round;                   // - текущий круг
  int32_t Ah_charge;               // - емкость заряда - миллиампер/часы / 100
  int32_t Wh_charge;               // - емкость заряда - милливатт/часы / 100
  uint32_t Time_charge;            // - время заряда - миллисекунды
  uint16_t Volt_charge;            // - напряжение заряда - миливольт / 1000 (макс 65.535 Вольт)
  uint16_t Volt_decrCur;           // - Напряжение при котором начинает снижаться ток
  uint16_t Volt_add_charge;        // - напряжение дозаряда - миливольт / 1000 (макс 65.535 Вольт)
  int16_t Current_charge;          // - ток заряда - миллиампер / 1000 (макс 32,767 Ампер)
  int16_t Current_add_charge;      // - ток дозаряда - миллиампер / 1000 (макс 32,767 Ампер)
  int16_t Current_charge_min;      // - ток завершения заряда - миллиампер / 1000 (макс 32,767 Ампер)
  int16_t Current_add_charge_min;  // - ток завершения дозаряда - миллиампер / 1000 (макс 32,767 Ампер)
  uint8_t Time_charge_h;           // - время заряда, часы
  uint8_t Time_add_charge_h;       // - время дозаряда, часы
  uint16_t Volt_discharge;         // - напряжение разряда - миливольт / 1000 (макс 65.535 Вольт)
  int16_t Current_discharge;       // - ток разряда - миллиампер / 1000 (макс 32,767 Ампер)
  int16_t Current_discharge_min;   // - ток завершения разряда - миллиампер / 1000 (макс 32,767 Ампер)
  uint16_t Volt_buffer;            // - напряжение буферного (выравнивающего режима) - миливольт / 1000 (макс 65.535 Вольт)
  uint16_t Volt_storage;           // - напряжение хранения - миливольт / 1000 (макс 65.535 Вольт)
  uint16_t Volt_pre_charge;        // - напряжение предзаряда (для сильно разр. аккумул) - миливольт / 1000 (макс 65.535 Вольт)
  int16_t Current_pre_charge;      // - ток предзаряда (для сильно разр. аккумул) - миллиампер / 1000 (макс 32,767 Ампер)
  int32_t Ah_discharge;            // - емкость разряда - миллиампер/часы / 1000
  int32_t Wh_discharge;            // - емкость разряда - милливатт/часы / 1000
  uint32_t Time_discharge;         // - время разряда - миллисекунды
  int32_t Ah_addcharge;            // - емкость дозаряда до напряжения аккумулятора - миллиампер/часы / 1000000
  int32_t Wh_addcharge;            // - емкость дозаряда до 12 вольт - милливатт/часы / 1000000
  uint32_t Time_addcharge;         // - время дозаряда - миллисекунды
  uint16_t Resist_1;               // - внутренее сопротивление аккумулятора - миллиОм / 100 - до
  uint16_t Resist_2;               // - пусковой ток
} pam;

#include <LiquidCrystal_I2C.h>  // Библиотека дисплея
LiquidCrystal_I2C lcd(ADDRDISP, DISPLAYx, DISPLAYy);
#include "Display.h"

uint8_t regim_end = 0;  // номер причины завершения заряда

// напряжение одной ячейки аккумулятора (умножить на 10, в мВ)
const uint16_t bt_volt[4][9] PROGMEM = {
  { 210, 210, 210, 210, 210, 365, 320, 240, 125 },  // напряжение одной ячейки аккумулятора
  { 245, 243, 241, 243, 235, 410, 330, 270, 160 },  // напряжение заряда ячейки аккумулятора
  { 271, 273, 250, 260, 240, 0, 0, 0, 0 },          // напряжение дозаряда ячейки аккумулятора - только для свинцово-кислотных
  { 193, 193, 193, 193, 193, 250, 280, 160, 100 },  // напряжение разряда ячейки аккумулятора
};
// ток заряда, указано в процентах от емкости Акб
const uint8_t bt_cur[3][9] PROGMEM = {
  { 10, 10, 10, 10, 10, 50, 50, 100, 100 },  // ток заряда ячейки аккумулятора, указано в процентах от емкости Акб
  { 2, 2, 2, 2, 1, 0, 0, 0, 0 },             // ток дозаряда ячейки аккумулятора,...
  { 4, 4, 4, 4, 4, 50, 50, 50, 50 },         // ток разряда ячейки аккумулятора,...
};

class Batt {
public:
  uint16_t Volt(uint8_t i) {
    return (uint16_t)pgm_read_word(&bt_volt[i][pam.typeAkb]) * 10 * pam.Voltage;
  }
  int16_t Cur(uint8_t i) {
    return (int16_t)pgm_read_byte(&bt_cur[i][pam.typeAkb]) * 10 * pam.Capacity;
  }
} bat;

void (*resetFunc)(void) = 0;  // функция reset с адресом 0

#include "INA226int.h"  // Библиотека INA226
INA226 ina;             // адрес INA226

#include "DCDC.h"  // класс управления DC-DC модулем

#if (VOLTIN == 1)
#include "ADCSred.h"
ADCSred vin;  // замер напряжения БП
#endif

#include "Kuler.h"  // класс управления кулером и чтения температуры

#include "AllFunctions.h"

// настройка команд энкодера и кнопок
#define LEFT -1
#define RIGHT 1
#define ENCCLICK 16
#define ENCHELD 2
#define OKCLICK 26
#define OKHELD 12
#define STOPCLICK 36
#define STOPHELD 22
#define PLUSHELD 32
#define MINUSHELD 42
class Enbutt {
public:

  Enbutt(void){};

  int8_t tick;
  // функция опрашивает энкодер и кнопки.
  void Tick(void) {
    tick = 0;
// -1 - left, 1 - right, 16 - click, 2 - hold, 4 - step
#if (ENCBUTT == 0)
    // 0 - только энкодер
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
#elif (ENCBUTT == 1)
    // 1 - только кнопки
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
    if (Plus.tick()) {
      if (Plus.click() or Plus.step()) tick = RIGHT;
    }
    if (Minus.tick()) {
      if (Minus.click() or Minus.step()) tick = LEFT;
    }
#elif (ENCBUTT == 2)
    // 2 - энкодер + OK + Stop
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
#elif (ENCBUTT == 3)
    // 3 - энкодер + все кнопки
    if (enc.tick()) {
      if (enc.turn()) tick = enc.dir();  // -1 - left, 1 - right,
      else {
        if (enc.click()) tick = 16;
        if (enc.hold()) tick = 2;
      }
    }
    if (OK.tick()) {
      if (OK.click()) tick = 26;
      if (OK.hold()) tick = 12;
    }
    if (Stop.tick()) {
      if (Stop.click()) tick = 36;
      if (Stop.hold()) tick = 22;
    }
    if (Plus.tick()) {
      if (Plus.click() or Plus.step()) tick = RIGHT;
    }
    if (Minus.tick()) {
      if (Minus.click() or Minus.step()) tick = LEFT;
    }
#endif
#if (TIME_LIGHT)
    if (tick) disp.Light_high();
#endif
  }
  //private:
} butt;


void setup() {
  // настройка пинов  
  gio::mode(PWMCH, OUTPUT);   // ШИМ заряд
  gio::mode(BUZER, OUTPUT);   // пин вывода звука
  gio::mode(PIN_13, OUTPUT);  // управление реле нагрузки в буферном режиме

#if (DISCHAR == 1)
  gio::mode(PWMDCH, OUTPUT);  // ШИМ разряд
#endif
#if (POWPIN)
  gio::mode(RELAY220, OUTPUT);  // управление реле подключения к сети 220В
#endif
#if (PROT)
  gio::mode(PROTECT, OUTPUT);  // управление защитой
#endif
#if (SENSTEMP1 == 2)
  gio::mode(PINTERM1, INPUT);  // вход датчика температуры силового транзистора
#endif
#if (SENSTEMP2 == 2)
  gio::mode(PINTERM2, INPUT);  // вход датчика температуры NTC
#endif
#if (GUARDA0)
  gio::mode(PINA0, OUTPUT);  // принудительная блокировка модуля защиты
#endif
#if (FAN)
  gio::mode(PWMKUL, OUTPUT);  // вентилятор
  // Пины D9 и D10 - 30 Гц для вентилятора
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00000101;  // x1024 phase correct
#endif
#if (VOLTIN == 1)
  gio::mode(POWIN, INPUT);  // пин замера напряжения от БП
    // настройка АЦП
  analogReference_my(DEFAULT);  // Установка напр реф /INTERNAL
  analogPrescaler_my(16);       // установка количества выборки АЦП - 2,4,8,16,32,64,128
#endif

#if (LOGGER)
  Serial.begin(115200);  // 9600   115200
#endif

  lcd.init();                                       // Инициализация дисплея
  lcd.backlight();                                  // Подключение подсветки
  if (EEPROM.read(0) != MEMVER) Reset_settings(2);  // сброс настроек
  disp.mysimbol();                                  // загрузка в память дисплея своих символов(состояние акб)

  // Приветствие. Версия прошивки и тип контроллера
  lcd.setCursor(trunc((DISPLAYx - 7) / 2), DISPLAYy / 2 - 1);
  lcd.print(F(txt_ViktoRi));                                // *      ViktoRi       *
  lcd.setCursor(trunc((DISPLAYx - 12) / 2), DISPLAYy / 2);  // *   Ver 1.0-328p     *
  lcd.print(F(VER));

  uint32_t t = millis();  // засекаем время
  Speaker(500);           // функция звукового сигнала (время в миллисекундах)
#if (FAN)  
  gio::high(PWMKUL);      // запуск вентилятора, проверка работоспособности
#endif
#if (SENSTEMP1 == 1)
  tr_Q1.online();  // опросить датчик температуры
  if (tr_Q1.online()) {
    // проверка DS18B20 датчик температуры
    tr_Q1.setResolution(9);  // Установить разрешение термометра 9-12 бит, разрешающей способность - 0,5 (1/2) °C, 0,25 (1/4) °C, 0,125 (1/8) °C и 0,0625 (1/16) °C
    tr_Q1.requestTemp();     // Запросить новое преобразование температуры
  } else {
    lcd.clear();
    lcd.print(F(txt7));  // "DS18B20 "
    print_mode(2);       // "error"
    Delay(1500);
  }
#endif
  // Ожидание 3 секунды в течении которых удержанием кнопки Стоп или энкодера можно сбросить настройки по умолчанию
  while (millis() - t <= 3000) {
    butt.Tick();
    if (butt.tick == STOPHELD or butt.tick == ENCHELD) {
#if (FAN)
     gio::low(PWMKUL);// остановка вентилятора
#endif
      Reset_settings(0);  // сброс настроек
    }
#if (VOLTIN == 1)
    analogRead_my(POWIN);  // замер напряжения БП
#endif
  }
#if (FAN)  
  gio::low(PWMKUL);// остановка вентилятора
#endif
  EEPROM.get(1, vkr);                                      // читаю из памяти значения
  EEPROM.get((int)(sizeof(pam) * vkr.profil + MEM), pam);  // читаю из памяти значения
  if (pam.Mode > POINTMODE) Reset_settings(1);             // сброс настроек

  // конфигурация INA226
  if (ina.begin()) ina.start(1);  // старт замеров INA
  else ina.error();

#if (MCP4725DAC)
  if (!dac.testConnection()) {
    lcd.clear();
    lcd.print(F("MCP4725"));
    print_mode(2);  // "error"
    Delay(1500);
  }
#endif
#if (VOLTIN == 1)
  vin.start();  // старт замеров напряжения от БП
#endif
#if (FAN)
  kul.begin();  // инициализация кулера
#endif
  if (bitRead(vkr.GlobFlag, TRQ1)) Q1_broken();
  // если заряд не был завершен корректно
  if (bitRead(pam.MyFlag, CHARGE)) {
    lcd.clear();
    print_mode(0);
    bitWrite(pam.MyFlag, CHARGE, (Choice(10, true)));
    lcd.clear();
  }
#if (TIME_LIGHT)
  disp.Light_setTime();  // установить время подсветки дисплея если оно больше 0
#endif
  Wire.setClock(WIRECLOCK);  // частота I2C в герцах // влияет на INA226 и на дисплей
}
// setup


void loop() {
  Menu1602();    // заряд отключен. Меню v4.0.
  Operations();  // заряд включен. Работа
}
// Конец программы
