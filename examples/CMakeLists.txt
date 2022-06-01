# This file must be included by add_subdirectory() from parent, it doesn't work as standalone
cmake_minimum_required(VERSION 3.12)
project(nitrogl-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-unwind-tables \
                            -fstrict-aliasing -O3  -flto \
                            -Wstrict-aliasing -fno-math-errno \
                            -ffast-math -fno-threadsafe-statics \
                            -fdata-sections -ffunction-sections \
                            " CACHE INTERNAL "" FORCE)
#-nostdlib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(OpenGL)
#find_package(GLFW3)
find_package(SDL2)
message(tomer ${OPENGL_INCLUDE_DIR})
#if(DEFINED SDL2_FOUND AND OPENGL_FOUND)
if(DEFINED SDL2_FOUND)
    set(LIBS  ${OPENGL_LIBRARY} ${SDL2_LIBRARY} micro-opengl)
#    set(LIBS  ${SDL2_LIBRARY} micro-opengl -ldl)
#    set(INCLUDES  ${SDL2_INCLUDE_DIR})
    set(INCLUDES  ${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
    set(SOURCES
            ex_create_numbers_db.cpp
            ex_1.cpp
            ex_cache.cpp

            ex_draw_triangles.cpp

            ex_draw_mask.cpp
            ex_draw_rounded_rect.cpp
            ex_draw_circle.cpp
            ex_draw_arc.cpp
            ex_draw_pie.cpp
            ex_draw_quadrilateral.cpp
            ex_draw_rect.cpp

            ex_sampler_color.cpp
            ex_sampler_texture.cpp
            ex_sampler_masking.cpp

            ex_sampler_block.cpp
            ex_sampler_d1_function.cpp

            ex_sampler_circle.cpp
            ex_sampler_rounded.cpp
            ex_sampler_arc.cpp
            ex_sampler_pie.cpp
            ex_sampler_capsule.cpp
            ex_sampler_linear_classifier.cpp

            ex_sampler_gradient_line.cpp
            ex_sampler_axial_2_colors_gradient.cpp
            ex_sampler_gradient_circular.cpp
            ex_sampler_gradient_angular.cpp
            )

    set(SOURCES_SHARED
#            libs/glad_3.2_core/src/glad.c
            libs/stb_image/stb_image.cpp
            src/example.cpp
            )

    foreach( testsourcefile ${SOURCES} )
        # I used a simple string replace, to cut off .cpp.
        string( REPLACE ".cpp" "" testname ${testsourcefile} )
        include_directories(${INCLUDES})
        include_directories("${PROJECT_SOURCE_DIR}/src")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
#        include_directories("${PROJECT_SOURCE_DIR}/libs/glad_3.2_core/include")
        include_directories("${PROJECT_BINARY_DIR}")

        add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
        #        add_executable( ${testname} ${testsourcefile}  )
        # Make sure YourLib is linked to each app
        target_link_libraries( ${testname} ${LIBS} )
        # this is redundant, as target file dir is always delegated to CMAKE_RUNTIME_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${testname} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${testname}>/assets)
    endforeach( testsourcefile ${SOURCES} )

endif()

# newer clion does not make the binary executable location the current working dir,
# so I copy the assets also to the top level build directory.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

