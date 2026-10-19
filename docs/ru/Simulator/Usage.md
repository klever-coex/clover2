# Использование

Сначала подготовьте среду по {doc}`инструкции установки <VirtualMachine>`. Команды ниже выполняются внутри Dev Container или гостевой Linux в VMware/UTM. Workspace готового образа находится в `~/clover2-dev`.

(simulator-launch)=

## Запуск симулятора

:::{important}
В готовом образе VMware/UTM среда уже подготовлена.  
Если используете Dev Container впервые, сначала выполните {ref}`загрузку зависимостей и сборку <simulator-first-build>`. Затем запускайте ту же команду в терминале контейнера с подключённым workspace.
:::

Откройте терминал внутри гостевой Linux и выполните:

```bash
ros2 launch clover2_sim gz_simple.launch.py
```

```{figure} @assets@/common/simulator/vmware/launch-simulator.webp
:alt: Запуск симулятора
:width: 90%
:align: center
Запуск симулятора
```

<br>

После запуска откроется графическое окно Gazebo с моделью квадрокоптера. В терминале появятся сообщения ROS 2; оставьте его открытым на время работы. Для остановки симулятора нажмите `Ctrl+C` в этом терминале.

```{figure} @assets@/common/simulator/vmware/gazebo-running.webp
:alt: Симулятор Gazebo
:width: 90%
:align: center
Симулятор Gazebo
```

<br>

## Повторный запуск

После перезагрузки компьютера не нужно заново устанавливать среду или собирать неизменённый проект.

1. Запустите машину в VMware/UTM, либо откройте `clover2-dev` в VS Code и выберите **Dev Containers: Reopen in Container**.
2. Откройте терминал внутри выбранной среды.
3. В готовом образе с настроенным окружением выполните команду запуска из раздела выше.
   ```bash
   ros2 launch clover2_sim gz_simple.launch.py
   ```

### Когда нужно пересобирать проект

Пересборка нужна, если:

- Вы впервые скачали исходники;
- Изменился C++-код;
- Изменились `CMakeLists.txt` или `package.xml`;
- Появились новые ROS 2-пакеты;
- Команда запуска не видит пакет;
- После `vcs import` были обновлены репозитории.

Для пересборки используйте:

```bash
cd ~/clover2-dev
colcon build --symlink-install
```
