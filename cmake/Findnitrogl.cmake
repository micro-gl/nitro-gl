#========================================================================================
# Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
# All Rights Reserved.
#
# this should help find the nitrogl headers-only package and define the nitrogl target that was
# installed on your system and does not include CMakeLists.txt file, so you can easily link to it.
# If successful, the following will happen:
# 1. nitrogl_INCLUDE_DIR will be defined
# 2. nitrogl::nitrogl target will be defined so you can link to it using target_link_libraries(..)
#========================================================================================
include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

find_path(nitrogl_INCLUDE_DIR
        NAMES nitrogl
        HINTS ${CMAKE_INSTALL_INCLUDEDIR}
        PATH_SUFFIXES samplers)
set(nitrogl_LIBRARY "/dont/use")
find_package_handle_standard_args(nitrogl DEFAULT_MSG
        nitrogl_LIBRARY nitrogl_INCLUDE_DIR)

if(nitrogl_FOUND)
    message("nitrogl was found !!!")
else(nitrogl_FOUND)
    message("nitrogl was NOT found !!!")
endif(nitrogl_FOUND)

if(nitrogl_FOUND AND NOT TARGET nitrogl::nitrogl)
    # build the target
    add_library(nitrogl::nitrogl INTERFACE IMPORTED)
    set_target_properties(
            nitrogl::nitrogl
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${nitrogl_INCLUDE_DIR}")
endif()