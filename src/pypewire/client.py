from typing import List, Optional, Type, Any
from types import TracebackType

from pypewire._core import PWModule, PWFactory, PWNode
from . import _core  # This is the compiled C extension


class PWClient:
    def __init__(self, name: str) -> None:
        self.name = name
        self.connection: Optional[_core.PWConnection] = None

    def __enter__(self) -> "PWClient":
        self.connection = _core.PWConnection(self.name)
        return self

    def __exit__(self, t: Optional[Type[BaseException]],
                 v: Optional[BaseException],
                 b: Optional[TracebackType]) -> None:
        self.connection = None

    def create_object(self, factory_name: str, type_name: str, version: int, props: dict[str, Any]) -> None:
        if not self.connection:
            raise RuntimeError("Not connected")
        return self.connection.create_object(factory_name, type_name, version, props)

    def load_module(self, module: str, args: dict[str, Any]) -> None:
        if not self.connection:
            raise RuntimeError("Not connected")
        print('Loading module:', module)
        self.connection.create_object(
            'adapter',
            _core.PW_TYPE_INTERFACE_Node,
            _core.PW_VERSION_NODE,
            {
                # example: wrap some SPA factory
                "factory.name": "support.null-audio-sink",  # or whatever you need
                "node.name": "my-sink2",
                # "media.class": "Audio/Sink",
                # adapter-specific / node props as needed...
            }
        )

    def get_modules(self) -> list[PWModule]:
        if not self.connection:
            raise RuntimeError("Not connected")
        return self.connection.get_modules()

    def get_devices(self) -> list[Any]:
        if not self.connection:
            raise RuntimeError("Not connected")
        return self.connection.get_devices()

    def get_factories(self) -> list[PWFactory]:
        if not self.connection:
            raise RuntimeError("Not connected")
        return self.connection.get_factories()

    def get_nodes(self) -> list[PWNode]:
        if not self.connection:
            raise RuntimeError("Not connected")
        return self.connection.get_nodes()
