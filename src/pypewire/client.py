from typing import List, Optional, Type
from types import TracebackType
from . import _core  # This is the compiled C extension
from .types import PWModule

class PipeWireClient:
    def __init__(self) -> None:
        self._conn: Optional[_core.PWConnection] = None

    def __enter__(self) -> "PipeWireClient":
        self._conn = _core.PWConnection()
        return self

    def __exit__(self, t: Optional[Type[BaseException]],
                 v: Optional[BaseException],
                 b: Optional[TracebackType]) -> None:
        self._conn = None

    def get_modules(self) -> List[PWModule]:
        if not self._conn:
            raise RuntimeError("Not connected")
        return self._conn.get_modules()