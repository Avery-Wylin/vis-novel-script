#include "library/glad.h"
#include "FBO.h"
#include <iostream>

static FBO* active;

FBO::FBO(){
    x_res = 0;
    y_res = 0;
    fboid = 0;
}

FBO::~FBO() {
    free();
}

/*
 * Creates the FBO on the GPU.
 * Nothing is performed if the FBO is already allcoated.
 */
void FBO::allocate(unsigned int width, unsigned int height){
    if( is_allocated())
        return;
    // Generate the framebuffer, bind it, and set the resoluton
    glGenFramebuffers(1, &fboid);
    glBindFramebuffer(GL_FRAMEBUFFER,fboid);
    x_res = width;
    y_res = height;
}

/*
 * Deletes the FBO on the GPU, including all attachments.
 */
void FBO::free(){
    if(!is_allocated())
        return;
    
    // Delete all color attachments
    for(unsigned int i = 0; i < FBO_MAX_COLOR_ATTACHMENTS; i++){
        if( color_buffers[i] == 0)
            continue;
        glDeleteRenderbuffers(1,&color_buffers[i]);
        color_buffers[i] = 0;
    }
    
    // Delete depth attachment
    if( depth_attachment != 0){
        glDeleteRenderbuffers(1,&depth_attachment );
        depth_attachment = 0;
    }
    
    // Delete the FBO
    glDeleteFramebuffers(1,&fboid);
    
}

/*
 * Returns whether or not the FBO is allocated on the GPU.
 */
bool FBO::is_allocated(){
    return fboid != 0;
}

/*
 * Binds the FBO, all drawing will be performed on this FBO.
 * Nothing will be performed if the FBO is unallocated.
 */
void FBO::bind(){
    if(!is_allocated())
        return;
    
    glBindFramebuffer(GL_FRAMEBUFFER, fboid);
    // Set the size of the viewport to the FBO's resolution
    glViewport(0, 0, x_res, y_res );
}

/*
 * Sets the FBO to the default one that renders to the screen.
 */
void FBO::bind_default(unsigned int xres, unsigned int yres){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, xres, yres);
}

/*
 * Creates a depth attachment for the FBO.
 * Nothing will be performed if the attachment exist or the FBO is unallocated.
 */
void FBO::create_depth_attachment(){
    if( depth_attachment != 0 || !is_allocated())
        return;
    bind();
    glGenRenderbuffers(1,&depth_attachment );
    glBindRenderbuffer(GL_RENDERBUFFER, depth_attachment );
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, x_res, y_res );
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_attachment );
}

/*
 * Create a color attachment for the FBO.
 * Color attachments allow for rendering to multiple targets using:
 * layout (location = <slot#>) out <uniform>
 * A slot can be chosen up to MAX_COLOR_ATTACHMENTS.
 * Nothing will be performed if the attachment exist, the slot number is invalid, or the FBO is unallocated.
 */
void FBO::create_color_attachment(unsigned int slot){
    if(!(slot < FBO_MAX_COLOR_ATTACHMENTS && is_allocated() && color_buffers[slot] != 0))
        return;
    
    bind();
    glGenRenderbuffers(1,&color_buffers[slot]);
    glBindRenderbuffer(GL_RENDERBUFFER, color_buffers[slot]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, x_res, y_res );
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, color_buffers[slot]);
    
    // Update the attachments that the shader will draw to
    GLenum drawableBuffers[FBO_MAX_COLOR_ATTACHMENTS];

    for(unsigned int i = 0; i < FBO_MAX_COLOR_ATTACHMENTS; i++){
        if( color_buffers[i] == 0)
            drawableBuffers[i] = GL_NONE;
        else
            drawableBuffers[i] = GL_COLOR_ATTACHMENT0+i;
    }

    glDrawBuffers( FBO_MAX_COLOR_ATTACHMENTS, drawableBuffers);

}

GLuint FBO::get_color_render_buffer (unsigned int slot){
    if(slot > FBO_MAX_COLOR_ATTACHMENTS )
        return 0;
    return color_buffers[slot];
}

void FBO::draw_to_default(unsigned int slot, unsigned int w, unsigned int h){
    if(!is_allocated() || slot >= FBO_MAX_COLOR_ATTACHMENTS )
        return;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboid);
    glReadBuffer(GL_COLOR_ATTACHMENT0+slot);
    glBlitFramebuffer(0,0, x_res, y_res,0,0,w,h,GL_COLOR_BUFFER_BIT,GL_LINEAR);
}

void FBO::copy_to(FBO &dest, uint32_t scaleType){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.fboid);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboid);
    glBlitFramebuffer(0, 0, x_res, y_res, 0, 0, dest.x_res, dest.y_res, GL_COLOR_BUFFER_BIT, scaleType);
}
