#========================================================================================
# Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
# All Rights Reserved.
#
# this should help find the microgl headers-only package and define the microgl target that was
# installed on your system and does not include CMakeLists.txt file, so you can easily link to it.
# If successful, the following will happen:
# 1. micro-opengl_INCLUDE_DIR will be defined
# 2. micro-opengl::micro-opengl target will be defined so you can link to it using target_link_libraries(..)
#========================================================================================
include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

find_path(micro-opengl_INCLUDE_DIR
        NAMES micro-opengl
        HINTS ${CMAKE_INSTALL_INCLUDEDIR}
        PATH_SUFFIXES clippers bitmaps samplers)
set(microgl_LIBRARY "/dont/use")
find_package_handle_standard_args(micro-opengl DEFAULT_MSG
        micro-opengl_LIBRARY micro-opengl_INCLUDE_DIR)

if(micro-opengl_FOUND)
    message("micro-opengl was found !!!")
else(micro-opengl_FOUND)
    message("micro-opengl was NOT found !!!")
endif(micro-opengl_FOUND)

if(micro-opengl_FOUND AND NOT TARGET micro-opengl::micro-opengl)
    # build the target
    add_library(micro-opengl::micro-opengl INTERFACE IMPORTED)
    set_target_properties(
            micro-opengl::micro-opengl
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${micro-opengl_INCLUDE_DIR}")
endif()