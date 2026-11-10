# Подключение к Wi-Fi

Для настройки Wi-Fi на Raspberry Pi используйте текстовый интерфейс `nmtui`. В режиме клиента Raspberry Pi подключается к существующей сети. В режиме точки доступа Raspberry Pi создает свою сеть.

Откройте {doc}`web терминал <DroneConnection>` Raspberry Pi или подключитесь по {doc}`SSH <DroneConnection>`. Запустите `nmtui`:

```bash
sudo nmtui
```

## Изменение имени сети и пароля точки доступа

1. Выберите `Edit a connection`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/main-menu.webp
    :alt: Главное меню nmtui
    :width: 600px
    :align: center

    Главное меню nmtui
    ```

    <br>

2. Выберите профиль точки доступа и нажмите `Edit`. Базовый профиль называется `clover2`. Если профиля нет, выберите `Add` → `Wi-Fi` и установите `Mode` в `Access Point`, а `IPv4 CONFIGURATION` — в `Shared`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-access-point.webp
    :alt: Выбор профиля точки доступа
    :width: 600px
    :align: center

    Выбор профиля точки доступа
    ```

    <br>

3. Введите имя сети в поле `SSID` и пароль длиной не менее восьми символов в поле `Password`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/edit-access-point.webp
    :alt: Изменение имени сети и пароля точки доступа
    :width: 600px
    :align: center

    Изменение имени сети и пароля точки доступа
    ```

    <br>

4. Нажмите `OK`, затем `Back` и `Quit`.
5. Перезапустите NetworkManager:

    ```bash
    sudo systemctl restart NetworkManager
    ```

## Переключение в режим клиента

1. Запустите `sudo nmtui` и выберите `Edit a connection`.
2. Нажмите `Add`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/add-connection.webp
    :alt: Добавление профиля подключения
    :width: 600px
    :align: center

    Добавление профиля подключения
    ```

    <br>

3. В окне `New Connection` выберите `Wi-Fi`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-wifi.webp
    :alt: Выбор типа подключения
    :width: 600px
    :align: center

    Выбор типа подключения
    ```

    <br>

4. Перейдите в поле `Security` и выберите тип защиты вашей сети, например `WPA & WPA2 Personal`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-security.webp
    :alt: Выбор типа защиты
    :width: 600px
    :align: center

    Выбор типа защиты
    ```

    <br>

5. Введите имя сети в поле `SSID` и пароль в поле `Password`. Нажмите `OK`, затем `Back`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/enter-network-credentials.webp
    :alt: Ввод имени сети и пароля
    :width: 600px
    :align: center

    Ввод имени сети и пароля
    ```

    <br>

6. Выберите `Activate a connection`. Найдите созданный профиль (на снимке `Wi-Fi connection 1`) и нажмите `Activate`.

    Если созданного профиля нет в списке, выйдите из `nmtui` и перезапустите NetworkManager:

    ```bash
    sudo systemctl restart NetworkManager
    ```

    Снова запустите `sudo nmtui`, выберите `Activate a connection` и активируйте профиль.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/activate-client.webp
    :alt: Активация профиля клиента
    :width: 600px
    :align: center

    Активация профиля клиента
    ```

    <br>

## Переключение в режим точки доступа

1. Запустите `sudo nmtui` и выберите `Activate a connection`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/open-connection-list.webp
    :alt: Переход к списку подключений
    :width: 600px
    :align: center

    Переход к списку подключений
    ```

    <br>

2. Выберите активный профиль клиента и нажмите `Deactivate`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/deactivate-client.webp
    :alt: Отключение профиля клиента
    :width: 600px
    :align: center

    Отключение профиля клиента
    ```

    <br>

3. Если точка доступа не запустилась автоматически, выберите ее профиль и нажмите `Activate`.

## Профили NetworkManager

NetworkManager хранит профили в каталоге `/etc/NetworkManager/system-connections/`. Для изменения профилей используйте `nmtui`.

Если интерфейс `wlan0` настроен через NetworkManager, не настраивайте его одновременно через `dhcpcd.conf`, `wpa_supplicant.conf` или отдельный файл конфигурации `dnsmasq`.
