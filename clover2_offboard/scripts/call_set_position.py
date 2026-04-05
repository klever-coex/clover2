#!/usr/bin/env python3

import argparse
import math
import sys

import rclpy
from clover2_offboard_msgs.srv import Navigate, SetPosition
from geometry_msgs.msg import Pose
from std_msgs.msg import Header


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Call /clover2_offboard_server/set_position"
    )
    parser.add_argument(
        "--service",
        default="/offboard_server/navigate",
        help="full service name",
    )
    parser.add_argument("--frame-id", default="map", help="Header.frame_id")
    parser.add_argument("--x", type=float, default=float("nan"))
    parser.add_argument("--y", type=float, default=float("nan"))
    parser.add_argument("--z", type=float, default=float("nan"))
    parser.add_argument("--yaw", type=float, default=float("nan"), help="radians")
    args = parser.parse_args()

    rclpy.init()
    node = rclpy.create_node("call_set_position_client")
    client = node.create_client(Navigate, args.service)

    if not client.wait_for_service(timeout_sec=5.0):
        node.get_logger().error("Service not available: %s", args.service)
        rclpy.shutdown()
        return 1

    req = Navigate.Request()
    req.header = Header()
    req.header.frame_id = args.frame_id
    req.header.stamp = node.get_clock().now().to_msg()

    cy = math.cos(args.yaw * 0.5)
    sy = math.sin(args.yaw * 0.5)
    req.speed = 0.5
    req.pose = Pose()
    req.pose.position.x = args.x
    req.pose.position.y = args.y
    req.pose.position.z = args.z
    req.pose.orientation.x = 0.0
    req.pose.orientation.y = 0.0
    req.pose.orientation.z = sy
    req.pose.orientation.w = cy

    future = client.call_async(req)
    rclpy.spin_until_future_complete(node, future)

    try:
        resp = future.result()
        node.get_logger().info(f"resp: {resp.message}")
    except Exception as e:  # noqa: BLE001
        node.get_logger().error("Service call failed: %s", e)
        rclpy.shutdown()
        return 1

    # node.get_logger().info("success=%s message=%s", resp.success, resp.message)
    rclpy.shutdown()
    return 0 if resp.success else 2


if __name__ == "__main__":
    sys.exit(main())
