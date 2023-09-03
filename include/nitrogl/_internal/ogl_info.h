/*========================================================================================
 Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
 All Rights Reserved.
 License is a custom open source semi-permissive license with the following guidelines:
 1. unless otherwise stated, derivative work and usage of this file is permitted and
    should be credited to the project and the author of this project.
 2. Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
========================================================================================*/
#pragma once

namespace nitrogl {

// if no version was specified go for gl-3.3, or gl-es-3.0
#ifndef NITROGL_OPENGL_MAJOR_VERSION
    #define NITROGL_OPENGL_MAJOR_VERSION 3
#endif

#ifndef NITROGL_OPENGL_MINOR_VERSION
    #ifdef NITROGL_OPEN_GL_ES
        #define NITROGL_OPENGL_MINOR_VERSION 0
    #else
        #define NITROGL_OPENGL_MINOR_VERSION 3
    #endif
#endif

// if VAO was not asked specifically, let's try to infer it
#ifndef NITROGL_SUPPORTS_VAO
    // fits both gl>=3.0, and gl-es>=3.0
    #if (NITROGL_OPENGL_MAJOR_VERSION>=3)
        #define NITROGL_SUPPORTS_VAO
    #endif
#endif

#ifndef NITROGL_OPENGL_GLSL_VERSION
    #ifdef NITROGL_OPEN_GL_ES
        #if (NITROGL_OPENGL_MAJOR_VERSION==2)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 100"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==0)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 300 es"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==1)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 310 es"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==2)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 320 es"
        #else
            #define NITROGL_OPENGL_GLSL_VERSION "#version 300 es"
        #endif
    #else
        #if (NITROGL_OPENGL_MAJOR_VERSION==2) and (NITROGL_OPENGL_MINOR_VERSION==0)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 110"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==2) and (NITROGL_OPENGL_MINOR_VERSION==1)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 120"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==0)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 130"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==1)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 140"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==2)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 150"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==3) and (NITROGL_OPENGL_MINOR_VERSION==3)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 330"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==0)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 400"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==1)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 410"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==2)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 420"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==3)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 430"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==4)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 440"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==5)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 450"
        #elif (NITROGL_OPENGL_MAJOR_VERSION==4) and (NITROGL_OPENGL_MINOR_VERSION==6)
            #define NITROGL_OPENGL_GLSL_VERSION "#version 460"
        #else
            #define NITROGL_OPENGL_GLSL_VERSION "#version 130"
        #endif
    #endif
#endif

    struct ogl_info {
#ifdef NITROGL_OPEN_GL_ES
        static constexpr bool is_es = true;
#else
        static constexpr bool is_es = false;
#endif

#ifdef NITROGL_SUPPORTS_VAO
        static constexpr bool supports_vao = true;
#else
        static constexpr bool supports_vao = false;
#endif
        static constexpr int major = NITROGL_OPENGL_MAJOR_VERSION;
        static constexpr int minor = NITROGL_OPENGL_MINOR_VERSION;
        static constexpr const char * const glsl_version_string = NITROGL_OPENGL_GLSL_VERSION;
    };

}