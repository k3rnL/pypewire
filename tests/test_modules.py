from time import sleep

import pytest
from pypewire.client import PWClient


def test_list_modules(pipewire_socket):
    """Test listing PipeWire modules."""
    with PWClient() as client:
        modules = client.get_modules()

        # Validate the result
        assert isinstance(modules, list), "modules should be a list"

        # Each module should have the required fields
        for module in modules:
            assert isinstance(module, dict), "module should be a dict"
            assert "id" in module, "module should have id"
            assert "name" in module, "module should have name"
            assert isinstance(module["id"], int), "id should be int"
            assert isinstance(module["name"], str), "name should be str"

        # Should have at least 0 modules
        assert len(modules) >= 0, "Should return at least 0 modules"


def test_list_modules_not_connected(pipewire_socket):
    """Test that listing modules fails when not connected."""
    client = PWClient()

    with pytest.raises(RuntimeError, match="Not connected"):
        client.get_modules()


def test_context_manager(pipewire_socket):
    """Test that the context manager properly connects and disconnects."""
    client = PWClient()
    assert client.connection is None, "Should not be connected initially"

    with client:
        assert client.connection is not None, "Should be connected inside context"
        modules = client.get_modules()
        assert isinstance(modules, list), "Should get modules list"

    assert client.connection is None, "Should be disconnected after context"
