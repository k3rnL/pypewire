cmake_minimum_required(VERSION 4.1)
project(pypewire C)

set(CMAKE_C_STANDARD 11)

include_directories(src/native)
include_directories(src/native/pw_connection)
include_directories(src/native/pw_factory)

find_package(Python3 REQUIRED COMPONENTS Development)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)

add_library(pypewire_core MODULE
        src/native/pw_connection/create_object.c
        src/native/pw_connection/get_factories.c
        src/native/pw_connection/get_devices.c
        src/native/pw_connection/get_modules.c
        src/native/pw_connection/pw_connection.c
        src/native/pw_connection/pw_connection.h
        src/native/pw_factory/pw_factory.c
        src/native/pw_factory/pw_factory.h
        src/native/pw_module.c
        src/native/pw_module.h
        src/native/core.c
        src/native/pw_node/pw_node.c
        src/native/pw_node/pw_node.h
        src/native/pw_connection/get_nodes.c)


target_include_directories(pypewire_core PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${PIPEWIRE_INCLUDE_DIRS}
        src/native
)

target_compile_options(pypewire_core PRIVATE
        ${PIPEWIRE_CFLAGS_OTHER}
)

target_link_directories(pypewire_core PRIVATE
        ${PIPEWIRE_LIBRARY_DIRS}
)

target_link_libraries(pypewire_core PRIVATE
        Python3::Python
        ${PIPEWIRE_LIBRARIES}
)