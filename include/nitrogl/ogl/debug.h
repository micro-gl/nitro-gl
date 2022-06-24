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

    template<class lambda_type>
    inline void glCheckError_1(const lambda_type & lambda,
                               const char *file, int line) {
        GLenum errorCode;
        while ((errorCode = glGetError()) != GL_NO_ERROR)
            lambda(errorCode, file, line);
    }

#ifdef NITROGL_DEBUG_MODE

    inline void glCheckError_2(const char *file, int line) {

        const auto lambda = [](GLenum errorCode, const char *file, int line) {
            const char * error_gist;
            switch (errorCode) {
                // I am using the real integer codes instead of enums because
                // some enums might be deprecated in some opengl versions,
                // which will cause compilation errors
                case 0x0500: error_gist = "INVALID_ENUM"; break;
                case 0x0501: error_gist = "INVALID_VALUE"; break;
                case 0x0502: error_gist = "INVALID_OPERATION"; break;
                case 0x0503: error_gist = "GL_STACK_OVERFLOW"; break;
                case 0x0504: error_gist = "GL_STACK_UNDERFLOW"; break;
                case 0x0505: error_gist = "OUT_OF_MEMORY"; break;
                case 0x0506: error_gist = "INVALID_FRAMEBUFFER_OPERATION"; break;
                case 0x0507: error_gist = "GL_CONTEXT_LOST"; break;
                case 0x8031: error_gist = "GL_TABLE_TOO_LARGE1"; break;
                default: error_gist = "UNKNOWN"; break;
            }
            std::cout << error_gist << " | code " << errorCode << " | file "
            << file << " ( line "
            << line << ")" << std::endl;
        };

        glCheckError_1(lambda, file, line);
    }

#define glCheckError() glCheckError_2(__FILE__, __LINE__)
#else
#define glCheckError()
#endif
}