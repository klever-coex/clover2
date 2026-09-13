#!/usr/bin/env python3

import numpy as np
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from std_msgs.msg import String


class ThermalRawSub(Node):
    def __init__(self):
        super().__init__("thermal_raw_subscriber")

        # publisher имеет тип String
        self.publisher = self.create_publisher(String, "/thermal_camera/status", 1)

        # подписываемся на raw-кадр тепловизора
        self.subscription = self.create_subscription(Image, "/thermal_camera/image_raw", self.image_callback, 1)

    def raw16_from_image(self, msg):
        # входной кадр приходит как sensor_msgs/msg/Image с encoding yuv422_yuy2
        # один пиксель занимает 2 байта, поэтому читаем data как uint16
        expected_size = msg.height * msg.width * 2

        # если данных меньше ожидаемого размера, кадр пропускаем
        if len(msg.data) < expected_size:
            return None

        # reshape превращает линейный массив в матрицу
        return np.frombuffer(msg.data[:expected_size], dtype="<u2").reshape(msg.height, msg.width)

    def convert_to_status(self, msg, raw):
        # проверенный кадр 384x256 делится по строкам на две половины:
        # raw[:192, :] - верхняя половина, ИК-изображение для визуализации
        # raw[192:, :] - нижняя половина, матрица температур
        half_rows = msg.height // 2

        # shape показывает размер верхней половины изображения
        image_part = raw[:half_rows, :]
        temperature_part = raw[half_rows:, :]

        text = (
            f"frame {msg.width}x{msg.height}, encoding={msg.encoding}, "
            f"image_shape={image_part.shape[1]}x{image_part.shape[0]}, "
            f"temperature_shape={temperature_part.shape[1]}x{temperature_part.shape[0]}"
        )
        return String(data=text)

    def image_callback(self, msg):
        # callback получает каждый новый кадр из /thermal_camera/image_raw

        # проверяем, что кадр нормально извлечен
        raw = self.raw16_from_image(msg)
        if raw is None:
            self.get_logger().error("raw frame is too small for uint16 image", throttle_duration_sec=5)
            return

        # публикуем короткую строку со статусом кадра
        self.publisher.publish(self.convert_to_status(msg, raw))


def main():
    rclpy.init()
    node = ThermalRawSub()

    try:
        # spin запускает обработку кадров
        rclpy.spin(node)
    except KeyboardInterrupt:
        # ошибка KeyboardInterrupt возникает после нажатия ctrl+c
        pass
    finally:
        # освобождаем ресурсы node
        node.destroy_node()

        # в ROS 2 shutdown мог уже выполниться после ctrl+c
        # поэтому перед shutdown проверяем, что контекст еще активен
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == "__main__":
    main()
