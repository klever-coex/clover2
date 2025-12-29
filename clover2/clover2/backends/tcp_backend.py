"""TCP backend implementation - no ROS dependencies."""

import socket
import threading
import time
import json
from typing import Optional

from ..core.interfaces import DroneBackend
from ..core.types import Pose


class TcpBackend(DroneBackend):
    """TCP/UDP socket-based backend for drone control."""

    def __init__(self, host: str = "127.0.0.1", port: int = 5760, timeout: float = 1.0):
        """
        Initialize TCP backend.

        Args:
            host: Server hostname or IP address
            port: Server port number
            timeout: Socket timeout in seconds
        """
        self.host = host
        self.port = port
        self.timeout = timeout
        self._socket: Optional[socket.socket] = None
        self._running = False
        self._current_pose: Optional[Pose] = None
        self._lock = threading.Lock()

    def start(self) -> None:
        """Start the TCP backend connection."""
        try:
            self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self._socket.settimeout(self.timeout)
            self._socket.connect((self.host, self.port))
            self._running = True

            # Start telemetry receiver thread
            thread = threading.Thread(target=self._telemetry_loop, daemon=True)
            thread.start()
        except (socket.error, OSError) as e:
            raise ConnectionError(f"Failed to connect to {self.host}:{self.port}: {e}")

    def _telemetry_loop(self) -> None:
        """Background loop to receive telemetry updates."""
        buffer = b""
        while self._running and self._socket:
            try:
                data = self._socket.recv(1024)
                if not data:
                    break

                buffer += data
                # Try to parse JSON messages (simple protocol)
                while b"\n" in buffer:
                    line, buffer = buffer.split(b"\n", 1)
                    try:
                        msg = json.loads(line.decode("utf-8"))
                        if msg.get("type") == "telemetry":
                            self._update_pose(msg)
                    except (json.JSONDecodeError, UnicodeDecodeError):
                        pass
            except socket.timeout:
                continue
            except (socket.error, OSError):
                break

    def _update_pose(self, msg: dict) -> None:
        """Update current pose from telemetry message."""
        with self._lock:
            self._current_pose = Pose(
                x=msg.get("x", 0.0),
                y=msg.get("y", 0.0),
                z=msg.get("z", 0.0),
                yaw=msg.get("yaw", 0.0),
                frame_id="map",
            )

    def _send_command(self, command: dict) -> bool:
        """Send a command to the server."""
        if not self._socket or not self._running:
            return False

        try:
            msg = json.dumps(command).encode("utf-8") + b"\n"
            self._socket.sendall(msg)
            return True
        except (socket.error, OSError):
            return False

    def arm(self, value: bool) -> bool:
        """Arm or disarm the drone."""
        return self._send_command({"type": "arm", "value": value})

    def land(self) -> bool:
        """Command the drone to land."""
        return self._send_command({"type": "land"})

    def set_target_pose(self, pose: Pose) -> bool:
        """Set the target pose for the drone."""
        return self._send_command(
            {
                "type": "set_target_pose",
                "x": pose.x,
                "y": pose.y,
                "z": pose.z,
                "yaw": pose.yaw,
                "frame_id": pose.frame_id,
            }
        )

    def get_telemetry(self) -> Optional[Pose]:
        """Get current drone telemetry."""
        with self._lock:
            if self._current_pose:
                return Pose(
                    x=self._current_pose.x,
                    y=self._current_pose.y,
                    z=self._current_pose.z,
                    yaw=self._current_pose.yaw,
                    frame_id=self._current_pose.frame_id,
                )
            return None

    def stop(self) -> None:
        """Stop the backend and close connections."""
        self._running = False
        if self._socket:
            try:
                self._socket.close()
            except (socket.error, OSError):
                pass
            self._socket = None

