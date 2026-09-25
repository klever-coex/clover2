# Настройка радиоаппаратуры

```{tip}
В статье описана подготовка аппаратуры **RadioMaster Pocket ELRS** и сопряжение с приёмником ExpressLRS.
```

```{caution}
Перед настройкой снимите пропеллеры с квадрокоптера. Это исключит случайный запуск моторов при проверке каналов и переключателей.
```

## Содержание

```{contents}
:local:
:depth: 2
```

## Обзор аппаратуры

```{figure} @assets@/ru/setup/radio/rc-pocket-front-part.webp
:alt: Аппаратура RadioMaster Pocket, вид спереди
:width: 90%
:align: center

Аппаратура RadioMaster Pocket, вид спереди
```

```{figure} @assets@/ru/setup/radio/rc-pocket-back-part.webp
:alt: Аппаратура RadioMaster Pocket, вид сзади
:width: 90%
:align: center

Аппаратура RadioMaster Pocket, вид сзади
```

## Подготовка аппаратуры

1. Снимите чёрные резиновые накладки с задней стороны аппаратуры.
2. Вставьте аккумулятор, соблюдая полярность.
3. Установите резиновые накладки обратно.
4. Включите аппаратуру, нажав и удерживая кнопку **POWER**.

```{caution}
Если на экране появляются предупреждения, проверьте положение тумблеров и стика газа (**Throttle**). Перед включением они должны находиться в нулевом положении.
```

```{figure} @assets@/common/setup/radio/warn.webp
:alt: Предупреждение о положении стиков или тумблеров
:width: 55%
:align: center

Предупреждение о положении стиков или тумблеров
```

После установки тумблеров и стиков в нулевое положение откроется главное меню.

```{figure} @assets@/common/setup/radio/main-menu-no-signal.webp
:alt: Главное меню аппаратуры без связи с приёмником
:width: 55%
:align: center

Главное меню аппаратуры без связи с приёмником
```

## Настройка модели для Betaflight

```{note}
Если модель «Клевер 5 ФПВ» уже настроена, проверьте режим внутреннего радиомодуля и назначение каналов в **MIXES**, затем переходите к сопряжению.
```

Модель можно создать вручную или загрузить готовый файл.

### Создание модели вручную

1. Нажмите кнопку **MDL**.
2. Выберите свободную ячейку и нажмите и удерживайте колесо-энкодер.

    ```{figure} @assets@/common/setup/radio/model/create-model-menu.webp
    :alt: Выбор свободной ячейки модели
    :width: 450px
    :align: center
    ```

3. Выберите **CREATE MODEL**.
4. Нажмите **PAGE >**, колесом-энкодером выберите **Model name** и задайте имя модели, например `CLOVER FPV`.

    ```{figure} @assets@/common/setup/radio/model/model-name.webp
    :alt: Настройка имени модели квадрокоптера
    :width: 450px
    :align: center
    ```

5. Нажмите **RTN**, прокрутите страницу вниз и в разделе **Internal RF** установите **Mode** в значение **CRSF**.

    ```{figure} @assets@/common/setup/radio/model/internal-rf-disabled.webp
    :alt: Выключенный внутренний радиомодуль
    :width: 450px
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/model/internal-rf-crsf.webp
    :alt: Внутренний радиомодуль в режиме CRSF
    :width: 450px
    :align: center
    ```

6. Перейдите на страницу **MIXES** и назначьте переключатели дополнительным каналам:

   - **CH5 / AUX1** — **SA**;
   - **CH6 / AUX2** — **SB**;
   - **CH7 / AUX3** — **SC**;
   - **CH8 / AUX4** — **SD**.

    ```{figure} @assets@/common/setup/radio/model/mix-channel-settings.webp
    :alt: Настройка переключателя на дополнительном канале
    :width: 450px
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/model/mixes-channels.webp
    :alt: Основные и дополнительные каналы модели
    :width: 450px
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/model/mixes-switches.webp
    :alt: Список настроенных переключателей
    :width: 450px
    :align: center
    ```

7. Нажмите **RTN**. После сопряжения проверьте в Betaflight Configurator, что стики и переключатели изменяют соответствующие каналы во вкладке **Receiver**.

### Загрузка готовой модели

Вместо ручной настройки можно загрузить подготовленный профиль:

1. Скачайте файл модели [`model05.yml`](https://drive.google.com/file/d/19Ab9sWPvaOvIavkEXPXzDXCRajpBcO7e/view?usp=sharing).
2. Включите аппаратуру, подключите кабель USB Type-C к верхнему USB-порту и выберите **USB Storage (SD)**.
3. Откройте накопитель аппаратуры и перейдите в каталог **MODELS**.

    ```{figure} @assets@/common/setup/radio/model/usb-storage-models-folder.webp
    :alt: Каталог MODELS на карте памяти аппаратуры
    :width: 600px
    :align: center
    ```

4. Скопируйте в каталог **MODELS** файл `model05.yml`.

    ```{figure} @assets@/common/setup/radio/model/model-file-copied.webp
    :alt: Файл готовой модели в каталоге MODELS
    :width: 600px
    :align: center
    ```

5. Безопасно извлеките USB-накопитель, отключите кабель и выберите загруженную модель в меню **MDL**.
6. Проверьте, что **Internal RF** работает в режиме **CRSF**, а переключатели назначены каналам **CH5–CH8**.

(clover5-fpv-binding-phrase)=

## Сопряжение с помощью Binding Phrase

Binding Phrase должна быть совершенно одинаковой на передатчике и приёмнике. Этот способ поддерживается через Web UI в ExpressLRS 3.0 и новее. Если на одном из устройств установлена более ранняя версия, сначала обновите его через [ExpressLRS Web Flasher](https://expresslrs.github.io/web-flasher/).

### Настройка передатчика

1. Включите аппаратуру и нажмите **SYS**.
2. Откройте инструмент **ExpressLRS**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Инструмент ExpressLRS в системном меню
    :width: 450px
    :align: center
    ```

3. Выберите **WiFi Connectivity**, затем **Enable WiFi** и дождитесь сообщения **WiFi Running**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-connectivity-menu.webp
    :alt: Пункт WiFi Connectivity в ExpressLRS
    :width: 450px
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-enable-wifi.webp
    :alt: Включение Wi-Fi передатчика ExpressLRS
    :width: 450px
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-running.webp
    :alt: Передатчик ExpressLRS в режиме Wi-Fi
    :width: 450px
    :align: center
    ```

4. Подключите компьютер или смартфон к сети **ExpressLRS TX**. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS TX
    :width: 450px
    :align: center
    ```

5. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-web-interface.webp
    :alt: Веб-интерфейс передатчика ExpressLRS
    :width: 450px
    :align: center
    ```

6. В разделе **Binding Phrase** введите выбранную фразу и нажмите **SAVE**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-binding-phrase.webp
    :alt: Binding Phrase в настройках передатчика
    :width: 700px
    :align: center
    ```

7. Подтвердите перезагрузку кнопкой **REBOOT**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Перезагрузка устройства ExpressLRS
    :width: 450px
    :align: center
    ```

### Настройка приёмника

```{caution}
Перед длительной подачей питания подключите антенну к видеопередатчику и обеспечьте его охлаждение либо временно отключите VTX от полётного контроллера. Не устанавливайте пропеллеры.
```

1. Выключите аппаратуру управления, чтобы приёмник не установил соединение с передатчиком.
2. Подайте питание на полётный контроллер и подождите не менее 60 секунд. Когда приёмник перейдёт в режим Wi‑Fi, его светодиод начнёт быстро мигать.
3. Подключите компьютер или смартфон к сети **ExpressLRS RX**. Пароль по умолчанию — `expresslrs`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS RX
    :width: 450px
    :align: center
    ```

4. Откройте в браузере адрес `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-web-interface.webp
    :alt: Веб-интерфейс приёмника ExpressLRS
    :width: 450px
    :align: center
    ```

5. Введите точно такую же **Binding Phrase**, как на передатчике, и нажмите **SAVE & REBOOT**.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-binding-phrase.webp
    :alt: Binding Phrase в настройках приёмника
    :width: 700px
    :align: center
    ```

6. Если появится окно подтверждения, нажмите **REBOOT** и дождитесь перезагрузки приёмника.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Подтверждение перезагрузки приёмника
    :width: 450px
    :align: center
    ```

7. Включите аппаратуру. После установления связи на главном экране появится индикатор приёмника.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после сопряжения с приёмником
    :width: 55%
    :align: center
    ```

## Сопряжение в режиме Bind

Если настроить Binding Phrase невозможно, используйте ручной режим Bind:

1. Трижды подряд включите и отключите питание полётного контроллера. После перехода в режим сопряжения светодиод приёмника начнёт мигать двойными импульсами с паузой.
2. На аппаратуре нажмите **SYS** и откройте инструмент **ExpressLRS**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Переход в меню ExpressLRS
    :width: 55%
    :align: center
    ```

3. Установите **TX Power** в значение **100 mW**.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-2.webp
    :alt: Мощность передатчика 100 mW
    :width: 55%
    :align: center
    ```

4. Нажмите **Bind** и дождитесь завершения сопряжения.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-3.webp
    :alt: Запуск сопряжения ExpressLRS
    :width: 55%
    :align: center
    ```

    ```{figure} @assets@/common/setup/radio/rx/elrs-bind.webp
    :alt: Процесс сопряжения ExpressLRS
    :width: 55%
    :align: center
    ```

5. Нажмите и удерживайте **RTN**. После выхода в главное меню должен появиться индикатор связи с приёмником.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после успешного сопряжения
    :width: 55%
    :align: center
    ```

После сопряжения откройте вкладку **Receiver** в Betaflight Configurator и проверьте работу стиков и всех назначенных переключателей.

Подробнее о способах сопряжения см. в [документации ExpressLRS](https://www.expresslrs.org/quick-start/binding/).
