from enum import Enum
from threading import Event

import rclpy
import rclpy.time
from rclpy.node import Node

import tf2_ros
import tf2_geometry_msgs
from tf_transformations import quaternion_from_euler
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros.transform_broadcaster import TransformBroadcaster
from tf2_ros.static_transform_broadcaster import StaticTransformBroadcaster
from geometry_msgs.msg import TransformStamped, PointStamped, Point, Vector3, Quaternion


from .client import ClientBase
from .data import GoToSetpoint, FlightMode


class OffboardMode(Enum):
    NONE = "none"
    SPEED = "speed"
    POSITION = "position"


class OffboardHelper:
    def __init__(self, node: Node, client: ClientBase):
        self._logger = node.get_logger().get_child("offboard")

        self.frame_id = "base_link"
        self._map_id = "map"
        self._client = client
        self._current_mode = OffboardMode.NONE

        self._position_setpoint = GoToSetpoint(None, None, None, None, None)

        self._offboard_timer = node.create_timer(
            1.0 / 50.0, self._offboard_timer_callback
        )

        self._tf2_broadcaster = StaticTransformBroadcaster(node)
        self._clock = node.get_clock()

        self._tf2_buffer = Buffer()
        self._tf2_listener = TransformListener(self._tf2_buffer, node)

        self._pos = Point()
        self._vel = Vector3()
        self._acc = Vector3()
        self._yaw = None
        self._yaw_rate = None

        self._offboard_first_send = Event()

        self._logger.info("Wait first offboard publish")
        self._offboard_first_send.wait(timeout=1.0)

        self._logger.debug("Setting mode...")
        if not self._client.set_mode(FlightMode.OFFBOARD):
            raise Exception("Mode change fail")

        self._logger.info("Offboard helper started")

    def _offboard_timer_callback(self):
        self._update_offboard()
        self._update_tasks()

    def _update_offboard(self):
        match (self._current_mode):
            case OffboardMode.POSITION:
                self._send_position()
            case OffboardMode.SPEED:
                self._send_speed()
            case OffboardMode.NONE:
                self._client.send_raw_body_impl(
                    vel=Vector3(),
                    acc=Vector3(x=None, y=None, z=None),
                    yaw=None,
                    yaw_rate=0.0,
                )
            case _:
                pass

        self._offboard_first_send.set()

    def _update_tasks(self):
        pass

    def _send_position(self):
        self._client.send_raw_local_impl(
            self._pos, self._vel, self._acc, self._yaw, self._yaw_rate
        )

    def _send_speed(self):
        pass

    def _publish_tf_setpoint(self, point: PointStamped, yaw: float):
        offboard_tf = TransformStamped()

        offboard_tf.header.frame_id = self._map_id
        offboard_tf.header.stamp = self._clock.now().to_msg()
        offboard_tf._child_frame_id = "target_position"

        offboard_tf.transform.translation.x = point.point.x
        offboard_tf.transform.translation.y = point.point.y
        offboard_tf.transform.translation.z = point.point.z

        q = quaternion_from_euler(0.0, 0.0, yaw)
        offboard_tf.transform.rotation.x = q[0]
        offboard_tf.transform.rotation.y = q[1]
        offboard_tf.transform.rotation.z = q[2]
        offboard_tf.transform.rotation.w = q[3]

        self._tf2_broadcaster.sendTransform(offboard_tf)

    def move(
        self,
        pos: Point,
        vel: Vector3,
        yaw: float,
        yaw_rate: float,
        frame_id: str = "map",
    ):
        self._current_mode = OffboardMode.POSITION

        state = self._client.get_state()
        yaw = state.yaw() if yaw is None else yaw

        if frame_id == "base_link":
            pos.x = 0.0 if pos.x is None else pos.x
            pos.y = 0.0 if pos.y is None else pos.y
            pos.z = 0.0 if pos.z is None else pos.z

        point_stamped = PointStamped()
        point_stamped.header.stamp = self._clock.now().to_msg()
        point_stamped.header.frame_id = frame_id
        point_stamped.point.x = pos.x
        point_stamped.point.y = pos.y
        point_stamped.point.z = pos.z

        self._logger.info(f"Input point in {frame_id}: ({pos.x}, {pos.y}, {pos.z})")

        if frame_id != self._map_id:
            try:
                transform = self._tf2_buffer.lookup_transform(
                    self._map_id,
                    frame_id,
                    rclpy.time.Time(),
                    timeout=rclpy.duration.Duration(seconds=1.0),
                )

                self._logger.info(
                    f"Transform from {frame_id} to map: "
                    f"translation=({transform.transform.translation.x}, "
                    f"{transform.transform.translation.y}, "
                    f"{transform.transform.translation.z})"
                )

                transformed_point_stamped = tf2_geometry_msgs.do_transform_point(
                    point_stamped, transform
                )

                pos = transformed_point_stamped.point
                self._publish_tf_setpoint(transformed_point_stamped, yaw)

                self._logger.debug(
                    f"Transformed point in map: " f"({pos.x}, {pos.y}, {pos.z})"
                )

            except tf2_ros.LookupException as ex:
                self._logger.error(
                    f"Could not lookup transform from {frame_id} to map: {ex}"
                )
                return
            except tf2_ros.ExtrapolationException as ex:
                self._logger.error(f"Transform extrapolation failed: {ex}")
                return
            except tf2_ros.ConnectivityException as ex:
                self._logger.error(f"Transform connectivity error: {ex}")
                return

        self._pos = pos
        self._vel = vel
        self._yaw = yaw
        self._yaw_rate = yaw_rate
