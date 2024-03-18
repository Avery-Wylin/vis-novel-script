#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <unordered_map>
#include <vector>
#include "library/glad_common.h"
#include "FBO.h"



class Texture {
    GLuint tid;
    uint32_t width = 0, height = 0, channels = 0;

    // Forbid Copy
    Texture ( Texture const& );
    Texture& operator= ( Texture const& );

    public:

    static void unbind();

    Texture();
    ~Texture();


    void bind ( uint32_t active_texture );
    void load_png ( std::string filename, uint32_t scale_type = GL_NEAREST, uint32_t extention_type = GL_CLAMP, uint32_t format = GL_RGBA );
    void from_FBO( FBO &fbo, uint32_t slot, uint32_t scale_type = GL_NEAREST );
    void allocate();
    bool is_allocated();
    void free();
    inline uint32_t get_width(){return width;}
    inline uint32_t get_height(){return height;}
    inline float get_ratio(){return (float)height/width;}
};

class TextureArray{
    GLuint texture_id = 0;
    uint32_t width = 0, height = 0, channels = 0, subimages = 0;

    // Forbid Copy
    TextureArray(TextureArray const&);
    TextureArray& operator=(TextureArray const&);

    public:
        TextureArray(){}
        ~TextureArray();

        static void unbind();
        void allocate();
        bool is_allocated();
        void free();
        void bind(uint32_t texture_slot);
        void load_atlas(std::string filename, uint8_t tile_count, uint32_t scale_type = GL_NEAREST, uint32_t extention_type = GL_CLAMP, uint32_t format = GL_RGBA);
        void load_file_list(std::vector<std::string> filenames,  uint32_t scale_type = GL_NEAREST, uint32_t extention_type = GL_CLAMP, uint32_t format = GL_RGBA);
        inline float get_ratio(){return (float)height/width;}

};

#endif /* TEXTURE_H */

