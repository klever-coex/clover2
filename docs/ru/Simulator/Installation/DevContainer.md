# Dev Container

Эта инструкция поможет запустить симулятор Gazebo для разработки Clover2 в Visual Studio Code через Dev Container.

## Что такое Dev Container

Dev Container — заранее подготовленная среда разработки внутри Docker, где уже установлены нужные программы. Без него пришлось бы вручную устанавливать ROS 2 Jazzy, Gazebo, MAVROS, PX4-зависимости и инструменты сборки. Dev Container запускает готовую среду внутри Docker.

---

## 1. Подготовка

:::{important}
Команды из этого раздела выполняются на вашем компьютере, а не внутри Dev Container.
Если вы уже установили Docker, VS Code и расширение Dev Containers, этот раздел можно использовать как проверку.
:::

### 1.1. Установка Docker

Воспользуйтесь [документацией по установке Docker](../../UsefulInformation/Docker), если он не стоит.

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
   - `clover2-dev:universe-devel (NVIDIA)` — если у вас NVIDIA-видеокарта.

:::{important}
При первом запуске VS Code скачает необходимые зависимости, это может занять 5-10 минут.
:::

Когда `VS Code` подключен к контейнеру в левом нижнем углу окна, это помечается.

```{figure} @assets@/common/simulator/gazebo-dev-container/runned-dev-container.webp
:alt: Запущенный Dev Container
:width: 90%
:align: center
```

---

(simulator-first-build)=

## Загрузка зависимостей и первая сборка

Команды выполняются в терминале Dev Container, в корне открытого workspace `clover2-dev` — папке с исходниками `src`.

### Загрузка зависимостей

В `clover2-dev` список репозиториев для симуляции находится в `repos/simulation.yaml`. Он содержит исходники `clover2` и `clover2-sim`, которые будут загружены в `src`:

```bash
vcs import src --recursive < repos/simulation.yaml
```

```{important}
Загрузка может занять много времени, так как скачивается исходный код PX4.
```

### Сборка workspace

В корне workspace выполните:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
```

При успешном завершении вывод будет выглядеть примерно так:

```{figure} @assets@/common/simulator/gazebo-dev-container/build-success.webp
:alt: Успешная сборка workspace
:width: 90%
:align: center
```

<br>

### Подключение собранных пакетов

В терминале выполните:

```bash
source ./install/setup.bash
ros2 pkg list | grep clover
```

Вы должны увидеть список пакетов Clover2, например:

```{figure} @assets@/common/simulator/gazebo-dev-container/list-builded-packages.webp
:alt: Пакеты Clover2 в собранном workspace
:width: 90%
:align: center

Пакеты Clover2 в собранном workspace
```

Рекомендуется добавить команды подключения в `~/.bashrc`, иначе в каждом новом терминале собранный workspace нужно будет подключать заново:

```bash
echo "source ~/clover2-dev/install/setup.bash" >> ~/.bashrc
```

<br>

После сборки перейдите к {ref}`запуску симулятора <simulator-launch>`.

