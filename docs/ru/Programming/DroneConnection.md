# Подключение к конструктору квадрокоптера

<br>

## Web IDE

Сейчас удобнее всего работать с терминалом конструктора через Web IDE. Откройте в браузере страницу `http://192.168.11.1:9880`, затем создайте терминал через меню `Terminal` -> `New Terminal`.

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-home.webp
:alt: Главная страница Web IDE
:width: 100%
:align: center

Web IDE в браузере
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-menu.webp
:alt: Пункт New Terminal в меню Web IDE
:width: 100%
:align: center

Создание нового терминала через меню Web IDE
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-open.webp
:alt: Открытый терминал в Web IDE
:width: 100%
:align: center

Рисунок 3 — Открытый терминал в Web IDE
```

## SSH

Также к конструктору можно подключиться по SSH. В Windows подключения можно использовать программу PuTTY или команду `ssh` в PowerShell или CMD.
По умолчанию используется IP-адрес `192.168.11.1`, имя пользователя — `pi`, пароль — `raspberry`.

```bash
ssh pi@192.168.11.1
```

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-connect.webp
:alt: Подключение к дрону по SSH
:width: 100%
:align: center

Подключение по SSH
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-login.webp
:alt: Успешный вход в терминал дрона
:width: 100%
:align: center

Вход в терминал
```
