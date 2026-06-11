from dataclasses import dataclass, field


@dataclass
class CameraConfig:
    enable: bool = field(default=False)
    feature_detector: bool = field(default=False)
