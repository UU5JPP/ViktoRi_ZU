
 // функция если пробит силовой транзистор
bool Q1_broken() { 
#if (TIME_LIGHT) 
  disp.Light_high();  // включение подсветки
#endif
  bitWrite(vkr.GlobFlag, TRQ1, false);
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
  
  do {
    Speaker(1000);  // функция звукового сигнала (время в миллисекундах)
    Delay(10000); // ждать 10 секунд
  } while (!butt.tick);  // ожидание нажатия
  
  bitWrite(vkr.GlobFlag, TRQ1, Choice(60, false)); // при нажатии OK записать в память что транзистор исправен, // при нажатии стоп (чрез 60 секунд)записать в память что транзистор пробит
  Saved(); // сохранить настройки
#if (PROT == 1)
  if (BitIsClear(vkr.GlobFlag, TRQ1)) gio::low(PROTECT); // открыть защитный транзистор Q4
#endif
#if (POWPIN == 1)
  if (BitIsClear(vkr.GlobFlag, TRQ1)) gio::high(RELAY220);    // включить сеть 220В
#endif
#if (GUARDA0)
  Guard();  // принудительная блокировка модуля защиты при напряжении заряда или разряда акб менее 5 Вольт.
#endif
  lcd.clear();
  return bitRead(vkr.GlobFlag, TRQ1);
}
