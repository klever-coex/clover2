from dataclasses import dataclass, field


@dataclass
class MapServerConfig:
    enable: bool = field(default=True)
    map_filename: str = field(default="example-1.yaml")
