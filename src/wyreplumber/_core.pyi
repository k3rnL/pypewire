from typing import List, Dict, Optional, final

# Node state constants
WP_NODE_STATE_ERROR: int
WP_NODE_STATE_CREATING: int
WP_NODE_STATE_SUSPENDED: int
WP_NODE_STATE_IDLE: int
WP_NODE_STATE_RUNNING: int


@final
class WPNode:
    """
    Represents a WirePlumber node.

    This object wraps a WpNode and provides access to all its properties,
    state information, and port counts.
    """

    @property
    def id(self) -> int:
        """The global ID of the node."""
        ...

    @property
    def properties(self) -> Dict[str, str]:
        """
        All node properties as a dictionary.

        Includes properties like:
        - node.name: The node name
        - node.nick: The node nickname
        - node.description: Node description
        - media.class: Media class (e.g., 'Audio/Source', 'Audio/Sink')
        - application.name: Application name
        And many others depending on the node type.
        """
        ...

    @property
    def state(self) -> int:
        """
        The current state of the node.

        Returns one of:
        - WP_NODE_STATE_ERROR (-1)
        - WP_NODE_STATE_CREATING (0)
        - WP_NODE_STATE_SUSPENDED (1)
        - WP_NODE_STATE_IDLE (2)
        - WP_NODE_STATE_RUNNING (3)
        """
        ...

    @property
    def n_input_ports(self) -> int:
        """Current number of input ports."""
        ...

    @property
    def max_input_ports(self) -> int:
        """Maximum number of input ports supported."""
        ...

    @property
    def n_output_ports(self) -> int:
        """Current number of output ports."""
        ...

    @property
    def max_output_ports(self) -> int:
        """Maximum number of output ports supported."""
        ...

    @property
    def error_message(self) -> Optional[str]:
        """Error message if state is WP_NODE_STATE_ERROR, None otherwise."""
        ...

    def delete(self) -> None:
        """
        Delete this node from the PipeWire server.

        Raises:
            RuntimeError: If the node is already deleted or invalid.
        """
        ...


@final
class WPConnection:
    """
    WirePlumber connection that manages communication with PipeWire via WirePlumber.

    Creates a dedicated thread for the WirePlumber event loop on initialization.
    All WirePlumber operations are dispatched to this thread while Python methods
    block until completion.
    """

    def __init__(self) -> None:
        """
        Initialize the WirePlumber connection.

        Creates a background thread where the WirePlumber GMainLoop runs.
        Blocks until the connection is established and the object manager is ready.

        Raises:
            RuntimeError: If the connection fails or thread creation fails.
        """
        ...

    def get_nodes(self) -> List[WPNode]:
        """
        Fetch the list of nodes from WirePlumber's object manager.

        This method is thread-safe and blocks from Python's perspective while
        releasing the GIL. The actual work is performed on the WirePlumber thread.

        Returns:
            A list of WPNode objects representing all nodes in the PipeWire graph.

        Raises:
            RuntimeError: If the node retrieval fails.
        """
        ...
