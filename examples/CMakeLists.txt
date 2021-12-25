# This file must be included by add_subdirectory() from parent, it doesn't work as standalone
cmake_minimum_required(VERSION 3.12)
project(micro-opengl-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-exceptions -fno-unwind-tables \
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
if(DEFINED SDL2_FOUND AND OPENGL_FOUND)
    set(libs  ${OPENGL_LIBRARY} ${SDL2_LIBRARY} micro-opengl)
    set(SOURCES
          ex_1.cpp)

    set(SOURCES_SHARED
            libs/stb_image/stb_image.cpp
            src/Resources.cpp
            src/Resources.cpp
            src/example.cpp src/example.h
            )

    foreach( testsourcefile ${SOURCES} )
        # I used a simple string replace, to cut off .cpp.
        string( REPLACE ".cpp" "" testname ${testsourcefile} )
        include_directories(${GLFW3_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
        include_directories("${PROJECT_SOURCE_DIR}/src")
        include_directories("${PROJECT_SOURCE_DIR}/libs/stb_image")
        include_directories("${PROJECT_BINARY_DIR}")

        add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
        #        add_executable( ${testname} ${testsourcefile}  )
        # Make sure YourLib is linked to each app
        target_link_libraries( ${testname} ${libs} )
        # this is redundant, as target file dir is always delegated to CMAKE_RUNTIME_OUTPUT_DIRECTORY
        add_custom_command(TARGET ${testname} POST_BUILD COMMAND
                ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
                $<TARGET_FILE_DIR:${testname}>/assets)
    endforeach( testsourcefile ${SOURCES} )

endif()

# newer clion does not make the binary executable location the current working dir,
# so I copy the assets also to the top level build directory.
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

