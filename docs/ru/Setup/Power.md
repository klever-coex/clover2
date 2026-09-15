# Настройка питания

1. Откройте в QGroundControl вкладку *Vehicle Configuration* 
2. Выберите меню *Power*.
3. Подключите аккумуляторную батарею.
4. Установите параметр *Number of cells* = **6S**.
5. Подключите индикатор напряжения к балансировочному разъему аккумуляторной батареи.
6. Нажмите *Calculate* напротив параметра *Voltage Divider*.
7. Введите полученное значение в открывшееся поле.
8. Нажмите *Close*, чтобы сохранить рассчитанное значение.

Если индикатора напряжения нет или ручная калибровка невозможна, используйте усредненное значение `Voltage divider = 21`.

```{figure} ../../assets/common/setup/qgc-voltage-divider.webp
:alt: Калибровка делителя напряжения в QGroundControl
:width: 90%
:align: center

Окно настройки питания
```
<br>

Дополнительная информация: [QGroundControl Power Setup](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/setup_view/power.html).
