from clover2_common import wait_future
from clover2_led_msgs.msg import Color, LedFrame
from clover2_led_msgs.srv import GetCurrentFrame, GetDriverInfo, StartAnimation
from rclpy.node import Node


def _make_color(r: int, g: int, b: int) -> Color:
    color = Color()
    color.r = r
    color.g = g
    color.b = b
    return color


class LEDClient:
    def __init__(self, node: Node, base_path: str = ""):
        self._node = node
        self._logger = self._node.get_logger().get_child("led_client")
        self._base_path = base_path

        self._valid = False
        self._led_count = 0
        self._max_fps = 0.0

        self._frame_pub = node.create_publisher(LedFrame, self._topic("led_frame"), 5)
        self._get_info_client = node.create_client(
            GetDriverInfo, self._service("get_driver_info")
        )
        self._get_frame_client = node.create_client(
            GetCurrentFrame, self._service("get_current_frame")
        )
        self._start_animation_client = node.create_client(
            StartAnimation, self._service("start_animation")
        )

        self._update_driver_info()

    @property
    def valid(self) -> bool:
        return self._valid

    @property
    def led_count(self) -> int:
        return self._led_count

    @property
    def max_fps(self) -> float:
        return self._max_fps

    def get_current_frame(
        self, timeout: float = 1.0
    ) -> tuple[list[tuple[int, int, int]], float]:
        if not self._get_frame_client.wait_for_service(timeout):
            raise RuntimeError(
                f"{self._get_frame_client.srv_name} service not available"
            )

        result = wait_future(
            self._get_frame_client.call_async(GetCurrentFrame.Request()), timeout
        )
        if not result:
            self._node.get_logger().error("Service did not respond")
            return [], 0.0
        if not result.success:
            self._node.get_logger().error(
                f"`get_current_frame`: {result.message}"
            )
            return [], 0.0

        return [(color.r, color.g, color.b) for color in result.colors], result.brightness

    def send_frame(self, colors: list[tuple[int, int, int]], brightness: float = 1.0):
        msg = LedFrame()
        msg.brightness = float(brightness)
        msg.colors = [_make_color(*color) for color in colors]
        self._frame_pub.publish(msg)

    def clear(self):
        self.send_frame([(0, 0, 0)] * self._led_count, brightness=0.0)

    def fill(self, r: int, g: int, b: int):
        self.send_frame([(r, g, b)] * self._led_count)

    def solid_color(
        self, r: int, g: int, b: int, brightness: float = 1.0, duration: float = 0.0
    ):
        req = StartAnimation.Request()
        req.animation_name = "solid_color"
        req.brightness = float(brightness)
        req.duration = float(duration)
        req.colors = [_make_color(r, g, b)]
        self._call_animation(req)

    def blink(
        self,
        r: int,
        g: int,
        b: int,
        period: float = 1.0,
        brightness: float = 1.0,
        duration: float = 0.0,
    ):
        req = StartAnimation.Request()
        req.animation_name = "blink"
        req.brightness = float(brightness)
        req.period = float(period)
        req.duration = float(duration)
        req.colors = [_make_color(r, g, b)]
        self._call_animation(req)

    def rainbow(
        self, period: float = 2.0, brightness: float = 1.0, duration: float = 0.0
    ):
        req = StartAnimation.Request()
        req.animation_name = "rainbow"
        req.brightness = float(brightness)
        req.period = float(period)
        req.duration = float(duration)
        self._call_animation(req)

    def _topic(self, name: str) -> str:
        return name if not self._base_path else f"{self._base_path}/{name}"

    def _service(self, name: str) -> str:
        return self._topic(name)

    def _call_animation(self, req: StartAnimation.Request):
        if not self._start_animation_client.wait_for_service(1.0):
            raise RuntimeError("start_animation service not available")

        result = wait_future(self._start_animation_client.call_async(req), timeout=1.0)
        if not result:
            self._node.get_logger().error("Service did not respond")
        elif not result.success:
            self._node.get_logger().error(f"`start_animation`: {result.message}")

    def _update_driver_info(self):
        if not self._get_info_client.wait_for_service(1.0):
            raise RuntimeError(
                f"{self._get_info_client.srv_name} service not available"
            )

        result = wait_future(
            self._get_info_client.call_async(GetDriverInfo.Request()), timeout=1.0
        )
        if not result:
            self._node.get_logger().error("Service did not respond")
            return
        if not result.success:
            self._node.get_logger().error(f"`get_driver_info`: {result.message}")
            return

        self._led_count = result.led_count
        self._max_fps = result.max_fps
        self._valid = True
        self._logger.debug(
            f"Driver info: led_count={self._led_count}, max_fps={self._max_fps:.1f}"
        )