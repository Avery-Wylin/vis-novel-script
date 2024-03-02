#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "library/glad_common.h"
#include "FBO.h"



class Texture {
    GLuint tid;
    int width = 0, height = 0, channels = 0;

    // Forbid Copy
    Texture ( Texture const& );
    Texture& operator= ( Texture const& );

    public:

    static void unbind();

    Texture();
    ~Texture();


    void bind ( int active_texture );
    void load_png ( std::string filename, uint32_t scale_type = GL_NEAREST, uint32_t extention_type = GL_CLAMP, uint32_t format = GL_RGBA );
    void from_FBO( FBO &fbo, unsigned int slot, uint32_t scale_type = GL_NEAREST );
    void allocate();
    bool is_allocated();
    void free();
    inline int get_width(){return width;}
    inline int get_height(){return height;}
    inline float get_ratio(){return (float)height/width;}
};

#endif /* TEXTURE_H */

