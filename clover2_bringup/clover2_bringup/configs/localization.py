from dataclasses import dataclass, field

from .map_server import MapServerConfig
from .tracker import TrackerConfig


@dataclass
class LocalizationConfig:
    enable: bool = field(default=False)
    map_server: MapServerConfig = field(default_factory=MapServerConfig)
    tracker: TrackerConfig = field(default_factory=TrackerConfig)
