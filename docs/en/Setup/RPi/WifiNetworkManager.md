# Wi-Fi Configuration Guide

To manage the Raspberry Pi's network settings, use the `nmtui` text-based interface. 

You can configure the device in two ways:
* Client Mode: The Raspberry Pi connects to an existing Wi-Fi network (like your home router).
* Access Point (AP) Mode: The Raspberry Pi creates its own Wi-Fi network for you to connect to.

To begin, open the Raspberry Pi {doc}`web terminal <DroneConnection>` or connect via {doc}`SSH <DroneConnection>`. Launch the configuration tool by running:

```bash
sudo nmtui
```

## How to Configure AP Settings

Use these steps to change the name (SSID) and password of the Wi-Fi network created by the Raspberry Pi.

1. Select `Edit a connection`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/main-menu.webp
    :alt: Главное меню nmtui
    :width: 600px
    :align: center

    nmtui main menu
    ```

    <br>

2. Select the access point profile and select `Edit`.

By default, this profile is named `clover2`. If you do not see it, select  `Add` → `Wi-Fi`. In the `Mode` field, select `Access Point`, and in the `IPv4 CONFIGURATION` section, select `Shared`.

  ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-access-point.webp
    :alt: Выбор профиля точки доступа
    :width: 600px
    :align: center

    Selecting AP
   ```

   <br>

5. Enter your desired network name in the `SSID` field and a password (minimum eight characters) in the  `Password` field.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/edit-access-point.webp
    :alt: Изменение имени сети и пароля точки доступа
    :width: 600px
    :align: center

    Editing AP
    ```

    <br>

6. Select `OK`, then `Back` and `Quit`.
7. Restart the NetworkManager to apply changes:

    ```bash
    sudo systemctl restart NetworkManager
    ```

```{important}
If you are currently connected to the Raspberry Pi via its Access Point, switching to Client Mode will turn off the AP and interrupt your SSH connection.
Ensure you have an alternative way to access the device (such as a physical connection or another network) before making this switch.
```

<br>

## How to Switch to Client Mode

Follow these steps to connect the Raspberry Pi to an external Wi-Fi network.

1. Run the `sudo nmtui` and select `Edit a connection`.
2. Select `Add`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/add-connection.webp
    :alt: Добавление профиля подключения
    :width: 600px
    :align: center

    Adding connection
    ```

    <br>

3. In the `New Connection` window, choose `Wi-Fi`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-wifi.webp
    :alt: Выбор типа подключения
    :width: 600px
    :align: center

   Selecting Wi-Fi
    ```

    <br>

4. Navigate to the `Security` field and select your network's security type (e.g. `WPA & WPA2 Personal`).

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/select-security.webp
    :alt: Выбор типа защиты
    :width: 600px
    :align: center

    Selecting security type
    ```

    <br>

5. Enter your network name in the `SSID` field and your password in the `Password` field. Select `OK`, then `Back`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/enter-network-credentials.webp
    :alt: Ввод имени сети и пароля
    :width: 600px
    :align: center

    Entering credentials
    ```

    <br>

6. Select `Activate a connection`. Locate your new profile in the list and select `Activate`.

   ```{note}
    The screenshot shows the profile Wi-Fi connection 1 as an example.
   ```

   If your new profile does not appear in the list, exit `nmtui` and restart the Network Manager:

    ```bash
    sudo systemctl restart NetworkManager
    ```

   Then, relaunch `sudo nmtui`, go to `Activate a connection`, and activate the profile.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/activate-client.webp
    :alt: Активация профиля клиента
    :width: 600px
    :align: center

    Activating client
    ```

```{note}
Once connected to a Wi-Fi network, the Raspberry Pi's IP address may change.
To reconnect via SSH, you will need to find the new IP address, which can typically be found in your router's connected devices list.
```

<br>

## How to Switch Back to AP Mode

To stop acting as a client and return to AP mode:

1. Run `sudo nmtui` and select `Activate a connection`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/open-connection-list.webp
    :alt: Переход к списку подключений
    :width: 600px
    :align: center

    Opening connection list
    ```

    <br>

2. Select your active client profile and click `Deactivate`.

    ```{figure} @assets@/ru/setup/rpi/wifi-network-manager/deactivate-client.webp
    :alt: Отключение профиля клиента
    :width: 600px
    :align: center

    Deactivating client
    ```

    <br>

3. If the access point does not start automatically, select its specific profile and click `Activate`.

## NetworkManager Profiles

All connection profiles are stored in the `/etc/NetworkManager/system-connections/` directory. While you can manage these manually, it is recommended to use `nmtui` for all modifications.

If the `wlan0` interface is managed by NetworkManager, do not attempt to configure it simultaneously using `dhcpcd.conf`, `wpa_supplicant.conf`, or a separate `dnsmasq` configuration file. Doing so may cause network instability.
