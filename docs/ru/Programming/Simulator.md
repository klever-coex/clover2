# Запуск симулятора Clover2 через Dev Container

Эта инструкция поможет запустить симулятор Clover2 в Visual Studio Code через Dev Container.

Dev Container — заранее подготовленная среда разработки внутри Docker, где уже установлены нужные программы.

---

## 1. Что мы будем делать

Мы подготовим компьютер и запустим проект `clover2-dev` в Dev Container.

В результате вы:

1. Откроете проект `clover2-dev` в Visual Studio Code.
2. Запустите Dev Container `clover2-dev:universe-devel` или `clover2-dev:universe-devel (NVIDIA)`.
3. Загрузите исходные коды симулятора и Clover2.
4. Соберёте проект командой `colcon build`.
5. Запустите симулятор командой:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

### Зачем нужен Dev Container

Без Dev Container пришлось бы вручную устанавливать ROS 2 Jazzy, Gazebo, MAVROS, PX4-зависимости и инструменты сборки. Dev Container запускает готовую среду внутри Docker.

### Результат

После запуска симулятора вы должны увидеть сообщения ROS 2 в терминале. Также должно открыться окно Gazebo / Gz или появиться процесс запуска симуляции.

---

## 2. Подготовка

> Важно
>
> Команды из этого раздела выполняются на вашем компьютере, а не внутри Dev Container.
>
> Если вы уже установили Docker, VS Code и расширение Dev Containers, этот раздел можно использовать как проверку.

### 2.1. Установка Docker

Воспользуйтесь [документацией по установке Docker](Docker.md).

---

### 2.2. Установка Visual Studio Code

Скачайте Visual Studio Code с официального сайта:

<https://code.visualstudio.com/download/>

Установите версию для вашей операционной системы.

Для Linux скачайте файл `.deb` для вашей системы, например x64, и выполните:

```bash
sudo apt install ./путь-до-файла/code_*.deb
```

**Результат:** Visual Studio Code запускается и может открыть папку проекта.

---

### 2.3. Установка расширения Dev Containers

Откройте страницу расширения:

<https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers>

Или установите его внутри VS Code:

1. Откройте VS Code.
2. Нажмите значок Extensions.
3. Найдите `Dev Containers`.
4. Установите расширение от Microsoft.

**Результат:** в VS Code появляется команда `Dev Containers: Reopen in Container`.

---

## 3. Клонирование проекта

Команды из этого раздела выполняются на компьютере, не внутри контейнера.

### 3.1. Перейти в папку для проектов

Например, можно использовать папку `~/projects`:

```bash
mkdir -p ~/projects
cd ~/projects
```

---

### 3.2. Склонировать `clover2-dev`

Склонируйте репозиторий по HTTPS:

```bash
git clone https://github.com/klever-coex/clover2-dev.git
cd clover2-dev
```

---

## 4. Первый запуск Dev Container

Команды и действия из этого раздела выполняются на компьютере в VS Code.

### 4.1. Открыть проект в VS Code

Если вы находитесь в папке `clover2-dev`, можно открыть её командой:

```bash
code .
```

**Результат:** откроется окно VS Code с файлами проекта.

Если команда `code` не работает, откройте VS Code вручную и выберите папку `clover2-dev` через меню `File → Open Folder`.

---

### 4.2. Запустить Dev Container

В VS Code:

1. Нажмите `Ctrl+Shift+P`.
2. Введите `Reopen in Container`.
3. Выберите команду `Dev Containers: Reopen in Container`.
4. Выберите один из вариантов:

   - `clover2-dev:universe-devel`;
   - `clover2-dev:universe-devel (NVIDIA)` — если у вас NVIDIA-видеокарта.

VS Code прочитает файл `.devcontainer/universe-devel/devcontainer.json` или `.devcontainer/universe-devel-nvidia/devcontainer.json`, затем Docker Compose запустит окружение из `docker/devcontainer`.

Docker попытается использовать образ:

```text
ghcr.io/klever-coex/clover2-dev/clover2-core:test
```

Проект будет подключён внутри контейнера в папку:

```text
/home/dev/clover2-dev
```

Терминал внутри VS Code будет работать от пользователя `dev`.

### Сколько это может занять

Первый запуск может занять некоторое время: Docker-образ большой, скачивается из интернета, а VS Code настраивает расширения внутри контейнера.

### Результат

В левом нижнем углу VS Code должно появиться указание, что окно открыто в Dev Container.

В терминале внутри VS Code текущая папка должна быть:

```text
/home/dev/clover2-dev
```

Проверить можно командой внутри контейнера:

```bash
pwd
```

---

## 5. Загрузка зависимостей

Теперь нужно скачать исходные коды, из которых собирается симулятор. Команды выполняются внутри Dev Container.

### 5.1. Файл со списком зависимостей

В проекте используется файл:

```text
repos/simulation.yaml
```

В нём указаны репозитории `clover2-sim` и `clover2`:

```yaml
repositories:
  clover2-sim:
    type: git
    url: https://github.com/klever-coex/clover2-sim.git
    version: feature/clover2-dev

  clover2:
    type: git
    url: https://github.com/klever-coex/clover2.git
    version: feature/clover2-dev
```

Оба репозитория будут скачаны в папку `src`.

---

### 5.2. Скачать зависимости

В терминале Dev Container выполните:

```bash
vcs import src --recursive < repos/simulation.yaml
```

**Результат:**

- в папке `src` появятся папки `clover2-sim` и `clover2`;
- команда завершится без ошибки Git.

Проверить можно командой:

```bash
ls src
```

---

## 6. Сборка проекта

Сборка подготавливает исходный код и создаёт готовые ROS 2-пакеты. Команды выполняются внутри Dev Container.

### 6.1. Подключить ROS 2 Jazzy

В терминале Dev Container выполните:

```bash
source /opt/ros/jazzy/setup.bash
```

**Результат:** команда обычно ничего не выводит, после неё должна работать команда `ros2`.

Проверить можно так:

```bash
ros2 --help
```

---

### 6.2. Собрать workspace

В терминале Dev Container выполните:

```bash
colcon build --symlink-install --cmake-args  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**Результат:**

- сборка завершится без строки `Failed`;
- появятся папки `build`, `install` и `log`;
- в конце будет сообщение о завершении сборки пакетов.

---

## 7. Запуск симулятора

После сборки нужно подключить собранные пакеты. Команды выполняются внутри Dev Container.

### 7.1. Подключить собранный проект

В терминале Dev Container выполните:

```bash
source ./install/setup.bash
```

**Результат:** команда обычно ничего не выводит, после неё ROS 2 должен видеть пакеты из workspace.

Проверить можно командой:

```bash
ros2 pkg list | grep clover
```

**Результат:** в списке должны быть пакеты Clover2, включая пакет симуляции, если он успешно скачан и собран.

---

### 7.2. Запустить симулятор

В терминале Dev Container, где уже выполнено `source ./install/setup.bash`, выполните:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

**Результат:**

- в терминале появятся сообщения ROS 2;
- должен начаться запуск Gazebo / Gz;
- откроется графическое окно симулятора;
- процесс не должен сразу завершиться с ошибкой.

---

## 8. Повторный запуск

После перезагрузки компьютера всё устанавливать заново не нужно.

### 8.1. Что делать после перезагрузки

1. Запустите Docker.
2. Откройте VS Code.
3. Откройте папку `clover2-dev`.
4. Выберите `Dev Containers: Reopen in Container`.
5. Откройте терминал внутри контейнера.
6. Подключите собранный проект:

```bash
source ./install/setup.bash
```

**Результат:** ROS 2 снова видит пакеты проекта.

После этого можно запускать симулятор:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

---

### 8.2. Когда нужно пересобирать проект

Пересборка нужна, если:

- вы впервые скачали исходники;
- изменился C++-код;
- изменились `CMakeLists.txt` или `package.xml`;
- появились новые ROS 2-пакеты;
- команда запуска не видит пакет;
- после `vcs import` были обновлены репозитории.

Для пересборки используйте:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install --cmake-args  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

**Результат:** сборка завершается без ошибок, папка `install` обновляется.

---

### 8.3. Когда достаточно только `source ./install/setup.bash`

Достаточно выполнить только:

```bash
source ./install/setup.bash
```

если:

- проект уже был собран;
- вы просто открыли новый терминал;
- вы перезапустили контейнер;
- вы хотите снова запустить симулятор без изменения кода.

**Результат:** ROS 2 видит уже собранные пакеты.
