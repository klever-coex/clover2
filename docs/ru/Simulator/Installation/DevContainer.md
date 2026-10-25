# Dev Container

Эта инструкция поможет настроить среду для разработки Clover2 в Visual Studio Code и запустить симулятор Gazebo с помощью Dev Container.

## Что такое Dev Container

Dev Container — готовая среда разработки, которая работает внутри Docker.  В ней уже установлены необходимые программы и зависимости для работы с Clover 2, включая ROS 2 Jazzy, Gazebo, MAVROS и инструменты сборки. Благодаря Dev Container не нужно устанавливать и настраивать все эти программы вручную на своем компьютере. Достаточно подготовить Docker и Visual Studio Code, открыть проект и запустить контейнер.

---

## 1. Подготовка

:::{important}
Команды и действия из этого раздела выполняйте на вашем компьютере, а не внутри Dev Container.
Если вы уже установили Docker, VS Code и расширение Dev Containers, этот раздел можно использовать как проверку.
:::

### 1.1. Установка Docker

Установите Docker, следуя [инструкции по установке Docker](../../UsefulInformation/Docker).

---

### 1.2. Установка Visual Studio Code

Скачайте Visual Studio Code с [официального сайта](https://code.visualstudio.com/download/) и установите версию для вашей операционной системы.

```{figure} @assets@/common/simulator/gazebo-dev-container/vscode-downloads.webp
:alt: VS Code загрузки
:width: 90%
:align: center
```

Если вы используете Linux на базе Ubuntu, скачайте установочный файл `.deb` для вашей системы, например x64.

Откройте терминал, перейдите в папку со скачанным файлом и выполните команду:

```bash
sudo apt install -f ./путь-до-файла/code_*.deb
```

---

### 1.3. Установка расширения Dev Containers

1. Откройте VS Code.
2. Откройте раздел `Extensions`.
3. Введите в поиске `Dev Containers`.
4. Найдите расширение от Microsoft и нажмите `Install`.

```{figure} @assets@/common/simulator/gazebo-dev-container/dev-container-extension.webp
:alt: Dev Containers расширение
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

В файле `repos/simulation_stable.yaml` перечислены репозитории, необходимые для работы симулятора, в том числе исходный код `clover2` и `clover2-sim`. Чтобы скачать исходный код этих проектов и других указанных зависимостей в папку `src`, выполните команду:

```bash
vcs import src --recursive < repos/simulation_stable.yaml
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

