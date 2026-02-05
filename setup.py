from setuptools import setup, Extension
import subprocess

# Detect PipeWire
cflags = subprocess.check_output(["pkg-config", "--cflags", "libpipewire-0.3", "wireplumber-0.4"], text=True).split()
libs = subprocess.check_output(["pkg-config", "--libs", "libpipewire-0.3", "wireplumber-0.4"], text=True).split()

setup(
    ext_modules=[
        Extension(
            "pypewire._core",
            sources=[
                "native/pypewire/pw_connection/pw_connection.c",
                "native/pypewire/pw_connection/get_factories.c",
                "native/pypewire/pw_connection/get_modules.c",
                "native/pypewire/pw_connection/get_devices.c",
                "native/pypewire/pw_connection/get_nodes.c",
                "native/pypewire/pw_connection/create_object.c",
                "native/pypewire/pw_factory/pw_factory.c",
                "native/pypewire/pw_node/pw_node.c",
                "native/pypewire/pw_module.c",
                "native/pypewire/pypewire.c"
            ],
            include_dirs=[
                "native/pypewire",
                "native/pypewire/pw_connection",
                "native/pypewire/pw_factory",
                "native/pypewire/pw_node",
            ],
            extra_compile_args=cflags,
            extra_link_args=libs
        ),
        Extension(
            "wyreplumber._core",
            sources=[
                "native/wyreplumber/wyreplumber.c",
                "native/wyreplumber/wp_connection/wp_connection.c",
                "native/wyreplumber/wp_connection/get_nodes.c",
                "native/wyreplumber/wp_node/wp_node.c",
            ],
            include_dirs=[
                "native/wyreplumber",
                "native/wyreplumber/wp_connection",
                "native/wyreplumber/wp_node",
            ],
            extra_compile_args=cflags,
            extra_link_args=libs
        )
    ]
)