# Gazebo Dev Container

:::{warning}
Текущая инструкция прeдназначена для пользователей операционной системы Ubuntu и на других системах не проверялась.
:::

Эта инструкция поможет запустить симулятор Gazebo для разработки Clover2 в Visual Studio Code через Dev Container.

## Dev Container

Dev Container - заранее подготовленная среда разработки внутри Docker, где уже установлены нужные программы. Без него пришлось бы вручную устанавливать ROS 2 Jazzy, Gazebo, MAVROS, PX4-зависимости и инструменты сборки. Dev Container запускает готовую среду внутри Docker.

---

## 1. Подготовка

:::{important}
Команды из этого раздела выполняются на вашем компьютере, а не внутри Dev Container.
Если вы уже установили Docker, VS Code и расширение Dev Containers, этот раздел можно использовать как проверку.
:::

### 1.1. Установка Docker

Воспользуйтесь [документацией по установке Docker](../Programming/Docker.md).

---

### 1.2. Установка Visual Studio Code

Скачайте Visual Studio Code с [официального сайта](https://code.visualstudio.com/download/) и установите версию для вашей операционной системы.

```{figure} @assets@/common/simulator/gazebo-dev-container/vscode-downloads.webp
:alt: VS Code загрузки
:width: 90%
:align: center
```

Для Linux скачайте файл `.deb` для вашей системы, например x64, и выполните:

```bash
sudo apt install -f ./путь-до-файла/code_*.deb
```

---

### 1.3. Установка расширения Dev Containers

1. Откройте VS Code.
2. Нажмите значок Extensions.
3. Найдите `Dev Containers`.
4. Установите расширение от Microsoft.

```{figure} @assets@/common/simulator/gazebo-dev-container/dev-container-extension.webp
:alt: Dev Containers расширение
:width: 90%
:align: center
```

---

## 2. Клонирование проекта

Команды из этого раздела выполняются на компьютере, не внутри контейнера.

### 2.1. Перейти в папку для проектов

Например, можно использовать папку `~/projects`:

```bash
mkdir -p ~/projects
cd ~/projects
```

---

### 2.2. Склонировать `clover2-dev`

Склонируйте репозиторий:

```bash
git clone https://github.com/klever-coex/clover2-dev.git
cd clover2-dev
```

---

## 3. Первый запуск Dev Container

Команды и действия из этого раздела выполняются на компьютере в VS Code.

### 3.1. Открыть проект в VS Code

Если вы находитесь в папке `clover2-dev`, можно открыть её командой:

```bash
code .
```

Если команда `code` не работает, откройте VS Code вручную и выберите папку `clover2-dev` через меню `File -> Open Folder`.

---

### 3.2. Запустить Dev Container

В VS Code:

1. Нажмите `Ctrl+Shift+P`.
2. Введите `Reopen in Container`.
3. Выберите команду `Dev Containers: Reopen in Container`.
4. Выберите один из вариантов:

   - `clover2-dev:universe-devel`;
   - `clover2-dev:universe-devel (NVIDIA)` - если у вас NVIDIA-видеокарта.

:::{important}
При первом запуске VS Code скачает необходимые зависимости, это может занять 5-10 минут.
:::

Когда `VS Code` подключен к контейнеру в левом нижнем углу окна, это помечается.

```{figure} @assets@/common/simulator/gazebo-dev-container/runned-dev-container.webp
:alt: Запушенный Dev Container
:width: 90%
:align: center
```

---

## 4. Загрузка зависимостей

Теперь нужно скачать исходные коды, из которых собирается симулятор. Команды выполняются внутри Dev Container.

### 4.1. Файл со списком зависимостей

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

### 4.2. Скачать зависимости

В терминале Dev Container выполните:

```bash
vcs import src --recursive < repos/simulation.yaml
```

:::{important}
Загрузка может занять много времени так как скачивается исходный код PX4.
:::

---

## 5. Сборка проекта

Сборка подготавливает исходный код и создаёт готовые ROS 2-пакеты. Команды выполняются внутри Dev Container.

### 5.1. Подключить ROS 2 Jazzy

В терминале Dev Container выполните:

```bash
source /opt/ros/jazzy/setup.bash
```

---

### 5.2. Собрать workspace

В терминале Dev Container выполните:

```bash
colcon build --symlink-install
```

Если все сделанно верно, то вывод в терминале будет выглядить примерно так:

```{figure} @assets@/common/simulator/gazebo-dev-container/build-success.webp
:alt: Успешная сборка
:width: 90%
:align: center
```

---

## 6. Запуск симулятора

После сборки нужно подключить собранные пакеты. Команды выполняются внутри Dev Container.

### 6.1. Подключить собранный проект

В терминале Dev Container выполните:

```bash
source ./install/setup.bash
```

**Результат:** команда обычно ничего не выводит, после неё ROS 2 должен видеть пакеты из workspace.

Проверить можно командой:

```bash
ros2 pkg list | grep clover
```

```{figure} @assets@/common/simulator/gazebo-dev-container/list-builded-packages.webp
:alt: Успешная настройка clover2 пакетов
:width: 90%
:align: center
```

---

### 6.2. Запустить симулятор

В терминале Dev Container, где уже выполнено `source ./install/setup.bash`, выполните:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

**Результат:**

- в терминале появятся сообщения ROS 2;
- должен начаться запуск Gazebo / Gz;
- откроется графическое окно симулятора;
- процесс не должен сразу завершиться с ошибкой.

```{figure} @assets@/common/simulator/gazebo-dev-container/runned-gazebo.webp
:alt: Работа симулятора
:width: 90%
:align: center
```

---

## 7. Повторный запуск

После перезагрузки компьютера всё устанавливать заново не нужно.

### 7.1. Что делать после перезагрузки

1. Откройте VS Code.
2. Откройте папку `clover2-dev`.
3. Выберите `Dev Containers: Reopen in Container`.
4. Откройте терминал внутри контейнера.
5. Подключите собранный проект:

```bash
source ./install/setup.bash
```

После этого можно запускать симулятор:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

---

### 7.2. Когда нужно пересобирать проект

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
colcon build --symlink-install
```

---

### 7.3. Когда достаточно только `source ./install/setup.bash`

Достаточно выполнить только:

```bash
source ./install/setup.bash
```

если:

- проект уже был собран;
- вы просто открыли новый терминал;
- вы перезапустили контейнер;
- вы хотите снова запустить симулятор без изменения кода.
