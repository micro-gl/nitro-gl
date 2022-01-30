/**
 * Author: Tomer Shalev
 *
 */
#pragma once

#include <iostream>
#include <fstream>
#include "../libs/stb_image/stb_image.h"
#include "../libs/rapidxml/rapidxml.hpp"
#include <nitrogl/ogl/gl_texture.h>

using std::cout;
using std::endl;

/**
 * basic resources loader, includes built in json and image support.
 *
 */
class Resources  {

public:
    /**
     * basic uncompressed image info structure
     */
    struct image_info_t {
        unsigned char * data;
        int width;
        int height;
        int channels;
        bool is_pre_mul_alpha;
    };

    struct buffer_t {
        char * data;
        unsigned long size;
    };

    Resources()=delete;

    static
    image_info_t loadImageFromCompressedPath(const char * path, bool pre_mul_alpha=true, bool flip_vertically=false) {
        auto buf = loadFileAsByteArray(path);
        auto img = loadImageFromCompressedMemory(reinterpret_cast<unsigned char *>(buf.data),
                                                 buf.size, pre_mul_alpha, flip_vertically);
        delete buf.data;
        return img;
    }

    static
    image_info_t loadImageFromCompressedMemory(unsigned char *byte_array,
                                               unsigned int length_bytes,
                                               bool pre_mul_alpha=true,
                                               bool flip_vertically=false) {
        int width, height, channels;
        if(flip_vertically)
            stbi_set_flip_vertically_on_load(true);
        unsigned char * data = stbi_load_from_memory(byte_array, length_bytes, &width, &height,
                                                     &channels, 0);
        image_info_t info {data, width, height, channels, pre_mul_alpha };
        if(pre_mul_alpha and channels==4) {
            using uint_t = unsigned int;
            const auto size = width*height;
            auto p = data;
            for (int ix = 0; ix < size; ++ix, p+=4) {
                const uint_t a = p[3];
                p[0] = (uint_t(p[0])*a)/255;
                p[1] = (uint_t(p[1])*a)/255;
                p[2] = (uint_t(p[2])*a)/255;
            }
        } else if (channels==4) { // this for test
            using uint_t = unsigned int;
            const auto size = width*height;
            auto p = data;
            for (int ix = 0; ix < size; ++ix, p+=4) {
                const uint_t a = p[3];
                if(a==0) p[1]=255;
            }

        }
        return info;
    }

    static
    nitrogl::gl_texture loadTexture(const char * path, bool pre_mul_alpha=true) {
        auto img = loadImageFromCompressedPath(path, pre_mul_alpha);
        auto tex = nitrogl::gl_texture::from_unpacked_image(img.width, img.height, img.data, 8, 8, 8,
                                                        img.channels==4?8:0, pre_mul_alpha);
        delete img.data;
        return tex;
    }

    static
    nitrogl::gl_texture loadTextureFromCompressedMemory(unsigned char *byte_array,
                                               unsigned int length_bytes,
                                               bool pre_mul_alpha=true,
                                               bool flip_vertically=false) {
        auto img = loadImageFromCompressedMemory(byte_array, length_bytes, pre_mul_alpha, flip_vertically);
        auto tex = nitrogl::gl_texture::from_unpacked_image(img.width, img.height, img.data, 8, 8, 8,
                                                            img.channels==4?8:0,pre_mul_alpha);
        delete img.data;
        return tex;
    }

    static buffer_t loadFileAsByteArray(const char * file_name, unsigned int pad=0) {
        std::ifstream ifs(file_name, std::ios::binary);
        ifs.seekg(0, std::ios::end);
        auto isGood = ifs.good();
        if(!isGood) return {nullptr, 0};
        auto length = static_cast<size_t>(ifs.tellg());
        auto *ret = new char[length + pad];
        ifs.seekg(0, std::ios::beg);
        ifs.read(ret, length);
        ifs.close();
        return { ret, length };
    }

    static char * loadTextFile(const char * file_name) {
        auto buffer = loadFileAsByteArray(file_name, 1);
        buffer.data[buffer.size-1] = '\0';
        return buffer.data;
    }

    static
    void loadXML(const char *file_name, rapidxml::xml_document<> & doc) {
        doc.clear();
        doc.parse<0>(loadTextFile(file_name));
    }

//    template<typename BITMAP>
//    static microgl::text::bitmap_font<BITMAP> loadFont(const std::string &name) {
//        microgl::text::bitmap_font<BITMAP> font;
//        stbi_set_flip_vertically_on_load(false);
//        rapidxml::xml_document<> d;
//        loadXML("fonts/"+name+"/font.fnt", d);
//        auto * f= d.first_node("font");
//        auto * f_info= f->first_node("info");
//        auto * f_common= f->first_node("common");
//        auto * f_chars= f->first_node("chars");
//        strncpy(font.name, f_info->first_attribute("face")->value(), 10);
//        font.nativeSize=atoi(f_info->first_attribute("size")->value());
//        font.lineHeight=atoi(f_common->first_attribute("lineHeight")->value());
//        font.baseline=atoi(f_common->first_attribute("base")->value());
//        font.width=atoi(f_common->first_attribute("scaleW")->value());
//        font.height=atoi(f_common->first_attribute("scaleH")->value());
//        font.glyphs_count=atoi(f_chars->first_attribute("count")->value());
//        auto * iter= f_chars->first_node("char");
//        do {
//            int id=atoi(iter->first_attribute("id")->value());
//            int x=atoi(iter->first_attribute("x")->value());
//            int y=atoi(iter->first_attribute("y")->value());
//            int w=atoi(iter->first_attribute("width")->value());
//            int h=atoi(iter->first_attribute("height")->value());
//            int xoffset=atoi(iter->first_attribute("xoffset")->value());
//            int yoffset=atoi(iter->first_attribute("yoffset")->value());
//            int xadvance=atoi(iter->first_attribute("xadvance")->value());
//            font.addChar(id, x, y, w, h, xoffset, yoffset, xadvance);
//            iter = iter->next_sibling();
//        } while (iter);
//        // load bitmap
//        auto img_font = loadImageFromCompressedPath("fonts/"+name+"/font.png");
//        auto *bmp_font = new BITMAP(img_font.data, img_font.width, img_font.height);
//        font.bitmap=bmp_font;
//        stbi_set_flip_vertically_on_load(true);
//        return font;
//    }

};
