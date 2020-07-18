# rotator
Достаточно задать угол на который должна повернуться антенна и нажать кнопку "Старт". Оба органа управления совмещены в енкодере с числом импульсов 24 на оборот и кнопкой на той же оси. При включении прибора на экране отражается угол Az (градус) куда смотрит антенна сейчас и предустановка (Pres) в 180 градусов. Значение 180 градусов выбрано не случайно. Теоретически это середина и в большинстве случаев от этой точки до желаемой (в пределах 360-ти гардусов) расстояние (время) минимальное.   Но мотор (антенна) не крутится ожидая установки.  Вращая енкодер, устанавливаем направление куда должна повернуться антенна -  значение 180 измениться на то что нам надо, например, 250 градусов. При нажатии кнопки энкодера Ардуино определит в какую сторону нужно крутить, включит соответствующее реле и покажет принятое значение направлениz "Set", стрелку показывающую что двигатель включен и куда  поворачивается антенна. Когдла антенна повернётся, компаратор выключит двигатель и "погасит" стрелку и Set.

В качестве датчика угла поворота, использую это:
[P3022-V1-CW360](https://wiki.dfrobot.com/Gravity__Hall_Angle_Sensor_SKU__SEN0221)
Абсолютный энкодер P3022-V1-CW360 выход 0-5 В
Specification
Mechanical Angle: 0-360° (No stop)
Operating Voltage: 5V ± 10%
Operating Current: <13.5mA
Output Signal: 0-5V DC (Ratio)
Resolution: 0.088° (12-bit ADC)
Accuracy: ± 0.3% FS
Output Shaft Dimension: 6 mm (D Sharp)
Rotational Torque: <5mN·m
Refresh Rate: 0.6ms / 0.2ms (high speed)
Operating Temperature: -30 ℃ ~ 80 ℃
Allowable Axial Load: Horizontal <5N; Vertical <10N
Protection Class: IP40
Mechanical Life:> 50 million revolutions
Weight: 36g
Board Ove

```
void loop()
{
  float analogValue = analogRead(A2);     //Voltage reading
  float amp = analogValue/1024.0 * 360;  //Angle calculation  (  UNO is a 10-bit AD )
  Serial.print("Angle:");
  Serial.println(amp);
  delay(500);
}
```

[Оригинал этого девайса](https://hammania.net/index.php/shack-ham-soft/forum/prostoe-povorotnoe-ustrojstvo)