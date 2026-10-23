# Dev Container

This guide will walk you through setting up a development environment for Clover2 using Visual Studio Code and running the Gazebo simulator via a Dev Container.

## What Is a Dev Container?

A Dev Container is a pre-configured development environment that runs inside Docker. It comes pre-loaded with all the essential tools and dependencies required for Clover2, such as ROS 2 Jazzy, Gazebo, MAVROS, and various build tools.
By using a Dev Container, you avoid the complex process of manually installing and configuring these programs on your host operating system. 
Once Docker and Visual Studio Code are ready, you simply open the project and launch the container to begin working.

---

## 1. Preparation

:::{important}
All commands and actions in this section must be performed on your local host computer, not inside the Dev Container.
If you have already installed Docker, VS Code, and the Dev Containers extension, you may skip this section.
:::

### 1.1. Installing Docker

Follow the [official Docker installation instructions](../../UsefulInformation/Docker) to set up Docker on your system.

---

### 1.2. Installing Visual Studio Code

Download Visual Studio Code from the [official website](https://code.visualstudio.com/download/) and install the version appropriate for your operating system.

```{figure} @assets@/common/simulator/gazebo-dev-container/vscode-downloads.webp
:alt: VS Code downloads
:width: 90%
:align: center
```

For Ubuntu-based Linux, download the `.deb` installation file (e.g., the x64 version) for your system.

Open a terminal, navigate to the directory containing the downloaded file, and run the following command:

```bash
sudo apt install -f ./path-to-file/code_*.deb
```

---

### 1.3. Installing the Dev Containers Extension

1. Launch VS Code.
2. Open the `Extensions` view.
3. Search for `Dev Containers`.
4. Locate the extension published by Microsoft and click `Install`.

```{figure} @assets@/common/simulator/gazebo-dev-container/dev-container-extension.webp
:alt: Dev Containers extension
:width: 90%
:align: center
```

---

## 2. Клонирование проекта

Команды из этого раздела выполняйте на своем компьютере, а не внутри контейнера.

### 2.1. Перейдите в папку для проектов

Создайте папку для проектов и перейдите в нее. Например, используйте папку `~/projects`:

```bash
mkdir -p ~/projects
cd ~/projects
```

* Команда `mkdir -p` создает папку, если она еще не существует.
* Команда `cd` открывает эту папку в терминале.

---

### 2.2. Клонируйте `clover2-dev`

Скачайте исходный код проекта из репозитория GitHub. Для этого выполните команды:

```bash
git clone https://github.com/klever-coex/clover2-dev.git
cd clover2-dev
```

* Команда `git clone` скачивает репозиторий на ваш компьютер. 
* Команда `cd` переходит в созданную папку `clover2-dev`.

---

## 3. Первый запуск Dev Container

Команды и действия из этого раздела выполняйте на своем компьютере в VS Code.

### 3.1. Откройте проект в VS Code

Если вы находитесь в папке `clover2-dev`, откройте проект командой:

```bash
code .
```

Если команда `code` не работает, откройте VS Code вручную и выберите `File` → `Open Folder`. Укажите папку `clover2-dev`.

---

### 3.2. Запустите Dev Container

В VS Code выполните следующие действия:

1. Нажмите `Ctrl+Shift+P`, чтобы открыть палитру команд.
2. Введите `Reopen in Container`.
3. Выберите команду `Dev Containers: Reopen in Container`.
4. Если VS Code предложит выбрать конфигурацию контейнера, выберите один из вариантов:

   - `clover2-dev:universe-devel` — стандартная конфигурация;
   - `clover2-dev:universe-devel (NVIDIA)` — конфигурация для компьютеров с видеокартой NVIDIA.

:::{important}
При первом запуске VS Code скачает необходимые зависимости, это может занять 5-10 минут.
:::

Когда VS Code подключится к контейнеру, в левом нижнем углу окна появится индикатор удаленной среды разработки. Это означает, что вы работаете внутри Dev Container.

```{figure} @assets@/common/simulator/gazebo-dev-container/runned-dev-container.webp
:alt: Запущенный Dev Container
:width: 90%
:align: center
```

---

(simulator-first-build)=

## Загрузка зависимостей и первая сборка

Все команды из этого раздела выполняйте  в терминале Dev Container, открытом в корне проекта `clover2-dev`. Это папка, в которой находится каталог `src` с исходным кодом.

### Загрузка зависимостей

В файле `repos/simulation.yaml` перечислены репозитории, необходимые для работы симулятора, в том числе исходный код `clover2` и `clover2-sim`. Чтобы скачать исходный код этих проектов и других указанных зависимостей в папку `src`, выполните команду:

```bash
vcs import src --recursive < repos/simulation.yaml
```

```{important}
Загрузка может занять продолжительное время, так как скачивается исходный код PX4. Не закрывайте терминал, пока команда не завершится.
```

### Сборка workspace

После загрузки зависимостей соберите проект. Выполните следующие команды в корне `clover2-dev`:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
```

* Первая команда подключает установленную среду `ROS 2 Jazzy`.
* Вторая команда собирает пакеты проекта и размещает результаты сборки в каталоге `install`.

При успешном завершении вывод будет выглядеть примерно так:

```{figure} @assets@/common/simulator/gazebo-dev-container/build-success.webp
:alt: Успешная сборка workspace
:width: 90%
:align: center
```

<br>

### Подключение собранных пакетов

Чтобы ROS 2 мог находить собранные пакеты Clover2, выполните в терминале:

```bash
source ./install/setup.bash
ros2 pkg list | grep clover
```

* Первая команда подключает результаты сборки текущего проекта.
* Вторая команда выводит названия доступных пакетов, содержащих clover.
  
Если команда вывела список пакетов Clover2, значит ROS 2 обнаруживает их в текущем терминале.
Вы должны увидеть список пакетов Clover2, например:

```{figure} @assets@/common/simulator/gazebo-dev-container/list-builded-packages.webp
:alt: Пакеты Clover2 в собранном workspace
:width: 90%
:align: center

Пакеты Clover2 в собранном workspace
```

Чтобы не подключать результаты сборки вручную при каждом открытии нового терминала, можно добавить команды подключения в `~/.bashrc`:

```bash
echo "source ~/clover2-dev/install/setup.bash" >> ~/.bashrc
```

<br>

После завершения настройки перейдите к инструкции по {ref}`запуску симулятора <simulator-launch>`.
