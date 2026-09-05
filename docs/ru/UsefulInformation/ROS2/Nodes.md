# Ноды

Нодой в контексте `ROS2` называют минимальную исполняемую единицу (программу). Каждая нода выполняет конкретную задачу, например чтение картинки с камеры или распознование ArUco меток. Логику специально разбивают на отдельные ноды чтобы повысить отказоучивость системы, так как ноды запускаются незвисимо друг от друга (бывают исключения, но о них позже).

Ноды на `ROS2` можно писать можно писать на любом языке программирования, главное чтобы для этого языка существовала библиотека-клиент.

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

Сразу к делу. Ниже два минимальных примера ноды на двух языках. Оба выполняют одно и тоже действие - публикацию строки в `/topic` с увеличивающимся значением. Публикация происходит по таймеру каждые 500мс.

Важно обратить внимание на `rclpy.spin(node)` и `rclcpp::spin(node);`, это важные строчки которые нельзя забывать. Так как `ROS2` имеет собственный планировщик задач, обработку событий ноды нужно запускать явно. Внутри функций `spin` происходит чтение новых сообщений из топиков на которые подписалась нода, счетчики таймеров и вызов соотвествующих функций-коллбеков для каждого события.

Например при создании таймера мы передаем в него функцию `timer_callback` которая будет вызываться каждый раз когда таймер отсчитает 500мс.

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

Идеалогия планировщика `ROS2` следующая - внутри коллбеков не может быть блокирующих операций, например - `time.sleep(...)`, вызов такой функции внутри коллбека вызовет остановку планировщика и он перестанет обрабатывать другие сигналы (например новое сообщение из другого топика). Это вызванно тем, что обычный `spin` обрабатывает коллебкеи последовательно и не может перейти к выполнению следующего пока не завершиться текущий. Такой тип планировщика называется - ***однопоточный кооперативный планировщик*** (работает в одном потоке и выполняет задачи по очереди).

## Продвинуте примеры

Основными языками для написания нод являются `Python` и `C++`, оба два поддерживают [ООП](https://habr.com/ru/articles/463125/) и такую возможность нельзя упускать, поэтому в больших проектах ноды создают следующим образом:

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

Подобный подход на первый взгляд кажется сложнее, но сильно упрощает жизнь когда ваша нода начинает расти. К тому же в случае `C++` такой подход позволяет создавать [composable](https://docs.ros.org/en/jazzy/Concepts/Intermediate/About-Composition.html) ноды, их можно запускать в одном процессе чтобы минизировать расходы на передачу больших сообщений между программами (например видео с камеры). В `ROS1` подобная система называлась Nodelet, но в `ROS2` этот подход был унифицирован и если правильно написать ноду вы сможете запускать ее и как обычную программу и как composable.

## Имя и неймспейс

В примерах выше мы каждый раз передавали в конструктор строку `minimal_publisher` — это ***имя ноды***, ее главный идентификатор в системе. По имени ноду находят остальные участники системы.

Второй атрибут ноды — ***неймспейс***, путь по которому нода располагается в системе. Полное имя ноды складывается из неймспейса и имени — `/неймспейс/имя`. Если неймспейс не задан, нода создается в корневом неймспейсе и ее полное имя совпадает с именем: `/minimal_publisher`. В рамках одного неймспейса имена нод должны быть уникальны.

Список запущенных нод показывает команда `ros2 node list`:

```bash
$ ros2 node list
/minimal_publisher
```

Имя и неймспейс решают разные задачи:

- **Имя** отличает ноды друг от друга. Например, в `clover2` драйвер камеры `camera_node` запускается дважды — под именами `main_camera` и `front_camera` для основной и передней камер. Исполняемый файл один, а ноды в системе получаются разные.
- **Неймспейс** группирует ноды и изолирует их друг от друга. Все относительные имена внутри ноды (топики, сервисы) достраиваются от ее неймспейса, поэтому одинаковые ноды в разных неймспейсах не конфликтуют: нода `camera` в неймспейсе `front` публикует изображения в `/front/image_raw`, а такая же нода в неймспейсе `main` — в `/main/image_raw`.

В именах и неймспейсах допускаются латинские буквы, цифры и подчеркивания.

Заданные в коде имя и неймспейс не окончательные — их можно переопределить при запуске, не меняя код. Как именно, разберем дальше.

## Запуск нод

Нода — это исполняемый файл, установленный внутри `ROS2` пакета. Самый простой способ ее запустить:

| Команда | Что делает |
| --- | --- |
| `ros2 pkg executables <пакет>` | Показывает исполняемые файлы пакета |
| `ros2 run <пакет> <исполняемый файл>` | Запускает один исполняемый файл |

К команде запуска можно добавить `--ros-args` и правила замены (`-r`), которые переопределяют имя и неймспейс из кода:

```bash
# запустить драйвер камеры под именем front_camera
ros2 run camera_ros camera_node --ros-args -r __node:=front_camera

# запустить его же в неймспейсе /front
ros2 run camera_ros camera_node --ros-args -r __node:=camera -r __ns:=/front
```

## Параметры

***Параметры*** — это настройки ноды: у каждого параметра есть имя, тип и значение.

Параметры обязательно объявляются в коде ноды: объявление связывает имя параметра с нодой и задает значение по умолчанию. Значение, переданное извне для необъявленного параметра, не применится.

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
Тип параметра фиксируется объявлением. Попытка установить значение другого типа (например строку вместо числа) будет отклонена с ошибкой.
```

Значения по умолчанию используются, если параметр не задан другим способом. Способов передать значение несколько.

**Из командной строки.** К `ros2 run` добавляются присваивания `-p имя:=значение`:

```bash
ros2 run my_package frequency_talker --ros-args \
    -p frequency:=2.0 \
    -p message:="Привет, мир"
```

Списки передаются в кавычках и квадратных скобках, например `-p image_size:="[256, 384]"`.

**Уже запущенной ноде.** Значения можно читать и менять на ходу командами `ros2 param get` / `ros2 param set` — все такие команды собраны в статье [Команды ROS 2](Commands).

**Файлом параметров.** Когда параметров много, удобнее описать их в одном YAML-файле и передавать его целиком. В `clover2` так устроены файлы из `clover2_bringup/params`, например фрагмент `klever5.yaml`:

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

Верхний ключ — шаблон, по которому выбираются ноды: `/**` соответствует всем нодам, а `/**/aruco_tracker` — ноде с именем `aruco_tracker` в любом неймспейсе. Благодаря шаблонам один файл может задавать параметры сразу для нескольких нод. Уровень `ros__parameters` — обязательный служебный, внутри него перечисляются сами параметры.

Файл передается ноде при запуске через `--params-file`:

```bash
ros2 run my_package frequency_talker --ros-args --params-file my_params.yaml
```

## `Lifecycle`

`ROS2` позиционируется как production-ready система, поэтому важно не уметь не только запустить программу, но и уметь контроллировать ее состояние и работоспособность. Для этого кроме обычных `rclcpp::Node` и `rclpy.Node` существуют [`Lifecycle`](https://design.ros2.org/articles/node_lifecycle.html) ноды. Lifecycle - жизненный цикл, они работают по жесткой машине состояний.

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

Такая система позволяет с помощью отдельной ноды контроллировать настройку, запуск и непредвиденное поведение компонентов которые построены на базе Lifecycle. Подобные ноды могут быть в следующих состояниях:

- `Unconfigured`
- `Inactive`
- `Active`
- `Finalized`

Вызывать переходы между состояними можно с помощью сервиса `/<имя ноды>/change_state` (эти сервисы создаются автоматически).
