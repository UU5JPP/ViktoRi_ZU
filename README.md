<h2 align="center">Зарядное устройство для аккумуляторных батарей на микроконтроллере Ардуино Viktori</a> </h2>

<h2 align="center">Телеграмм  <a href="https://t.me/arduino_viktori/" target="_blank">чат</a> </h2>


Общие сведения.<br>
<p>Зарядное устройство ViktoRi является открытым проектом (Open source) со свободным исходным кодом и схемой. Основано на микроконтроллере Atmega 328P (Arduino Nano, UNO,…) , LGT8F328P. Предназначено для заряда аккумуляторных батарей (в дальнейшем — АКБ) свинцово-кислотных, AGM, GEL, литий-ионных, литий-титанатных, никель-кадмиевых напряжением 3 — 24 Вольт и емкостью 1 ... 255 Ач, применяемых на автомобилях, мотоциклах, катерах, скутерах, ИБП  и т. д.</p>

<p>Зарядное устройства ViktoRi имеет жидкокристаллический дисплей 16*2 либо 20*4, на котором отображаются текущие параметры заряда, настраиваемые параметры и информационные сообщения. Для управления применяется Энкодер и кнопки.</p><br>
<p>Зарядное устройство реализует несколько алгоритмов заряда, включающий: 
  - предварительный заряд сильно разряженного аккумулятора;
  - основной заряд комбинированным методом (стабилизация тока на этапе основного заряда с переключением в режим стабилизации напряжения на конечном этапе); 
  - режим качели в конце основного заряда; 
  - дозаряд батареи;  режим безопасного хранения батареи с поддержанием заряда; Имеется режим разряда, измерения внутреннего сопротивления, проведение контрольно-тренировочных циклов (в дальнейшем КТЦ), заряд аккумулятора по методике Бранимира (заряд малыми токами).
Во время работы зарядное устройство контролирует и регулирует напряжение на АКБ, ток заряда/разряда. Рассчитываются и сохраняются в память Ампер/часы, Ватт/часы.</p>
