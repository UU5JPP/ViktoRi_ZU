
 // функция если пробит силовой транзистор
void Q1_broken(void) { 
#if (TIME_LIGHT) 
  disp.Light_high();  // включение подсветки
#endif
  bitSet(vkr.GlobFlag, TRQ1);
  dcdc.Off();
#if (GUARDA0)
  gio::low(PINA0);  // разблокировать защитный модуль
#endif
#if (PROT == 1)
  gio::low(PROTECT); //закрыть защитный транзистор
#endif
#if (POWPIN == 1)
  gio::low(RELAY220);    // отключить сеть 220В
#endif

  Saved(); // сохранить настройки
  lcd.clear();
  lcd.print(F(txt24));  //"!!-BROKEN Q1-!!" 

  pauses();

/*  
  do {
    Speaker(1000);  // функция звукового сигнала (время в миллисекундах)
    Delay(10000); // ждать 10 секунд
  } while (!butt.tick);  // ожидание нажатия
*/
  
  bitClear(vkr.GlobFlag, TRQ1); // при нажатии записать в память что транзистор исправен
  Saved(); // сохранить настройки
#if (PROT == 1)
  gio::low(PROTECT); // открыть защитный транзистор Q4
#endif
#if (POWPIN == 1)
  gio::high(RELAY220);    // включить сеть 220В
#endif
#if (GUARDA0)
  Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
  lcd.clear();  
}
