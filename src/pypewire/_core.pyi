from typing import List, Optional, Dict, Any, TypedDict, final

# --- Interface Types ---
PW_TYPE_INTERFACE_Core: str
PW_TYPE_INTERFACE_Client: str
PW_TYPE_INTERFACE_Device: str
PW_TYPE_INTERFACE_Factory: str
PW_TYPE_INTERFACE_Link: str
PW_TYPE_INTERFACE_Module: str
PW_TYPE_INTERFACE_Node: str
PW_TYPE_INTERFACE_Port: str
PW_TYPE_INTERFACE_Registry: str

# --- Versions ---
PW_VERSION_CORE: int
PW_VERSION_CLIENT: int
PW_VERSION_DEVICE: int
PW_VERSION_FACTORY: int
PW_VERSION_LINK: int
PW_VERSION_MODULE: int
PW_VERSION_NODE: int
PW_VERSION_PORT: int
PW_VERSION_REGISTRY: int

# --- Common Properties (Keys) ---
PW_KEY_APP_NAME: str        # "application.name"
PW_KEY_APP_ID: str          # "application.id"
PW_KEY_OBJECT_ID: str       # "object.id"
PW_KEY_OBJECT_SERIAL: str   # "object.serial"
PW_KEY_MODULE_NAME: str     # "module.name"
PW_KEY_FACTORY_NAME: str    # "factory.name"
PW_KEY_DEVICE_NAME: str     # "device.name"
PW_KEY_NODE_NAME: str       # "node.name"
PW_KEY_MEDIA_CLASS: str     # "media.class"
PW_KEY_MEDIA_TYPE: str      # "media.type"
PW_KEY_MEDIA_CATEGORY: str  # "media.category"
PW_KEY_MEDIA_ROLE: str      # "media.role"
PW_KEY_AUDIO_CHANNEL: str   # "audio.channel"

# Helper Type for get_devices return value
# class DeviceDict(TypedDict):
#     id: int
#     name: str
#     props: Dict[str, str]


@final
class PWModule:
    """
    Represents a remote PipeWire module.
    """

    @property
    def id(self) -> int:
        """The global ID of the module."""
        ...

    @property
    def name(self) -> str:
        """The name of the module (e.g., 'libpipewire-module-rt')."""
        ...

    @property
    def args(self) -> Optional[str]:
        """Arguments passed to the module upon loading."""
        ...

    def unload(self) -> None:
        """
        Unload this module from the remote PipeWire server.
        Raises RuntimeError if the module is already destroyed.
        """
        ...


@final
class PWFactory:
    """
    Represents a remote PipeWire factory.
    """

    @property
    def id(self) -> int:
        """The global ID of the module."""
        ...

    @property
    def name(self) -> str:
        """The name of the module (e.g., 'libpipewire-module-rt')."""
        ...

    @property
    def type(self) -> Optional[str]:
        """Arguments passed to the module upon loading."""
        ...

    @property
    def version(self) -> int:
        """Version of the factory."""
        ...


@final
class PWNode:
    """
    Represents a remote PipeWire Node.
    """
    @property
    def id(self) -> int:
        """The global ID of the node."""
        ...

    @property
    def max_input_ports(self) -> int:
        """Maximum supported input ports for this node."""
        ...

    @property
    def max_output_ports(self) -> int:
        """Maximum supported output ports for this node."""
        ...

    @property
    def n_input_ports(self) -> int:
        """Current number of input ports."""
        ...

    @property
    def n_output_ports(self) -> int:
        """Current number of output ports."""
        ...

    @property
    def state_code(self) -> int:
        """
        The numeric state code of this node
        (e.g. PW_NODE_STATE_RUNNING).
        """
        ...

    @property
    def error(self) -> Optional[str]:
        """
        The error message if the node is in an error state,
        or None otherwise.
        """
        ...

    def state(self) -> str:
        """
        Get the string representation of the node state
        (e.g., 'suspended', 'idle', 'running', 'error').
        """
        ...


class PWConnection:
    """
    Manages the connection to the PipeWire daemon.

    This object owns the Core, Context, and ThreadLoop.
    """
    name: str

    def __init__(self, name: str = "default") -> None:
        """
        Initialize the connection.

        Args:
            name: The name of the client app as it will appear in PipeWire.
        """
        ...

    def get_modules(self) -> List[PWModule]:
        """
        Fetch the list of currently loaded modules from the server.

        This method blocks until the server has replied with the full list
        and details of all modules.
        """
        ...

    def get_devices(self) -> List[DeviceDict]:
        """
        Fetch the list of devices from the server.

        Returns a list of dictionaries containing device metadata.
        """
        ...

    def get_factories(self) -> List[PWFactory]:
        """
        Fetch the list of available factories from the server.

        This method blocks until the server has replied with the full list
        and details of all factories.
        :return:
        """
        ...

    def get_nodes(self) -> List[PWNode]:
        """
        Fetch the list of nodes from the server.

        This method blocks until the server has replied with the full list
        and details of all nodes.
        """
        ...

    def create_object(self, factory_name: str, type_name: str, version: int, props: dict[str, Any]) -> None:
        """
        Create an object on the server
        """
        ...

    def load_module(self, name: str, args: Optional[str] = None) -> None:
        """
NOPE FOR NOW
        """
        ...