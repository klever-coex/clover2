# Ноды

Нодой в контексте `ROS2` называют программный компонент, который выполняет конкретную задачу, например получение изображения с камеры или распознавание ArUco-меток.

Логику системы специально разделяют на отдельные ноды чтобы упростить разработку, тестирование и сопровождение программы. Ноды могут запускаться и работать независимо друг от друга, поэтому ошибка или остановка одной ноды не обязательно останавливает остальные. 

Ноды в `ROS2` можно писать на разных языках программирования, если для этого языка существует библиотека-клиент ROS 2.

````{list-table}
:header-rows: 1
:widths: 30 90 90

* - Язык
  - Библиотека
  - Поддержка

* - `Python`
  - [`rclpy`](https://github.com/ros2/rclpy/)
  - Полная

* - `C++`
  - [`rclcpp`](https://github.com/ros2/rclcpp/)
  - Полная

* - `C`
  - [`rclc`](https://github.com/ros2/rclc/)
  - Полная

* - `Rust`
  - [`ros2_rust`](https://github.com/ros2-rust/ros2_rust/)
  - Экспериментальная

````

## Простые примеры

Сразу к делу. Ниже приведены два минимальных примера ноды на двух языках. Оба выполняют одно и тоже действие — публикуют строку в `/topic` с увеличивающимся значением. Публикация происходит по таймеру каждые 500мс.

Важно обратить внимание на `rclpy.spin(node)` и `rclcpp::spin(node)`. Эти вызовы передают управление планировщику, который обрабатывает события ноды: вызовы callback-функций таймеров, получение сообщений и другие доступные события.
Без `spin()` в этих примерах программа создаст ноду и таймер, но не будет выполнять callback-функцию таймера. Например, при создании таймера мы передаем функцию `timer_callback` которая будет вызываться, когда наступает время следующего срабатывания таймера.

````{tab-set-code}
```python
import rclpy
from std_msgs.msg import String

def main(args=None):
    counter = 0

    rclpy.init(args=args)

    node = rclpy.create_node('minimal_publisher')    
    publisher = node.create_publisher(String, 'topic', 10)

    def timer_callback():
        nonlocal counter
        msg = String()
        msg.data = f'Hello World: {counter}'
        publisher.publish(msg)
        node.get_logger().info(f'Publishing: "{msg.data}"')
        counter += 1

    timer = node.create_timer(0.5, timer_callback)
    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

```c++
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <chrono>

int main(int argc, char * argv[]) {
    int counter = 0;

    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("minimal_publisher");
    auto publisher = node->create_publisher<std_msgs::msg::String>("topic", 10);

    auto timer_callback = [&]() {
        auto msg = std_msgs::msg::String();
        msg.data = "Hello World: " + std::to_string(counter);
        publisher->publish(msg);
        RCLCPP_INFO(node->get_logger(), "Publishing: '%s'", msg.data.c_str());
        counter++;
    };

    auto timer = node->create_wall_timer(
        std::chrono::milliseconds(500),
        timer_callback
    );

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
```
````

`spin()` не всегда означает однопоточную обработку всех событий. Вызов `rclpy.spin()` и `rclcpp::spin()` использует планировщик по умолчанию, который обрабатывает callback-функции последовательно в одном потоке. В ROS 2 также существуют многопоточные планировщики, которые позволяют выполнять совместимые callback-функции параллельно.

При этом не следует выполнять длительные или блокирующие операции внутри callback-функций без необходимости. Например, `time.sleep(...)` внутри `callback` блокирует поток, который выполняет этот `callback`. При однопоточном планировщике в это время другие callback-функции этой ноды не смогут выполняться.

## Продвинуте примеры

Основными языками для написания нод являются `Python` и `C++`, оба два поддерживают [ООП](https://habr.com/ru/articles/463125/). 

Для больших программ удобно использовать следующий подход: логику ноды размещают в классе, а настройки, `publishers`, `subscribers` и `timers` хранят как свойства этого класса:

````{tab-set-code}
```python
import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class MinimalPublisher(Node):
    def __init__(self):
        super().__init__('minimal_publisher')
        self.publisher_ = self.create_publisher(String, 'topic', 10)
        timer_period = 0.5
        self.timer = self.create_timer(timer_period, self.timer_callback)
        self.counter = 0

    def timer_callback(self):
        msg = String()
        msg.data = f'Hello World: {self.counter}'
        self.publisher_.publish(msg)
        self.get_logger().info(f'Publishing: "{msg.data}"')
        self.counter += 1

def main(args=None):
    rclpy.init(args=args)
    node = MinimalPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

```c++
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <chrono>

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher()
    : Node("minimal_publisher"), counter_(0)
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&MinimalPublisher::timer_callback, this)
        );
    }

private:
    void timer_callback()
    {
        auto msg = std_msgs::msg::String();
        msg.data = "Hello World: " + std::to_string(counter_);
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
        counter_++;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int counter_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MinimalPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
```
````

Подобный подход на первый взгляд кажется сложнее, но упрощает разработку, когда нода начинает расти. Кроме того, в `C++` такой подход позволяет создавать [composable](https://docs.ros.org/en/jazzy/Concepts/Intermediate/About-Composition.html) ноды. 
Их можно запускать в одном процессе. Это позволяет уменьшить расходы на обмен большими сообщениями между отдельными процессами, например изображениями с камеры. В `ROS 1 `похожий механизм назывался Nodelet. В `ROS 2` используется механизм composition, который позволяет загружать совместимые ноды в один процесс.

## Имя и неймспейс

В примерах выше мы каждый раз передавали в конструктор строку `minimal_publisher` — это ***имя ноды***. Оно используется для идентификации ноды в ROS 2-графе.

Второй атрибут ноды — ***неймспейс***. Он позволяет организовать имена нод и ресурсов в иерархию. Полное имя ноды состоит из неймспейса и имени.

Если неймспейс не задан, нода создается в корневом неймспейсе, и ее полное имя будет `/minimal_publisher`. В одном неймспейсе две ноды не должны иметь одинаковое полное имя.

Список запущенных нод показывает команда `ros2 node list`:

```bash
$ ros2 node list
/minimal_publisher
```

Имя и неймспейс решают разные задачи:

- **Имя** идентифицирует конкретную ноду. Например, в `clover2` драйвер камеры `camera_node` запускается дважды под именами `main_camera` и `front_camera` для основной и передней камер. Исполняемый файл при этом может быть одним и тем же, а в ROS 2-графе будут находиться две разные ноды.
- **Неймспейс** позволяет группировать ноды и избегать конфликтов имен. Относительные имена топиков и сервисов учитывают неймспейс. Например, нода `camera` в неймспейсе `front` публикует изображения в `/front/image_raw`, а такая же нода в неймспейсе `main` — в `/main/image_raw`.

В именах и неймспейсах ROS 2 используются латинские буквы, цифры и символы подчеркивания.

Заданные в коде имя и неймспейс не окончательные — их можно переопределить при запуске, не меняя код. 

## Запуск нод

Нода — это исполняемый файл внутри `ROS2`-пакета. Самый простой способ ее запустить:

| Команда | Что делает |
| --- | --- |
| `ros2 pkg executables <пакет>` | Показывает исполняемые файлы пакета |
| `ros2 run <пакет> <исполняемый файл>` | Запускает один исполняемый файл из пакета |

К команде запуска можно добавить `--ros-args` и правила переназначения (`-r`), которые переопределяют имя и неймспейс ноды:

```bash
# запустить драйвер камеры под именем front_camera
ros2 run camera_ros camera_node --ros-args -r __node:=front_camera

# запустить его же в неймспейсе /front
ros2 run camera_ros camera_node --ros-args -r __node:=camera -r __ns:=/front
```

## Параметры

***Параметры*** — это настройки ноды. У каждого параметра есть имя, тип и значение.

Параметр обязательно объявляется в коде ноды. При объявлении задаются его имя и значение по умолчанию. В зависимости от настроек ноды и версии ROS 2 попытка передать значение необъявленного параметра может быть отклонена.

````{tab-set-code}
```python
node = rclpy.create_node('frequency_talker')

# имя и значение по умолчанию
node.declare_parameter('frequency', 0.5)
node.declare_parameter('message', 'Hello World')

# чтение текущих значений
frequency = node.get_parameter('frequency').value
message = node.get_parameter('message').value
```

```c++
auto node = std::make_shared<rclcpp::Node>("frequency_talker");

// имя и значение по умолчанию
node->declare_parameter<double>("frequency", 0.5);
node->declare_parameter<std::string>("message", "Hello World");

// чтение текущих значений
double frequency = node->get_parameter("frequency").as_double();
std::string message = node->get_parameter("message").as_string();
```
````

```{note}
Тип параметра определяется при его объявлении. При попытке установить значение несовместимого типа ROS 2 может отклонить такое значение.
```

Значения по умолчанию используются, если параметр не задан другим способом. Передать значение можно несколькими способами:

* **Из командной строки.** К `ros2 run` добавляются присваивания `-p имя:=значение`:

```bash
ros2 run my_package frequency_talker --ros-args \
    -p frequency:=2.0 \
    -p message:="Привет, мир"
```

Списки можно передавать в квадратных скобках, например: `-p image_size:="[256, 384]"`.

* **Уже запущенной ноде.** Значения можно читать и менять на ходу командами `ros2 param get` / `ros2 param set` — все такие команды собраны в статье [Команды ROS 2](Commands).

* **Файлом параметров.** Когда параметров много, удобнее описать их в одном YAML-файле и передавать его целиком. В `clover2` так устроены файлы из `clover2_bringup/params`, например фрагмент `klever5.yaml`:

```yaml
/**:
  ros__parameters:
    use_intra_process_comms: true

/**/aruco_tracker:
  ros__parameters:
    tracking: "base_link"

/**/led_strip:
  ros__parameters:
    brightness_scale: 0.5
    led_count: 80
```

Верхний ключ — шаблон, по которому выбираются ноды: `/**` соответствует нодам в любом неймспейсе, а `/**/aruco_tracker` — ноде с именем `aruco_tracker` в любом неймспейсе. Благодаря шаблонам один файл может задавать параметры сразу для нескольких нод. Уровень `ros__parameters` — обязательный элемент структуры файла параметров. Внутри него перечисляются сами параметры.

Файл передается ноде при запуске через `--params-file`:

```bash
ros2 run my_package frequency_talker --ros-args --params-file my_params.yaml
```

## Lifecycle

ROS 2 поддерживает специальные [`Lifecycle`](https://design.ros2.org/articles/node_lifecycle.html)-ноды , которые позволяют явно управлять состоянием ноды и последовательностью ее запуска и остановки. Обычная нода может начать выполнять свою работу сразу после запуска. Lifecycle-ноды работают как конечный автомат с заранее определенными состояниями и переходами.

```{mermaid}
stateDiagram-v2
    [*] --> Unconfigured: create

    Unconfigured --> Configuring: configure
    Configuring --> Inactive: on_configure() == SUCCESS
    Configuring --> Unconfigured: on_configure() == FAILURE
    Configuring --> ErrorProcessing: on_configure() == ERROR

    Inactive --> Activating: activate
    Activating --> Active: on_activate() == SUCCESS
    Activating --> ErrorProcessing: on_activate() == ERROR

    Active --> Deactivating: deactivate
    Deactivating --> Inactive: on_deactivate() == SUCCESS
    Deactivating --> ErrorProcessing: on_deactivate() == ERROR

    Active --> ErrorProcessing: Unhandled error

    Inactive --> CleaningUp: cleanup
    CleaningUp --> Unconfigured: on_cleanup() == SUCCESS
    CleaningUp --> ErrorProcessing: on_cleanup() == ERROR

    Unconfigured --> ShuttingDown: shutdown
    Inactive --> ShuttingDown: shutdown
    Active --> ShuttingDown: shutdown
    ShuttingDown --> Finalized: on_shutdown() == SUCCESS

    ErrorProcessing --> Unconfigured: on_error() == SUCCESS
    ErrorProcessing --> Finalized: on_error() == FAILURE / ERROR

    Finalized --> [*]: destroy
```

Такая система позволяет внешнему компоненту управлять настройкой, активацией, деактивацией и завершением Lifecycle-ноды.
Lifecycle-нода может находиться в следующих основных состояниях:


- `Unconfigured` — нода еще не настроена.
- `Inactive` — нода настроена, но ее основная работа не активирована.
- `Active` — нода активна и выполняет основную работу.
- `Finalized` — нода завершила жизненный цикл и готова к уничтожению.

Переходы между состояниями выполняются через специальные интерфейсы Lifecycle-ноды. В частности, стандартный интерфейс предоставляет сервис `/<имя ноды>/change_state`, через который можно запросить переход в другое состояние.
