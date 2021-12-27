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

    class gl_texture {
    public:
        static GLenum ch2enum(unsigned channels) {
            return (channels==4) ? GL_RGBA : (channels==3 ? GL_RGB : (channels==2) ? GL_RG : GL_RED);
        }
        static GLenum bits2type(unsigned bits) {
            return (bits<=8) ? GL_UNSIGNED_BYTE : (bits<=16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT);
        }
        /**
         * create a texture definition from a tightly unpacked byte-array of pixels (no padding between rows).
         * This will create the same texture layout in gpu memory. Assumes each color channel is 8 bits.
         */
        static gl_texture from_unpacked_image(GLsizei width, GLsizei height, const void * data,
                                              const unsigned channels=4) {
            return gl_texture(ch2enum(channels), width, height, ch2enum(channels), GL_UNSIGNED_BYTE, data, 1);
        }
        static gl_texture from_packed_image(GLsizei width, GLsizei height, const void * data,
                                            char r_bits, char g_bits, char b_bits, char a_bits,
                                            bool reversed=false) {
            GLenum format=GL_RGBA, type=GL_UNSIGNED_INT_8_8_8_8;
            const bool r = reversed;
            const GLint internalformat = a_bits ? GL_RGBA : GL_RGB;

            const auto sum = r_bits+g_bits+b_bits+a_bits;
            // start with supported ones
            if(r_bits==3 && g_bits==3 && b_bits==2)
            { format=r ? GL_BGR : GL_RGB; type=GL_UNSIGNED_BYTE_3_3_2; }
            else if(r_bits==5 && g_bits==6 && b_bits==5)
            { format=r ? GL_BGR : GL_RGB; type=GL_UNSIGNED_SHORT_5_6_5; }
            else if(r_bits==4 && g_bits==4 && b_bits==4 && a_bits==4)
            { format=r ? GL_BGRA : GL_RGBA; type=GL_UNSIGNED_SHORT_4_4_4_4; }
            else if(r_bits==5 && g_bits==5 && b_bits==5 && a_bits==1)
            { format=r ? GL_BGRA : GL_RGBA; type=GL_UNSIGNED_SHORT_5_5_5_1; }
            else if(r_bits==8 && g_bits==8 && b_bits==8 && a_bits==8)
            { format=r ? GL_BGRA : GL_RGBA; type=GL_UNSIGNED_INT_8_8_8_8; }
            else if(r_bits==10 && g_bits==10 && b_bits==10 && a_bits==2)
            { format=r ? GL_BGRA : GL_RGBA; type=GL_UNSIGNED_INT_10_10_10_2; }
            else if(r_bits && g_bits==0 && b_bits==0 && a_bits==0)
            { format=GL_RED; type=bits2type(r_bits); }

            return gl_texture(internalformat, width, height, format, type, data, 1);
        }
        /**
         * The most general ctor
         * @param internalformat Specify The pixel format to store the texture on GPU, usually GL_RGBA
         * @param width The width of the image data
         * @param height The height of the image data
         * @param format specifies the layout of color channels in a pixel (unless the type argument has that info
         *               like type=GL_UNSIGNED_BYTE_3_3_2). Common values are GL_RGB/GL_RGBA/GL_BGR/GL_BGRA or
         *               GL_RED
         * @param type The pixel type of each pixel in the image array. If it is unpacked just GL_UNSIGNED_BYTE,
         *             otherwise specify your pixel type, for example GL_UNSIGNED_INT/GL_UNSIGNED_SHORT_5_6_5
         * @param data The image array of pixels
         * @param unpack_row_alignment Image data is made up of rows. In case, each end of row of pixels is padded
         *                  so the next row is aligned to (1|2|4|8), then you need to specify it. Most data can be saved
         *                  continuously and I recommend saving images with alignment of 1 (no-padding)
         */
        gl_texture(GLint internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type,
                   const void * data, GLint unpack_row_alignment=1) :
                _id(0), _size_bytes(0), _internalformat(internalformat), _width(width),
                _height(height), _format(format), _type(type), _data(data),
                _unpack_row_alignment(unpack_row_alignment) {};
        /**
         * this ctor assumes pixel data is unpacked and will store the same layout in gpu
         */
        gl_texture(GLsizei width, GLsizei height, const void * data, const unsigned channels=4) :
                        gl_texture(ch2enum(channels), width, height, ch2enum(channels), GL_UNSIGNED_BYTE, data, 1) {}

        ~gl_texture() { del(); GL_RGBA}

        void uploadData(GLuint * array, GLsizeiptr array_size_bytes) {
            if(_id==0) return;
            bind();
            glBufferData(GL_ARRAY_BUFFER, array_size_bytes, array, GL_STATIC_DRAW);
            _size_bytes = array_size_bytes;
        }
        void uploadSubData(GLintptr offset, const void *array, GLuint size_bytes) {
            if(_id==0) return;
            bind();
            glBufferSubData(GL_ARRAY_BUFFER, offset, size_bytes, array);
        }
        void generate() { if(_id==0) glGenBuffers(1, &_id); bind(); }
        void del() { if(_id) { glDeleteBuffers(1, &_id); _id=0; } }
        void bind() const { glBindBuffer(GL_ARRAY_BUFFER, _id); }
        void unbind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }
        GLsizeiptr size() const { return _size_bytes / GLsizeiptr(sizeof(GLuint)); }
        GLsizeiptr size_bytes() const { return _size_bytes; }

    private:
        GLuint _id;
        GLsizeiptr _size_bytes;
        GLint _internalformat;
        GLsizei _width, _height;
        GLenum _format, _type;
        const void * _data;
        GLint _unpack_row_alignment;

    };

}

