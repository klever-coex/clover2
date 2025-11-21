from abc import ABC, abstractmethod
import pathlib

class ComponentBase(ABC):
    def __init__(self):
        pass

    @abstractmethod
    async def copy_to(self, src: pathlib.Path, dest: pathlib.Path):
        pass

    @abstractmethod
    async def execute(self, args: list[str]):
        pass

    @abstractmethod
    async def __aenter__(self):
        pass

    @abstractmethod
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        pass
