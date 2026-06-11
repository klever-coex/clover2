from dataclasses import dataclass, field


@dataclass
class OpticalFlowConfig:
    enable: bool = field(default=False)
