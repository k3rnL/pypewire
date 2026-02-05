from setuptools import setup, Extension
import subprocess

# Detect PipeWire
cflags = subprocess.check_output(["pkg-config", "--cflags", "libpipewire-0.3"], text=True).split()
libs = subprocess.check_output(["pkg-config", "--libs", "libpipewire-0.3"], text=True).split()

setup(
    ext_modules=[
        Extension(
            "pypewire._core",
            sources=[
                "src/native/pw_connection/pw_connection.c",
                "src/native/pw_connection/get_factories.c",
                "src/native/pw_connection/get_modules.c",
                "src/native/pw_connection/get_devices.c",
                "src/native/pw_connection/get_nodes.c",
                "src/native/pw_connection/create_object.c",
                "src/native/pw_factory/pw_factory.c",
                "src/native/pw_node/pw_node.c",
                "src/native/pw_module.c",
                "src/native/core.c"
            ], # Path to C file
            extra_compile_args=cflags,
            extra_link_args=libs
        )
    ]
)