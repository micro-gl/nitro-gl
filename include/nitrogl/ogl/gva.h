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

    // this describes the General Vertex Attrib bounded by a VBO buffer
    struct generic_vertex_attrib_t {
        GLint index; // the index of the generic vertex attribute
        // the type of element in VBO, this is important because opengl will know
        // better how to convert it to the vertex shader processor
        GLenum type;
        // the type of components in vertex attribute in shader.
        // vec3->float. ivec2->integer etc...
        GLuint size; // the number of components in attribute array vbo (1,2,3,4)
        // the attribute's first relative occurrence offset in the VBO
        const void * offset;
        // stride can be calculated automatically if the buffer is interleaved or non.
        GLsizei stride;
        GLuint vbo; // corresponding vbo
    };

}