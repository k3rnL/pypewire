from typing import List, TypedDict, final

class NodeDict(TypedDict, total=False):
    """Dictionary containing node information."""
    id: int
    name: str
    nick: str
    description: str
    media_class: str


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

    def get_nodes(self) -> List[NodeDict]:
        """
        Fetch the list of nodes from WirePlumber's object manager.

        This method is thread-safe and blocks from Python's perspective while
        releasing the GIL. The actual work is performed on the WirePlumber thread.

        Returns:
            A list of dictionaries, each containing node information with keys:
            - id: The node's global ID
            - name: The node name (PW_KEY_NODE_NAME)
            - nick: The node nickname (PW_KEY_NODE_NICK)
            - description: Node description (PW_KEY_NODE_DESCRIPTION)
            - media_class: Media class (PW_KEY_MEDIA_CLASS)

        Raises:
            RuntimeError: If the node retrieval fails.
        """
        ...
