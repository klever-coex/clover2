# Настройка радиоаппаратуры

В статье описана подготовка аппаратуры **RadioMaster Pocket ELRS** и сопряжение с приемником **ExpressLRS**.

```{caution}
Перед настройкой снимите пропеллеры. Это исключит случайный запуск электродвигателей при проверке каналов и переключателей.
```

## Обзор аппаратуры

```{figure} @assets@/ru/setup/radio/rc-pocket-front-part.webp
:alt: Аппаратура RadioMaster Pocket, вид спереди
:width: 90%
:align: center

Аппаратура управления, вид спереди
```

<br>

```{figure} @assets@/ru/setup/radio/rc-pocket-back-part.webp
:alt: Аппаратура RadioMaster Pocket, вид сзади
:width: 90%
:align: center

Аппаратура управления, вид сзади
```

<br>

## Подготовка аппаратуры

1. Снимите черные резиновые накладки с задней стороны аппаратуры управления.
2. Установите аккумулятор в отсек аппаратуры. Соблюдайте полярность при установке.
3. Установите резиновые накладки обратно.
4. Включите аппаратуру, нажав и удерживая кнопку `POWER`.
5. Если на экране аппаратуры отображается предупреждение `WARNING`, установите стики и тумблеры в нулевое положение. Установите стик газа (`Throttle`) в нижнее положение. Установите тумблеры в положения, соответствующие нулевым значениям.


```{figure} @assets@/common/setup/radio/warn.webp
:alt: Предупреждение о положении стиков или тумблеров
:width: 55%
:align: center

Предупреждение при неверном положении стиков или тумблеров
```

<br>

6. После устранения предупреждения перейдите в главное меню аппаратуры.

```{figure} @assets@/common/setup/radio/main-menu-no-signal.webp
:alt: Главное меню аппаратуры без связи с приемником
:width: 55%
:align: center
Главное меню аппаратуры без связи с приемником
```

<br>

## Настройка модели для Betaflight

```{note}
Если модель «Клевер 5 ФПВ» уже настроена, проверьте режим внутреннего радиомодуля и назначение каналов в **MIXES**, затем переходите к сопряжению.
```

Модель можно создать вручную или загрузить готовый файл.

### Создание модели вручную

1. Нажмите кнопку `MDL`.
2. Выберите свободную ячейку и нажмите и удерживайте колесо-энкодер.

    ```{figure} @assets@/common/setup/radio/model/create-model-menu.webp
    :alt: Выбор свободной ячейки модели
    :width: 450px
    :align: center
    Выбор свободной ячейки модели
    ```

    <br>

3. Выберите `CREATE MODEL`.
4. Нажмите `PAGE`, колесом-энкодером выберите `Model name` и задайте имя модели, например `CLOVER FPV`.

    ```{figure} @assets@/common/setup/radio/model/model-name.webp
    :alt: Настройка имени модели квадрокоптера
    :width: 450px
    :align: center
    Настройка имени модели квадрокоптера
    ```

    <br>

5. Нажмите `RTN`, прокрутите список вниз. В разделе `Internal RF` установите `Mode` в значение `CRSF`.

    ```{figure} @assets@/common/setup/radio/model/internal-rf-disabled.webp
    :alt: Выключенный внутренний радиомодуль
    :width: 450px
    :align: center
    Выключенный внутренний радиомодуль
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/model/internal-rf-crsf.webp
    :alt: Внутренний радиомодуль в режиме CRSF
    :width: 450px
    :align: center
    Внутренний радиомодуль в режиме CRSF
    ```

    <br>

6. Перейдите на страницу `MIXES` и назначьте переключатели дополнительным каналам:

   - `CH5 / AUX1` — `SA`;
   - `CH6 / AUX2` — `SB`;
   - `CH7 / AUX3` — `SC`;
   - `CH8 / AUX4` — `SD`.

    ```{figure} @assets@/common/setup/radio/model/mix-channel-settings.webp
    :alt: Настройка переключателя на дополнительном канале
    :width: 450px
    :align: center
    Настройка переключателя на дополнительном канале
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/model/mixes-channels.webp
    :alt: Основные и дополнительные каналы модели
    :width: 450px
    :align: center
    Основные и дополнительные каналы модели
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/model/mixes-switches.webp
    :alt: Список настроенных переключателей
    :width: 450px
    :align: center
    Список настроенных переключателей
    ```

    <br>

7. Нажмите `RTN`. После сопряжения проверьте в **Betaflight Configurator**, что стики и переключатели изменяют соответствующие каналы во вкладке `Receiver`.

### Загрузка готовой модели

Вместо ручной настройки можно загрузить подготовленный профиль:

1. Скачайте файл модели [`model05.yml`](https://drive.google.com/file/d/19Ab9sWPvaOvIavkEXPXzDXCRajpBcO7e/view?usp=sharing).
2. Включите аппаратуру, подключите кабель USB-C к верхнему USB-порту и выберите `USB Storage (SD)`.
3. Откройте накопитель аппаратуры и перейдите в каталог `MODELS`.

    ```{figure} @assets@/common/setup/radio/model/usb-storage-models-folder.webp
    :alt: Каталог MODELS на карте памяти аппаратуры
    :width: 600px
    :align: center
    Каталог MODELS на карте памяти аппаратуры
    ```

    <br>

4. Скопируйте в каталог `MODELS` файл `model05.yml`.

    ```{figure} @assets@/common/setup/radio/model/model-file-copied.webp
    :alt: Файл готовой модели в каталоге MODELS
    :width: 600px
    :align: center
    Файл готовой модели в каталоге MODELS
    ```

    <br>

5. Безопасно извлеките USB-накопитель, отключите кабель и выберите загруженную модель в меню `MDL`.
6. Проверьте, что `Internal RF` работает в режиме `CRSF`, а переключатели назначены каналам `CH5–CH8`.

## Сопряжение с помощью Binding Phrase

**Binding Phrase** должна быть одинаковой на передатчике и приемнике. Этот способ поддерживается через Web UI в ExpressLRS 3.0 и новее. Если на одном из устройств установлена более ранняя версия, сначала обновите его через [ExpressLRS Web Flasher](https://expresslrs.github.io/web-flasher/).

### Настройка передатчика

1. Включите аппаратуру управления и нажмите `SYS`.
2. Откройте инструмент `ExpressLRS`.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Инструмент ExpressLRS в системном меню
    :width: 450px
    :align: center
    Инструмент ExpressLRS в системном меню
    ```

    <br>

3. Выберите `WiFi Connectivity`, затем `Enable WiFi` и дождитесь сообщения `WiFi Running`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-connectivity-menu.webp
    :alt: Пункт WiFi Connectivity в ExpressLRS
    :width: 450px
    :align: center
    Пункт WiFi Connectivity в ExpressLRS
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-enable-wifi.webp
    :alt: Включение Wi-Fi передатчика ExpressLRS
    :width: 450px
    :align: center
    Включение Wi-Fi передатчика ExpressLRS
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-running.webp
    :alt: Передатчик ExpressLRS в режиме Wi-Fi
    :width: 450px
    :align: center
     Передатчик ExpressLRS в режиме Wi-Fi
    ```

    <br>

4. Подключите компьютер или смартфон к сети `ExpressLRS TX`. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS TX
    :width: 450px
    :align: center
    Подключение к сети ExpressLRS TX
    ```

    <br>

5. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-web-interface.webp
    :alt: Веб-интерфейс передатчика ExpressLRS
    :width: 450px
    :align: center
    Веб-интерфейс передатчика ExpressLRS
    ```

    <br>

6. В разделе `Binding Phrase` введите выбранную фразу и нажмите `SAVE`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-binding-phrase.webp
    :alt: Binding Phrase в настройках передатчика
    :width: 700px
    :align: center
    ```

    <br>

7. Подтвердите перезагрузку кнопкой `REBOOT`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Перезагрузка устройства ExpressLRS
    :width: 450px
    :align: center
    Перезагрузка устройства ExpressLRS
    ```

    <br>

### Настройка приемника

```{caution}
Перед длительной подачей питания подключите антенну к видеопередатчику и обеспечьте его охлаждение, либо временно отключите VTX от полетного контроллера. Не устанавливайте пропеллеры.
```

1. Выключите аппаратуру управления, чтобы приемник не установил соединение с передатчиком.
2. Подайте питание на полетный контроллер и подождите не менее 60 секунд. Когда приемник перейдет в режим Wi‑Fi, его светодиод начнет быстро мигать.
3. Подключите компьютер или смартфон к сети `ExpressLRS RX`. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS RX
    :width: 450px
    :align: center
    Подключение к сети ExpressLRS RX
    ```

    <br>

4. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-web-interface.webp
    :alt: Веб-интерфейс приемника ExpressLRS
    :width: 450px
    :align: center
    Веб-интерфейс приемника ExpressLRS
    ```

    <br>

5. Введите фразу в строке `Binding Phrase`, как на передатчике.  Нажмите `SAVE & REBOOT`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-binding-phrase.webp
    :alt: Binding Phrase в настройках приемника
    :width: 700px
    :align: center
    Binding Phrase в настройках приемника
    ```

    <br>

6. При появлении окна подтверждения нажмите `REBOOT` и дождитесь перезагрузки приемника.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Подтверждение перезагрузки приемника
    :width: 450px
    :align: center
    Подтверждение перезагрузки приемника
    ```

    <br>

7. Включите аппаратуру. После установления связи на главном экране появится индикатор приемника.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после сопряжения с приёмником
    :width: 55%
    :align: center
    ```

    <br>

## Сопряжение в режиме Bind

Если настроить ***Binding Phrase*** невозможно, используйте ручной режим **Bind**:

1. Переведите приемник ELRS в режим сопряжения (bind). Для этого трижды подключите и отключите питание полетного контроллера. Убедитесь, что светодиод приемника начал мигать с установленной индикацией (дважды с паузой).

2. На аппаратуре нажмите `SYS` и откройте инструмент `ExpressLRS`.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Переход в меню ExpressLRS
    :width: 55%
    :align: center
    Переход в меню ExpressLRS
    ```

    <br>

3. Установите `TX Power` в значение `100 mW`.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-2.webp
    :alt: Мощность передатчика 100 mW
    :width: 55%
    :align: center
    Установка мощности передатчика TX Power
    ```

    <br>

4. Выберите команду `Bind` и дождитесь завершения сопряжения.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-3.webp
    :alt: Запуск сопряжения ExpressLRS
    :width: 55%
    :align: center
    Запуск сопряжения ExpressLRS
    ```

    <br>

    ```{figure} @assets@/common/setup/radio/rx/elrs-bind.webp
    :alt: Процесс сопряжения ExpressLRS
    :width: 55%
    :align: center
    Процесс сопряжения ExpressLRS
    ```

    <br>

5. Нажмите и удерживайте кнопку `RTN` для возврата в основное меню. Убедитесь, что на основном экране отображается индикатор связи с приемником. 

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после успешного сопряжения
    :width: 55%
    :align: center
    Индикация связи с приемником на основном экране аппаратуры
    ```

    <br>

После сопряжения откройте вкладку `Receiver` в **Betaflight Configurator** и проверьте работу стиков и всех назначенных переключателей.

Подробнее о способах сопряжения см. в [документации ExpressLRS](https://www.expresslrs.org/quick-start/binding/).
