#!/usr/bin/env python3
"""
Development helper script to start a PipeWire container for local development.
This script provides an interactive environment with a running PipeWire instance.
"""

import tempfile
import shutil
import sys
import os
import time
from pathlib import Path
from testcontainers.core.wait_strategies import FileExistsWaitStrategy
from testcontainers.core.container import DockerContainer
from testcontainers.core.image import DockerImage


def main():
    print("🔧 Starting PipeWire development container...")

    # Create temporary directories for PipeWire runtime
    runtime_dir = tempfile.mkdtemp(prefix="pipewire-runtime-")
    config_dir = str(Path('tests/pipewire_container/config').absolute())

    print(f"📁 Runtime directory: {runtime_dir}")

    # Build the image
    print("🐳 Building Docker image...")
    image = DockerImage(path=str(Path("tests/pipewire_container").absolute()), tag="pypewire-dev")
    image.build()

    # Build the container from the Dockerfile
    container = DockerContainer(image=image.tag)

    # Set environment variables
    container.with_env("PIPEWIRE_DEBUG", "3")
    container.with_env("XDG_RUNTIME_DIR", "/pipewire-runtime")

    # Mount the runtime directory to expose PipeWire socket to host
    container.with_volume_mapping(runtime_dir, "/pipewire-runtime", mode="rw")

    try:
        # Start the container
        print("🚀 Starting container...")
        container.start()
        container.waiting_for(FileExistsWaitStrategy("/pipewire-runtime/pipewire-0"))

        # Set permissions
        container.exec("chmod 777 -R /pipewire-runtime")

        print("\n✅ PipeWire container is running!")
        print("\n📋 Environment variables to use:")
        print(f"   export XDG_RUNTIME_DIR={runtime_dir}")
        print(f"   export PIPEWIRE_CONFIG_DIR={config_dir}")

        print("\n🔌 PipeWire socket available at:")
        print(f"   {runtime_dir}/pipewire-0")

        print("\n💡 You can now run your Python code with these environment variables set.")
        print("   Press Ctrl+C to stop the container and clean up.\n")

        # Set environment variables for this process
        os.environ["XDG_RUNTIME_DIR"] = runtime_dir
        os.environ["PIPEWIRE_CONFIG_DIR"] = config_dir

        # Keep the container running
        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        print("\n\n🛑 Stopping container...")
    finally:
        # Cleanup
        container.stop()
        shutil.rmtree(runtime_dir, ignore_errors=True)
        print("✨ Cleanup complete!")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"❌ Error: {e}", file=sys.stderr)
        sys.exit(1)
