from dataclasses import dataclass, field


@dataclass
class TrackerConfig:
    enable: bool = field(default=True)
