from setuptools import setup, Extension
import subprocess

# Detect PipeWire
cflags = subprocess.check_output(["pkg-config", "--cflags", "libpipewire-0.3"], text=True).split()
libs = subprocess.check_output(["pkg-config", "--libs", "libpipewire-0.3"], text=True).split()

setup(
    ext_modules=[
        Extension(
            "pypewire._core",
            sources=["src/native/core.c"], # Path to C file
            extra_compile_args=cflags,
            extra_link_args=libs
        )
    ]
)