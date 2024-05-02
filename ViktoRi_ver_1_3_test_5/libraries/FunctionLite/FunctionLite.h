#ifndef _FunctionLite_h
#define _FunctionLite_h
#include <Arduino.h>
void pinMode_my(uint8_t pin, uint8_t mode);      // установка режима работы пин
void digitalWrite_speed(uint8_t pin, bool x);    // вкл/откл цифрового пин
bool digitalRead_speed(uint8_t pin);             // чтение значения из регистра пин

/* Analog inputs functions */
void analogReference_my(uint8_t mode);           // установка референсного напряжения
void analogPrescaler_my(uint8_t prescaler);      // установка количества выборки АЦП 2,4,8,16,32,64,128
uint16_t analogRead_my(uint8_t pin);             // чтение аналогового значения напряжения

/* PWM function */
void analogWrite_my(uint8_t pin, uint16_t duty); // вкл/откл ШИМ
uint8_t RegRead_my(uint8_t pin);                 // чтение значения ШИМ из регистра пин
#endif
