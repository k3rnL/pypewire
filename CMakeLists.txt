cmake_minimum_required(VERSION 4.1)
project(pypewire C)

set(CMAKE_C_STANDARD 11)

include_directories(native/pypewire)
include_directories(native/pypewire/pw_connection)
include_directories(native/pypewire/pw_factory)

find_package(Python3 REQUIRED COMPONENTS Development)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED libpipewire-0.3)
pkg_check_modules(WIREPLUMBER REQUIRED wireplumber-0.4)

add_library(pypewire_core MODULE
        native/pypewire/pw_connection/create_object.c
        native/pypewire/pw_connection/get_factories.c
        native/pypewire/pw_connection/get_devices.c
        native/pypewire/pw_connection/get_modules.c
        native/pypewire/pw_connection/pw_connection.c
        native/pypewire/pw_connection/pw_connection.h
        native/pypewire/pw_factory/pw_factory.c
        native/pypewire/pw_factory/pw_factory.h
        native/pypewire/pw_module.c
        native/pypewire/pw_module.h
        native/pypewire/pypewire.c
        native/pypewire/pw_node/pw_node.c
        native/pypewire/pw_node/pw_node.h
        native/pypewire/pw_connection/get_nodes.c)

add_library(wyreplumber_core MODULE
        native/wyreplumber/wyreplumber.c
        native/wyreplumber/wp_connection/wp_connection.c
        native/wyreplumber/wp_connection/wp_connection.h)


target_include_directories(pypewire_core PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${PIPEWIRE_INCLUDE_DIRS}
        ${WIREPLUMBER_INCLUDE_DIRS}
        src/native
)
target_include_directories(wyreplumber_core PRIVATE
        ${Python3_INCLUDE_DIRS}
        ${PIPEWIRE_INCLUDE_DIRS}
        ${WIREPLUMBER_INCLUDE_DIRS}
        src/native
)

target_compile_options(pypewire_core PRIVATE
        ${PIPEWIRE_CFLAGS_OTHER}
        ${WIREPLUMBER_CFLAGS_OTHER}
)
target_compile_options(wyreplumber_core PRIVATE
        ${PIPEWIRE_CFLAGS_OTHER}
        ${WIREPLUMBER_CFLAGS_OTHER}
)

target_link_directories(pypewire_core PRIVATE
        ${PIPEWIRE_LIBRARY_DIRS}
        ${WIREPLUMBER_LIBRARY_DIRS}
)
target_link_directories(wyreplumber_core PRIVATE
        ${PIPEWIRE_LIBRARY_DIRS}
        ${WIREPLUMBER_LIBRARY_DIRS}
)

target_link_libraries(pypewire_core PRIVATE
        Python3::Python
        ${PIPEWIRE_LIBRARIES}
        ${WIREPLUMBER_LIBRARIES}
)
target_link_libraries(wyreplumber_core PRIVATE
        Python3::Python
        ${PIPEWIRE_LIBRARIES}
        ${WIREPLUMBER_LIBRARIES}
)