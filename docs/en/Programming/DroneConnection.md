# How to Connect to Your Quadcopter

<br>

There are two primary ways to access the quadcopter’s terminal.

## Method 1: Using the Web IDE

The Web IDE is the quickest way to start working without installing any extra software.

1. Open your web browser and go to `http://192.168.11.1:9880`.
2. In the top menu, navigate to `Terminal` -> `New Terminal`.
3. Your terminal session will open directly in your browser.

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-home.webp
:alt: Главная страница Web IDE
:width: 100%
:align: center

Web IDE in the browser
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-menu.webp
:alt: Пункт New Terminal в меню Web IDE
:width: 100%
:align: center

Creating a new terminal via the Web IDE menu
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/web-ide-terminal-open.webp
:alt: Открытый терминал в Web IDE
:width: 100%
:align: center

Active terminal window in the Web IDE
```

## Method 2: Using SSH

If you prefer working in your own environment, you can connect via SSH.

On Windows, you can use PuTTY or simply use the `ssh` command within PowerShell or CMD.
Default Connection Details:
- IP adress: `192.168.11.1`
- Username: `pi`
- Password: `raspberry`

```bash
ssh pi@192.168.11.1
```

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-connect.webp
:alt: Подключение к дрону по SSH
:width: 100%
:align: center

Connecting via SSH
```

<br>

```{figure} @assets@/common/programming/drone-programming/drone-connection/ssh-login.webp
:alt: Успешный вход в терминал дрона
:width: 100%
:align: center

Logging into the terminal
```
