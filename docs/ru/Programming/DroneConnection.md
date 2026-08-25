# Подключение к дрону

<br>

## Web IDE

Сейчас удобнее всего работать с терминалом дрона через Web IDE. Откройте в браузере `http://192.168.11.1:9880`, затем создайте терминал через меню `Terminal` -> `New Terminal`.

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-home.webp
:alt: Главная страница Web IDE
:width: 100%
:align: center

Рисунок 1 — Web IDE в браузере
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-menu.webp
:alt: Пункт New Terminal в меню Web IDE
:width: 100%
:align: center

Рисунок 2 — Создание нового терминала через меню Web IDE
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-open.webp
:alt: Открытый терминал в Web IDE
:width: 100%
:align: center

Рисунок 3 — Открытый терминал в Web IDE
```

## SSH

Также к дрону можно подключиться по SSH. На Windows для этого можно использовать PuTTY или команду `ssh` в PowerShell/CMD.
По умолчанию адрес дрона — `192.168.11.1`, имя пользователя — `pi`, пароль — `raspberry`.

```bash
ssh pi@192.168.11.1
```

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-connect.webp
:alt: Подключение к дрону по SSH
:width: 100%
:align: center

Рисунок 4 — Подключение к дрону по SSH
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-login.webp
:alt: Успешный вход в терминал дрона
:width: 100%
:align: center

Рисунок 5 — Вход в терминал
```
