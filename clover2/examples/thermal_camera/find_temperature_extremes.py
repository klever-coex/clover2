#!/usr/bin/env python3

import numpy as np
import rclpy
from geometry_msgs.msg import Point, PointStamped
from rclpy.node import Node
from sensor_msgs.msg import Image


class ThermalExtremes(Node):
    def __init__(self):
        super().__init__("thermal_temperature_extremes")

        # все publisher имеют тип PointStamped
        self.min_publisher = self.create_publisher(PointStamped, "/thermal_camera/min_temperature", 1)
        self.max_publisher = self.create_publisher(PointStamped, "/thermal_camera/max_temperature", 1)
        self.center_publisher = self.create_publisher(PointStamped, "/thermal_camera/center_temperature", 1)

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

    def convert_to_point_stamped(self, header, x, y, z) -> PointStamped:
        msg = PointStamped()
        msg.header = header
        msg.point = Point(x=float(x), y=float(y), z=float(z))
        return msg

    def image_callback(self, msg):
        # callback получает каждый новый кадр из /thermal_camera/image_raw

        # проверяем, что кадр нормально извлечен
        raw = self.raw16_from_image(msg)
        if raw is None:
            self.get_logger().error("raw frame is too small for uint16 image", throttle_duration_sec=5)
            return

        # проверенный кадр 384x256 делится по строкам на две половины:
        # raw[:192, :] - верхняя половина, ИК-изображение для визуализации
        # raw[192:, :] - нижняя половина, матрица температур
        half_rows = msg.height // 2
        temperature_raw = raw[half_rows:, :]

        # формула преобразования raw-значений в градусы Цельсия
        # 64.0 - масштаб тепловизора, -273.15 переводит Кельвины в Цельсии
        temperature_c = temperature_raw.astype(np.float32) / 64.0 - 273.15

        # np.argmin/argmax возвращают индекс в плоском массиве
        # np.unravel_index переводит его обратно в координаты строки/столбца
        min_index = int(np.argmin(temperature_c))
        max_index = int(np.argmax(temperature_c))
        min_y, min_x = np.unravel_index(min_index, temperature_c.shape)
        max_y, max_x = np.unravel_index(max_index, temperature_c.shape)

        # центральная точка нужна как простой пример чтения температуры в заданном пикселе
        center_y = temperature_c.shape[0] // 2
        center_x = temperature_c.shape[1] // 2

        # берем сами значения температуры в найденных точках
        min_temp = temperature_c[min_y, min_x]
        max_temp = temperature_c[max_y, max_x]
        center_temp = temperature_c[center_y, center_x]

        # публикуем сразу в PointStamped:
        # /thermal/min_temperature, /thermal/max_temperature, /thermal/center_temperature
        # в каждой точке z содержит температуру в градусах Цельсия
        self.min_publisher.publish(self.convert_to_point_stamped(msg.header, min_x, min_y, min_temp))
        self.max_publisher.publish(self.convert_to_point_stamped(msg.header, max_x, max_y, max_temp))
        self.center_publisher.publish(self.convert_to_point_stamped(msg.header, center_x, center_y, center_temp))


def main():
    rclpy.init()
    node = ThermalExtremes()

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
