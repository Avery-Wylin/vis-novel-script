#ifndef WINDOW_H
#define WINDOW_H

#include "glad_common.h"
#include "Texture.h"
#include "FBO.h"
#include "View.h"
#include "pthread.h"
#include "Menu.h"

// Callback Functions
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void win_resize_callback(GLFWwindow *window, int width, int height);
void char_callback(GLFWwindow *window, unsigned int  codepoint);
void cursor_move_callback( GLFWwindow *window, double xpos, double ypos);
void click_callback(GLFWwindow *window, int key, int action, int mods);

class Window {
    GLFWwindow *window = nullptr;
    GLFWmonitor *monitor = nullptr;
    const GLFWvidmode *vidmode;

    int
    window_width = 0,
    window_height = 0;

    // Ratio of the main rendering FBO, is set when window is intialized
    float fbo_ratio = 1;

    // The camera view for the window
    View view;

    // Main rendering FBO, more may be added for effects
    FBO render_fbo;
    Texture render_fbo_texture;
    // TODO create an asset-render handler

public:
    bool paused = false;

    inline float get_ratio(){return fbo_ratio;};
    void init();
    void run();
    void close();
};

#endif // WINDOW_H
