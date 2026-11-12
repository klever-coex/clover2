"""FCU client over the clover2_fcu bindings."""

from collections.abc import Callable

from clover2_fcu import Client, Commands, SystemState


class FCUClient:
    """Commands and telemetry over the new fcu client (private spun node)."""

    def __init__(self) -> None:
        """Connect to the bridge."""
        self._client = Client()

    @property
    def commands(self) -> Commands:
        """Commands provider (ack-bool callback contract)."""
        return self._client["commands"]

    @property
    def system_state(self) -> SystemState:
        """System state provider."""
        return self._client["system_state"]

    def arm(
        self, cb: Callable[[bool], None], run_preflight_checks: bool = False
    ) -> None:
        """Arm the vehicle; cb(ok) fires on the FCU ack."""
        self.commands.arm(cb, run_preflight_checks)

    def disarm(self, cb: Callable[[bool], None]) -> None:
        """Disarm the vehicle; cb(ok) fires on the FCU ack."""
        self.commands.disarm(cb)

    def set_mode(self, cb: Callable[[bool], None], mode: str) -> None:
        """Switch the flight mode; cb(ok) fires on the FCU ack."""
        self.commands.set_mode(cb, mode)

    def takeoff(
        self,
        cb: Callable[[bool], None],
        altitude: float | None = None,
        heading: float | None = None,
    ) -> None:
        """Trigger an automatic takeoff; cb(ok) is the FCU ack."""
        self.commands.takeoff(cb, altitude=altitude, heading=heading)

    def land(self, cb: Callable[[bool], None]) -> None:
        """Trigger an automatic landing; cb(ok) is the FCU ack."""
        self.commands.land(cb)

    def rtl(self, cb: Callable[[bool], None]) -> None:
        """Trigger return-to-launch; cb(ok) is the FCU ack."""
        self.commands.rtl(cb)

    def is_armed(self) -> bool:
        """Whether the vehicle is armed (raises until state data arrives)."""
        return bool(self.system_state.last()["armed"])

    def flight_mode(self) -> str:
        """Current flight mode name (raises until state data arrives)."""
        return str(self.system_state.last()["mode"])
