#ifndef FBO_H
#define FBO_H

#include "definitions.h"
#include <stdlib.h>


class FBO {
public:
    FBO();
    virtual ~FBO();
    
    static void bind_default(unsigned int xres, unsigned int yres);
    
    void allocate(unsigned int width, unsigned int height);
    bool is_allocated();
    void free();
    void bind();
    void create_depth_attachment();
    void create_color_attachment(unsigned int slot);
    const GLuint& get_fboid(){return fboid;};
    GLuint get_color_render_buffer(unsigned int slot);
    unsigned int get_width(){return x_res;};
    unsigned int get_height(){return y_res;};
    void draw_to_default(unsigned int slot, unsigned int w, unsigned int h);
    void copy_to(FBO &dest, uint32_t scaleType);

private:
    unsigned int x_res, y_res;
    GLuint fboid = 0;
    GLuint color_buffers[FBO_MAX_COLOR_ATTACHMENTS] = {0,0,0,0,0,0,0,0};
    GLuint depth_attachment = 0;
    
    // Forbid copy
    FBO(const FBO&);
    FBO operator=(const FBO&);
    
};

#endif /* FBO_H */

