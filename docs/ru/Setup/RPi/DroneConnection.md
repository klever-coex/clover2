# Подключение к квадрокоптеру

1. Подключите аккумулятор к квадрокоптеру и подождите пару минут, пока загрузится образ на Raspberry Pi.
2. В списке доступных сетей Wi-Fi найдите сеть `clover2-xxxxxx` и подключитесь к ней с паролем `cloverwifi`.

<br>

## Web IDE

После подключения к сети откройте в браузере страницу `http://192.168.11.1:9880`, затем создайте терминал через меню `Terminal` -> `New Terminal`.

```{figure} @assets@/common/setup/rpi/drone-connection/web-ide-home.webp
:alt: Главная страница Web IDE
:width: 100%
:align: center

Web IDE в браузере
```

<br>

```{figure} @assets@/common/setup/rpi/drone-connection/web-ide-terminal-menu.webp
:alt: Пункт New Terminal в меню Web IDE
:width: 100%
:align: center

Создание нового терминала через меню Web IDE
```

<br>

```{figure} @assets@/common/setup/rpi/drone-connection/web-ide-terminal-open.webp
:alt: Открытый терминал в Web IDE
:width: 100%
:align: center

Открытый терминал в Web IDE
```

<br>

## SSH

Также к квадрокоптеру можно подключиться по SSH. В Windows для подключения можно использовать программу PuTTY или команду `ssh` в PowerShell или CMD.
По умолчанию используется IP-адрес `192.168.11.1`, имя пользователя — `pi`, пароль — `raspberry`.

```bash
ssh pi@192.168.11.1
```

```{figure} @assets@/common/setup/rpi/drone-connection/ssh-connect.webp
:alt: Подключение к дрону по SSH
:width: 100%
:align: center

Подключение по SSH
```

<br>

```{figure} @assets@/common/setup/rpi/drone-connection/ssh-login.webp
:alt: Успешный вход в терминал дрона
:width: 100%
:align: center

Вход в терминал
```

<br>
