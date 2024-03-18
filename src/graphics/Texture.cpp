#include "library/glad.h"
#include "library/stb_image.h"
#include "Texture.h"
#include "FBO.h"
#include <stdexcept>

void Texture::unbind(){
    glBindTexture(GL_TEXTURE_2D,0);
}

Texture::Texture() {
    tid = 0;
}

Texture::~Texture() {
    free();
}

void Texture::bind(uint32_t active_texture ){
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
    int w, h, c;
    unsigned char *data = stbi_load(filepath.c_str(), &w, &h, &c, 0);
    width = w, height = h, channels = c;

    if(!data){
        printf("Failed to load png texture: %s\n", filepath.c_str());
        fflush(stdout);
        return;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
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

/*
 * Texture Array
 */


TextureArray::~TextureArray(){
    free();
}

void TextureArray::unbind(){
    glBindTexture(GL_TEXTURE_2D_ARRAY,0);
}

void TextureArray::allocate(){
    if(!texture_id)
        glGenTextures(1, &texture_id);
}

bool TextureArray::is_allocated(){
    return texture_id;
}

void TextureArray::free(){
    if(texture_id){
        glDeleteTextures(1, &texture_id);
        texture_id = 0;
    }
}

void TextureArray::bind(uint32_t texture_slot){
    if(texture_id){
        glActiveTexture(GL_TEXTURE0 + texture_slot );

        GLint bound_tex;
        glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &bound_tex);
        if(bound_tex == texture_id)
            return;

        glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    }
}

void TextureArray::load_atlas(std::string filename,  uint8_t tile_count, uint32_t scale_type, uint32_t extention_type, uint32_t format){
    allocate();
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, extention_type);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, extention_type);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, scale_type );
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, scale_type );
    std::string filepath = (std::string)DIR_TEXTURES + filename + ".png";
    unsigned char *data = nullptr;
    {
        int w, h, c;
        unsigned char *data = stbi_load(filepath.c_str(), &w, &h, &c, 0);
        width = w, height = h, channels = c;
    }
    if(!data){
        printf("Failed to load atlas png texture: %s\n", filepath.c_str());
        fflush(stdout);
        return;
    }
    if(width != height){
        printf("Failed to load atlas, texture must be a square : %s\n", filename.c_str());
        fflush(stdout);
        return;
    }
    if(width%tile_count != 0){
        printf("Failed to load atlas, dimensions must be multiple of tile count : %s\n", filename.c_str());
        fflush(stdout);
        return;
    }


    uint32_t tile_size = width/tile_count;
    std::vector<unsigned char> tile(tile_size * tile_size * channels);

    // Create an empty texture slot to fill
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, tile_size, tile_size, tile_count*tile_count, 0, format, GL_UNSIGNED_BYTE, nullptr);


    for(uint32_t y = 0; y < tile_count; ++y){
        for( uint32_t x = 0; x < tile_count; ++x){
            // Find the offset in the data that is represented by x and y
            unsigned char* subdata = data + (channels*tile_size)*(y*width + x);

            // Copy the tile over row-by-row
            for(uint32_t r = 0; r < tile_size; ++r){
                std::copy(subdata + r*width*channels, subdata+ channels*(r*width + tile_size), tile.begin() + r*tile_size*channels);

                uint32_t i = y * tile_count + x;
                glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, tile_size, tile_size, 1, format, GL_UNSIGNED_BYTE, tile.data());
            }
        }
    }

    stbi_image_free(data);
}

void TextureArray::load_file_list(std::vector<std::string> filenames,  uint32_t scale_type, uint32_t extention_type, uint32_t format){

    allocate();
    glBindTexture(GL_TEXTURE_2D_ARRAY, texture_id);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, extention_type);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, extention_type);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, scale_type );
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, scale_type );

    uint32_t image_id = 0;
    width = 0; height = 0, channels = 0;
    std::string filepath;
    bool initialized = false;

    for(std::string& filename : filenames){
        // Place name in map

        bool failed = false;
        // Read data from each file, if data is not read, a blank texture is used instead
        filepath = (std::string)DIR_TEXTURES + filename + ".png";
        unsigned char *data = nullptr;
        {
            int w, h, c;
            data = stbi_load(filepath.c_str(), &w, &h, &c, 0);

            if(!initialized){
                width = w, height = h, channels = c;
                // Create an empty texture slot to fill
                glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, format, width, height, filenames.size(), 0, format, GL_UNSIGNED_BYTE, nullptr);
                initialized = true;
            }
            else if(w!=width || h!=height || c!=channels){
                printf("Dimensions do not match previous images in list: %s\n", filename.c_str());
                fflush(stdout);
                failed = true;
                if(data)
                     stbi_image_free(data);
                ++image_id;
                continue;
            }
        }

        // If the data failed to load, skip the image
        if(!data){
            printf("Failed to load image in image list : %s\n", filename.c_str());
            fflush(stdout);
            ++image_id;
            continue;
        }

        // Load subimage
        glTexSubImage3D( GL_TEXTURE_2D_ARRAY, 0, 0, 0, image_id, width, height, 1, format, GL_UNSIGNED_BYTE, data);

        // Free the data
        stbi_image_free(data);

        ++image_id;
    }
}
