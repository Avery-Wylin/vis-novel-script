#include "library/glad.h"
#include "library/stb_image.h"
#include "Texture.h"
#include "FBO.h"

void Texture::unbind(){
    glBindTexture(GL_TEXTURE_2D,0);
}

Texture::Texture() {
    tid = 0;
}

Texture::~Texture() {
    free();
}

void Texture::bind(int active_texture ){
    if( is_allocated()){
        glActiveTexture(GL_TEXTURE0+ active_texture );

        GLint bound_tex;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound_tex);
        if(bound_tex == tid)
            return;

        glBindTexture(GL_TEXTURE_2D, tid);
    }
}

void Texture::load_png(std::string filename, uint32_t scale_type, uint32_t extention_type, uint32_t format){
    allocate();
    glBindTexture(GL_TEXTURE_2D, tid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, extention_type);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, extention_type);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scale_type );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scale_type );
    std::string filepath = (std::string)DIR_TEXTURES + filename + ".png";
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else{
        std::string error = "Failed to load texture "+filepath;
        perror(error.c_str());
    }
}

/*
 * Attaches the texture to the FBO and allocates the texture.
 * This will fail if the FBO already has a color attachment, 
 * if the FBO is unallocated, or if the slot is invalid.
 */
void Texture::from_FBO(FBO &fbo, unsigned int slot, uint32_t scale_type){
    if(  slot >= FBO_MAX_COLOR_ATTACHMENTS || !fbo.is_allocated() || fbo.get_color_render_buffer(slot) != 0)
        return;
    
    allocate();
    fbo.bind();
    glBindTexture(GL_TEXTURE_2D, tid);
    
    // Settings for FBO texture
    width = fbo.get_width();
    height = fbo.get_height();
    channels = 1;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo.get_width(), fbo.get_height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scale_type );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scale_type );

    // Get the texture from the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+slot,GL_TEXTURE_2D,tid,0);

}

void Texture::allocate(){
    if(!is_allocated())
        glGenTextures(1, &tid);
}

bool Texture::is_allocated(){
    return tid != 0;
}

void Texture::free(){
    if( is_allocated()){
        glDeleteTextures(1, &tid);
        tid = 0;
    }
}
