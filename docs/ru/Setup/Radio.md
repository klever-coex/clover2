# Настройка аппаратуры управления

В ручном режиме вы можете управлять конструктором квадрокоптера с помощью аппаратуры радиоуправления, подключенной к полетному контроллеру через приемник.

```{tip}
В статье описана подготовка аппаратуры **RadioMaster Pocket ELRS** и сопряжение с приемником ExpressLRS.
```

```{caution}
Перед настройкой снимите пропеллеры. Это исключит случайный запуск электродвигателей при проверке каналов и переключателей.
```

## Обзор аппаратуры

```{figure} @assets@/ru/setup/radio/rc-pocket-front-part.webp
:alt: Аппаратура управления, вид спереди
:width: 90%
:align: center

Аппаратура управления, вид спереди
```
```{figure} @assets@/ru/setup/radio/rc-pocket-back-part.webp
:alt: Аппаратура управления, вид сзади
:width: 90%
:align: center

Аппаратура управления, вид сзади
```
## Подготовка аппаратуры управления

1. Снимите черные резиновые накладки с задней стороны аппаратуры управления.
2. Установите аккумулятор. Соблюдайте полярность при установке.
3. Установите резиновые накладки обратно.
4. Включите аппаратуру, нажав и удерживая кнопку **POWER**.

```{caution}
Если на экране аппаратуры отображается предупреждение WARNING, установите стик газа (Throttle) в нижнее положение. 
```

```{figure} @assets@/common/setup/radio/warn.webp
:alt: Предупреждение при неверном положении стиков или тумблеров
:width: 55%
:align: center

Предупреждение при неверном положении стиков
```
5. После устранения предупреждения перейдите в главное меню аппаратуры.

```{figure} @assets@/common/setup/radio/main-menu-no-signal.webp
:alt: Главное меню аппаратуры без связи с приёмником
:width: 55%
:align: center

Главное меню аппаратуры без связи с приемником
```

## Сопряжение аппаратуры управления с приемником

### Настройка передатчика

1. Нажмите кнопку **SYS**.
2. Откройте меню **ExpressLRS**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Инструмент ExpressLRS в системном меню
    :width: 55%
    :align: center
    Инструмент ExpressLRS в системном меню
    ```

3. Выберите **WiFi Connectivity**.
  
    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-connectivity-menu.webp
    :alt: Пункт WiFi Connectivity в ExpressLRS
    :width: 450px
    :align: center
   Пункт WiFi Connectivity в ExpressLRS 
    ```

4. Выберите **Enable WiFi**.
    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-enable-wifi.webp
    :alt: Включение Wi-Fi передатчика ExpressLRS
    :width: 450px
    :align: center
    Включение Wi-Fi передатчика ExpressLRS
    ```
5. Дождитесь сообщения **WiFi Running**.
    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-running.webp
    :alt: Передатчик ExpressLRS в режиме Wi-Fi
    :width: 450px
    :align: center
    Передатчик ExpressLRS в режиме Wi-Fi
    ```

6. Подключите компьютер или смартфон к сети **ExpressLRS TX**. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS TX
    :width: 450px
    :align: center
    Подключение к сети ExpressLRS TX
    ```

7. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-web-interface.webp
    :alt: Веб-интерфейс передатчика ExpressLRS
    :width: 450px
    :align: center
    Веб-интерфейс передатчика ExpressLRS
    ```

8. В строке **Binding Phrase** введите выбранную фразу и нажмите **SAVE**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-binding-phrase.webp
    :alt: Binding Phrase в настройках передатчика
    :width: 700px
    :align: center
    Binding Phrase в настройках передатчика
    ```

9. Подтвердите перезагрузку кнопкой **REBOOT**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Перезагрузка устройства ExpressLRS
    :width: 450px
    :align: center
    Перезагрузка устройства ExpressLRS
    ```

### Настройка приемника

```{caution}
Если вместе с полетным контроллером получает питание видеопередатчик, подключите к нему антенну и обеспечьте охлаждение либо временно отключите VTX. Не устанавливайте пропеллеры.
```

1. Выключите аппаратуру управления, чтобы приемник не установил соединение с передатчиком.
2. Подайте питание на полетный контроллер и подождите не менее 60 секунд. Когда приемник перейдет в режим Wi‑Fi, его светодиод начнет быстро мигать.
3. Подключите компьютер или смартфон к сети **ExpressLRS RX**. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS RX
    :width: 450px
    :align: center
    Подключение к сети ExpressLRS RX
    ```

4. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-web-interface.webp
    :alt: Веб-интерфейс приёмника ExpressLRS
    :width: 450px
    :align: center
    Веб-интерфейс приемника ExpressLRS
    ```

5. Введите фразу в строке **Binding Phrase**, как на передатчике. Нажмите **SAVE & REBOOT**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-binding-phrase.webp
    :alt: Binding Phrase в настройках приёмника
    :width: 700px
    :align: center
    Binding Phrase в настройках приемника
    ```

6. При появлении окна подтверждения нажмите **REBOOT** и дождитесь перезагрузки приемника.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Подтверждение перезагрузки приёмника
    :width: 450px
    :align: center
    Подтверждение перезагрузки приемника
    ```

7. Включите аппаратуру управления. После установления связи на главном экране появится индикатор приемника.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после сопряжения с приёмником
    :width: 55%
    :align: center
    Главное меню после сопряжения с приемником
    ```

### Альтернативный способ сопряжения: режим Bind

Если настроить Binding Phrase невозможно, используйте ручной режим Bind:

1. Переведите приемник ELRS в режим сопряжения (bind). Для этого трижды подключите и отключите питание полетного контроллера. Убедитесь, что светодиод приемника начал мигать с установленной индикацией (дважды с паузой).

2. На аппаратуре нажмите **SYS** и откройте меню **ExpressLRS**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Переход в меню ExpressLRS
    :width: 55%
    :align: center
    Меню ExpressLRS
    ```

3. Установите для **TX Power**  значение **100 mW**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-2.webp
    :alt: Мощность передатчика 100 mW
    :width: 55%
    :align: center
    Установка мощности передатчика TX Power
    ```

4. Выберите команду **Bind**. Дождитесь завершения сопряжения аппаратуры с приемником.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-3.webp
    :alt: Запуск сопряжения ExpressLRS
    :width: 55%
    :align: center
    Меню ExpressLRS
    ```

5. После успешного сопряжения убедитесь, что на экране отображается состояние соединения. 
    ```{figure} @assets@/common/setup/radio/rx/elrs-bind.webp
    :alt: Процесс сопряжения ExpressLRS
    :width: 55%
    :align: center
    Экран состояния после успешного сопряжения
    ```

6. Нажмите и удерживайте **RTN** для возврата в основное меню. 
Убедитесь, что на основном экране отображается индикатор связи с приемником. 

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после успешного сопряжения
    :width: 55%
    :align: center
    Индикация связи с приемником на основном экране аппаратуры
    ```

