from clover2.client.client_base import *
from clover2.data import *
from clover2 import utils

import rclpy
import rclpy.qos
from rclpy.node import Node
from sensor_msgs.msg import BatteryState
from mavros_msgs.msg import State, PositionTarget
from mavros_msgs.srv import SetMode, CommandTOL, CommandBool
from geometry_msgs.msg import PoseStamped, TwistStamped


class ClientMavros(ClientBase):
    _type = ClientType.MAVROS

    def __init__(self, node: Node):
        ClientBase.__init__(self)

        self._node = node

        # Current state variables
        self._current_state = None
        self._battery_state = None
        self._current_pose = None
        self._current_velocity = None

        self._initialize_ros_components(node)

        self._logger.info("Client started")

    def _initialize_ros_components(self, node: Node):
        # Create logger instance for client
        self._logger = node.get_logger().get_child("client")

        ReliabilityQoS = rclpy.qos.QoSProfile(
            reliability=rclpy.qos.ReliabilityPolicy.BEST_EFFORT,
            history=rclpy.qos.HistoryPolicy.KEEP_LAST,
            depth=10,
            durability=rclpy.qos.DurabilityPolicy.VOLATILE,
        )

        self._setpoint_raw_local = node.create_publisher(
            PositionTarget, "mavros/setpoint_raw/local", 10
        )

        self._state_sub = node.create_subscription(
            State, "mavros/state", self._state_callback, ReliabilityQoS
        )

        self._battery_sub = node.create_subscription(
            BatteryState, "mavros/battery", self._battery_callback, ReliabilityQoS
        )

        self._local_pose_sub = node.create_subscription(
            PoseStamped,
            "mavros/local_position/pose",
            self._pose_callback,
            ReliabilityQoS,
        )

        self._local_velocity_sub = node.create_subscription(
            TwistStamped,
            "mavros/local_position/velocity_local",
            self._velocity_callback,
            ReliabilityQoS,
        )

        self._set_mode_client = node.create_client(SetMode, "mavros/set_mode")
        self._takeoff_client = node.create_client(CommandTOL, "mavros/cmd/takeoff")
        self._land_client = node.create_client(CommandTOL, "mavros/cmd/land")
        self._arming_client = node.create_client(CommandBool, "mavros/cmd/arming")

        self._logger.info("Waiting for MAVROS services...")

        self._set_mode_client.wait_for_service()
        self._takeoff_client.wait_for_service()
        self._land_client.wait_for_service()
        self._arming_client.wait_for_service()

    def _state_callback(self, msg: State):
        self._current_state = msg

    def _battery_callback(self, msg: BatteryState):
        self._battery_state = msg

    def _pose_callback(self, msg: PoseStamped):
        self._current_pose = msg

    def _velocity_callback(self, msg: TwistStamped):
        self._current_velocity = msg

    def is_armed(self) -> bool:
        if not self._current_state:
            return False

        return self._current_state.armed

    def get_mode(self) -> FlightMode:
        if not self._current_state:
            return FlightMode.UNKNOWN

        mode_str = self._current_state.mode
        current_mode = self._map_mavros_mode_to_enum(mode_str)

        return current_mode

    def set_mode(self, mode: FlightMode) -> bool:
        if not self._set_mode_client:
            return False

        try:
            mode_str = self._map_enum_to_mavros_mode(mode)

            request = SetMode.Request()
            request.custom_mode = mode_str

            response = self._set_mode_client.call(request, timeout_sec=1.0)
            return response.mode_sent or False

        except Exception as e:
            self._logger.error(f"Service call fail: {e}")
            return False

    def get_state(self) -> DroneState:

        position = (0.0, 0.0, 0.0)
        orientation = (0.0, 0.0, 0.0)
        velocity = (0.0, 0.0, 0.0)

        if self._current_pose:
            pose = self._current_pose.pose.position
            position = (pose.x, pose.y, pose.z)

            orient = self._current_pose.pose.orientation
            orientation = self._quaternion_to_euler(
                orient.x, orient.y, orient.z, orient.w
            )

        if self._current_velocity:
            vel = self._current_velocity.twist.linear
            velocity = (vel.x, vel.y, vel.z)

        return DroneState(position=position, orientation=orientation, velocity=velocity)

    def get_battery(self) -> Battery:
        if not self._battery_state:
            return Battery()

        return Battery(
            voltage=self._battery_state.voltage,
            percentage=self._battery_state.percentage,
            current=self._battery_state.current,
        )

    def turn_motors_on_impl(self) -> Future:
        try:
            request = CommandBool.Request()
            request.value = True

            future = self._arming_client.call_async(request)
            return future

        except Exception as e:
            self._logger.error(f"Arming error: {str(e)}")
            return None

    def turn_motors_off_impl(self) -> Future:
        try:
            request = CommandBool.Request()
            request.value = False

            future = self._arming_client.call_async(request)
            return future

        except Exception as e:
            self._logger.error(f"Arming error: {str(e)}")
            return None

    def takeoff(self, z: float | int, timeout: float | int = 10.0) -> bool:
        try:
            request = CommandTOL.Request()
            request.altitude = z

            response = self._takeoff_client.call(request, timeout_sec=1.0)
            return response.success or False

        except Exception as e:
            self._logger.error(f"Takeoff error: {e}")
            return False

    def land(self, timeout: float | int = 10.0) -> bool:
        try:
            future = self._land_client.call_async(CommandTOL.Request())
            rclpy.spin_until_future_complete(self._node, future, timeout_sec=timeout)

            response = self._land_client.call(CommandTOL.Request(), timeout_sec=1.0)
            return response.success or False

        except Exception as e:
            self._logger.error(f"Land error: {str(e)}")
            return False

    def _goto_impl(
        self,
        x: SetpointType,
        y: SetpointType,
        z: SetpointType,
        yaw: SetpointType,
        frame_id: str,
        speed: SetpointType,
    ):
        pass

    def send_state_impl(self, setpoint: GoToSetpoint, frame_id: str):
        request = PositionTarget()

        request.header.stamp = self._node.get_clock().now().to_msg()

        request.header.frame_id = frame_id if frame_id else "base_link"
        request.coordinate_frame = PositionTarget.FRAME_LOCAL_NED

        request.type_mask = (
            PositionTarget.IGNORE_VX
            | PositionTarget.IGNORE_VY
            | PositionTarget.IGNORE_VZ
            | PositionTarget.IGNORE_AFX
            | PositionTarget.IGNORE_AFY
            | PositionTarget.IGNORE_AFZ
            | PositionTarget.IGNORE_YAW
            | PositionTarget.IGNORE_YAW_RATE
        )

        if setpoint.x is not None:
            request.type_mask &= ~PositionTarget.IGNORE_PX
            request.position.x = float(setpoint.x)

        if setpoint.y is not None:
            request.type_mask &= ~PositionTarget.IGNORE_PY
            request.position.y = float(setpoint.y)

        if setpoint.z is not None:
            request.type_mask &= ~PositionTarget.IGNORE_PZ
            request.position.z = float(setpoint.z)

        # Set yaw or yaw rate
        if setpoint.yaw is not None:
            request.type_mask &= ~PositionTarget.IGNORE_YAW
            request.yaw = float(setpoint.yaw)
        else:
            request.type_mask &= ~PositionTarget.IGNORE_YAW_RATE
            request.yaw_rate = 0.0

        self._setpoint_raw_local.publish(request)

    def _map_mavros_mode_to_enum(self, mode_str: str) -> FlightMode:
        mode_mapping = {
            "MANUAL": FlightMode.MANUAL,
            "STABILIZE": FlightMode.STABILIZE,
            "ALT_HOLD": FlightMode.ALT_HOLD,
            "POSCTL": FlightMode.POSITION,
            "OFFBOARD": FlightMode.OFFBOARD,
            "AUTO.RTL": FlightMode.RETURN_TO_LAUNCH,
            "AUTO.LAND": FlightMode.LAND,
            "AUTO.LOITER": FlightMode.HOLD,
        }
        return mode_mapping.get(mode_str, FlightMode.UNKNOWN)

    def _map_enum_to_mavros_mode(self, mode: FlightMode) -> str:
        mode_mapping = {
            FlightMode.MANUAL: "MANUAL",
            FlightMode.STABILIZE: "STABILIZE",
            FlightMode.ALT_HOLD: "ALT_HOLD",
            FlightMode.POSITION: "POSCTL",
            FlightMode.OFFBOARD: "OFFBOARD",
            FlightMode.RETURN_TO_LAUNCH: "AUTO.RTL",
            FlightMode.LAND: "AUTO.LAND",
            FlightMode.HOLD: "AUTO.LOITER",
            FlightMode.UNKNOWN: "UNKNOWN",
        }
        return mode_mapping.get(mode, "UNKNOWN")

    def _quaternion_to_euler(self, x: float, y: float, z: float, w: float) -> tuple:
        import math

        t0 = +2.0 * (w * x + y * z)
        t1 = +1.0 - 2.0 * (x * x + y * y)
        roll_x = math.atan2(t0, t1)

        t2 = +2.0 * (w * y - z * x)
        t2 = +1.0 if t2 > +1.0 else t2
        t2 = -1.0 if t2 < -1.0 else t2
        pitch_y = math.asin(t2)

        t3 = +2.0 * (w * z + x * y)
        t4 = +1.0 - 2.0 * (y * y + z * z)
        yaw_z = math.atan2(t3, t4)

        return (roll_x, pitch_y, yaw_z)
