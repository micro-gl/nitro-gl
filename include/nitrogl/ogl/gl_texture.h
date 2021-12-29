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
        static GLenum ch2enum(unsigned channels)
        { return (channels==4) ? GL_RGBA : (channels==3 ? GL_RGB : (channels==2) ? GL_RG : GL_RED); }
        static GLenum bits2type(unsigned bits)
        { return (bits<=8) ? GL_UNSIGNED_BYTE : (bits<=16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT); }
        static unsigned max(unsigned a, unsigned b) { return a<b ? b : a; }
    public:
        /**
         * create a texture definition from a tightly unpacked byte-array of pixels (no padding between rows).
         * This will create the same texture layout in gpu memory.
         * data = {r,g,b, r,g,b, r,g,b ....}
         */
        static gl_texture from_unpacked_image(GLsizei width, GLsizei height, const void * data,
                                              char r_bits, char g_bits, char b_bits, char a_bits) {
            const auto max_bits = max(max(r_bits, g_bits), max(b_bits, a_bits));
            GLenum type = bits2type(max_bits);
            GLenum format=GL_RGBA;
            const GLint internalformat = a_bits ? GL_RGBA : GL_RGB;

            if(r_bits && g_bits && b_bits && a_bits) { format=GL_RGBA; }
            else if(r_bits && g_bits && b_bits) { format= GL_RGB; }
            else if(r_bits && g_bits) { format=GL_RG; }
            else if(r_bits) { format=GL_RED; }

            return gl_texture(internalformat, width, height, format, type, data, 1);
        }
        /**
         * create a texture definition from a tightly packed pixel-array of pixels (no padding between rows).
         * This will create the same texture layout in gpu memory.
         * data = { (r|g|b|a), (r|g|b|a), (r|g|b|a) ....}
         */
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
            else { /* custom converter with memory allocation ? */ }

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
                _unpack_row_alignment(unpack_row_alignment) {
            create();
        };
        /**
         * this ctor assumes pixel data is unpacked and will store the same layout in gpu
         */
        gl_texture(GLsizei width, GLsizei height, const void * data=nullptr, const unsigned channels=4) :
            gl_texture(ch2enum(channels), width, height, ch2enum(channels), GL_UNSIGNED_BYTE, data, 1) {}

        ~gl_texture() { del(); }

        bool wasCreated() const { return _id; }
        bool create() {
            if(wasCreated()) return false;
            glGenTextures(1, &_id);
            glBindTexture(GL_TEXTURE_2D, _id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, _unpack_row_alignment);
            glTexImage2D(GL_TEXTURE_2D, 0, _internalformat, _width, _height, 0,
                         _format, _type, _data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // these two are required for opengl es 2.0
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            return true;
        }

        /**
         *
         * @param x, y where to start x update in current tex
         * @param width/height width of sub texture
         * @param pixels data of sub texture, has to be with the same format and type
         * @return
         */
        bool updateSubTexture(GLint x, GLint y, GLsizei width, GLsizei height,
                              const void * pixels, GLint unpack_row_alignment=1) {
            use();
            glPixelStorei(GL_UNPACK_ALIGNMENT, unpack_row_alignment);
            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, _format, _type, pixels);
            unuse();
            return true;
        }

        GLuint id() const { return _id; }
        static void unuse() { glBindTexture(GL_TEXTURE_2D, 0); }
        void use() const { use(0); }
        void use(int index) const {
            glActiveTexture(GL_TEXTURE0 + (unsigned int)index);
            glBindTexture(GL_TEXTURE_2D, _id);
        }
        GLsizei width() const { return _width; }
        GLsizei height() const { return _height; }
        const void * data_unsafe() const { return _data; } // might be gone

        void del() {
            if(_id) glDeleteTextures(1, &_id);
            _id=_size_bytes=_internalformat=_width=_height=_format=_type=0;
            _data=nullptr;
        }

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

