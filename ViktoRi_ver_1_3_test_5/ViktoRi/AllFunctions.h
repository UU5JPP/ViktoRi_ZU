void Storage(bool regim);
int32_t aw_ch(int32_t i);
void Res_mem_char(void);
void Res_mem_dischar(void);
void Res_ktc(void);
void Delay(uint16_t t);
void pauses(void);
uint16_t Volt_DCur(void);
void Fixcurrent(int16_t amp);
void Korrect(const uint8_t kof);
void Wait(uint16_t time_charge);
void End(void);
bool Choice(uint8_t secund, bool rez);
int8_t myTick(void);
void Speaker(uint16_t n);
void ChargeTimeCalc(void);
uint8_t Percentage(uint16_t volt_max, uint16_t volt, int16_t current, int16_t curr_min);
void Menu1602();
void Operations();
void ChargeAkb(uint16_t volt_charge_init, int16_t curr_charge_init, int16_t curr_min, uint32_t time_charge, bool add_charge);
void PrintVA(uint16_t volt, int16_t current, int32_t Ah, int32_t Wh, uint8_t x);
void pr_tm(uint16_t tm);
void Print_time(uint32_t time_real, uint8_t x, uint8_t y);
void setCursorx(void);
void setCursory(void);
void printSimb(void);
void print_tr(int8_t tr);
void PrintOnOff(bool i);
void Write_x(bool x);
void print_mode(uint8_t i);
void print_Capacity(void);
void printCykl(void);
void Vout(void);
void ClearStr(uint8_t i);
bool Q1_broken();
void Saved();
void Reset_settings(uint8_t sett);
void Freq(const uint8_t frq);
void Settings_point(uint8_t point, bool edit, int8_t x);
void Prstatistic(int i, uint32_t time_i, int32_t Ah, int32_t Wh);
void Statistics_point(uint8_t point);
void calculate(uint8_t i);


#if (POWPIN == 1)
void Relay(void);
#endif
#if (GUARDA0)
void Guard(void);
#endif
#if (SENSTEMP2 == 2)
bool Control_trAkb();
#endif
#if (LOGGER == 1)
void Serial_out(float volt, float amperSred, uint8_t tQ1, float volt_in, float Achas, uint8_t tAkb);
#endif
#if (LOGGER == 2)
void Serial_out(float volt, float amperSred, uint8_t tQ1, float Achas);
#endif
#if (DISCHAR == 1)
void Discharge();
#endif
#if (RESIST == 1)
void Resist();
#endif
#if ((DISPLAYx == 16 and DISPLAYy == 2) or (DISPLAYx == 20 and DISPLAYy == 2))
void Display_print(int32_t Ah, uint32_t time_real, uint8_t prc, bool add);
#endif
#if (DISPLAYx == 20 and DISPLAYy == 4)
void Display_print(int32_t Ah, int32_t Wh, uint32_t time_real, int8_t tr, uint8_t prc);
#endif
#if (DISPLAYx == 16 and DISPLAYy == 4)
void Display_print(int32_t Ah, int32_t Wh, uint32_t time_real, int8_t tr, uint8_t prc);
#endif
#if (SERVICE == 1)
void serviceparam(void);
#endif



struct VA_var;
//class Enbutt;