from clover2_display_msgs.srv import GetDriverInfo
from rclpy.node import Node
from sensor_msgs.msg import Image

from ..utils import wait_future


class DisplayClient:
    def __init__(self, node: Node, base_path: str = ''):
        self._node = node
        self._logger = self._node.get_logger().get_child('display_client')
        self._base_path = base_path

        self._valid = False
        self._width = 0
        self._height = 0
        self._max_fps = 0.0
        self._supported_encodings: list[str] = []

        self._image_pub = node.create_publisher(Image, self._topic('image'), 5)
        self._get_info_client = node.create_client(
            GetDriverInfo, self._service('get_driver_info')
        )

        self._update_driver_info()

    @property
    def valid(self) -> bool:
        return self._valid

    @property
    def width(self) -> int:
        return self._width

    @property
    def height(self) -> int:
        return self._height

    @property
    def max_fps(self) -> float:
        return self._max_fps

    @property
    def supported_encodings(self) -> list[str]:
        return self._supported_encodings.copy()

    def send_image(self, image: Image):
        self._image_pub.publish(image)

    def _topic(self, name: str) -> str:
        if not self._base_path:
            return name
        return f'{self._base_path}/{name}'

    def _service(self, name: str) -> str:
        return self._topic(name)

    def _update_driver_info(self):
        if not self._get_info_client.wait_for_service(1.0):
            raise RuntimeError(
                f'{self._get_info_client.srv_name} service not available'
            )

        req = GetDriverInfo.Request()
        future = self._get_info_client.call_async(req)
        result = wait_future(future, timeout=1.0)

        if not result:
            self._node.get_logger().error('Service not response')
            return

        if not result.success:
            self._node.get_logger().error(f'`get_driver_info`: {result.message}')
            return

        self._width = result.width
        self._height = result.height
        self._max_fps = result.max_fps
        self._supported_encodings = list(result.supported_encodings)
        self._valid = True
        self._logger.debug(
            'Driver info: '
            f'{self._width}x{self._height}, max_fps={self._max_fps:.1f}, '
            f'supported_encodings={self._supported_encodings}'
        )
