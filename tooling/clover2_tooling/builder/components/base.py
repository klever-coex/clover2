from abc import ABC, abstractmethod
import pathlib


class ComponentBase(ABC):
    @abstractmethod
    async def copy_to(self, src: pathlib.Path, dest: pathlib.Path) -> None: ...

    @abstractmethod
    async def execute(self, cmd: str, check: bool = True) -> None: ...

    @abstractmethod
    async def __aenter__(self) -> "ComponentBase": ...

    @abstractmethod
    async def __aexit__(self, exc_type, exc_val, exc_tb) -> None: ...
