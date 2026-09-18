#!/usr/bin/env python3

import cv2
import numpy as np
import rclpy
from cv_bridge import CvBridge
from rclpy.node import Node
from sensor_msgs.msg import Image


class ThermalColormapPub(Node):
    def __init__(self):
        super().__init__("thermal_colormap_publisher")

        self.bridge = CvBridge()

        # publisher имеет тип Image
        self.colormap_publisher = self.create_publisher(Image, "/thermal_camera/image_colormap", 1)

        # подписываемся на raw-кадр тепловизора
        self.subscription = self.create_subscription(Image, "/thermal_camera/image_raw", self.image_callback, 1)

    def convert_to_bgr(self, msg):
        # cv_bridge сам корректно преобразует yuv422_yuy2 в bgr8
        return self.bridge.imgmsg_to_cv2(msg, desired_encoding="bgr8")

    def normalize_to_u8(self, image):
        # colormap принимает 8-битную картинку, поэтому приводим raw к 0-255
        # percentile 1 и 99 помогают не терять контраст из-за шумных пикселей
        low, high = np.percentile(image, (1, 99))
        if high <= low:
            # если весь кадр одинаковый, возвращаем черную картинку
            return np.zeros(image.shape, dtype=np.uint8)

        # переносим диапазон low/high в 0-255
        normalized = (image.astype(np.float32) - low) * 255.0 / (high - low)
        return np.clip(normalized, 0, 255).astype(np.uint8)

    def convert_to_colormap(self, msg, image):
        # проверенный кадр 384x256 делится по строкам на две половины:
        # image_gray[:192, :] - верхняя половина, ИК-изображение для визуализации
        # image_gray[192:, :] - нижняя половина, матрица температур
        image_gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        half_rows = msg.height // 2
        thermal_image_raw = image_gray[:half_rows, :]

        # усиливаем контраст и накладываем цветовую карту opencv
        thermal_image_u8 = self.normalize_to_u8(thermal_image_raw)
        return cv2.applyColorMap(thermal_image_u8, cv2.COLORMAP_PLASMA)

    def image_callback(self, msg):
        # callback получает каждый новый кадр из /thermal_camera/image_raw

        # проверяем, что кадр нормально преобразован в обычное изображение
        try:
            image_bgr = self.convert_to_bgr(msg)
        except Exception as error:
            self.get_logger().warning(f"failed to convert image: {error}")
            return

        colormap = self.convert_to_colormap(msg, image_bgr)

        # bgr8 используется потому, что opencv хранит цветные изображения в bgr
        color_msg = self.bridge.cv2_to_imgmsg(colormap, encoding="bgr8")
        color_msg.header = msg.header
        self.colormap_publisher.publish(color_msg)


def main():
    rclpy.init()
    node = ThermalColormapPub()

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
