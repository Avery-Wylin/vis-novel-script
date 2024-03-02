#include <chrono>
#include <thread>
#include "Window.h"
#include "DebugDraw.h"
#include "definitions.h"
#include "GUI.h"
#include "VNAssetManager.h"
#include "VNInterpreter.h"

// GL Error Callback
static void GLAPIENTRY glMessageCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam ) {
    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
        type, severity, message );
}

// GLFW Error Callback
static void glfwErrorCallback( int error, const char *description ) {
    fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

void Window::init() {

    // Start GLFW, close the program if it fails
    if( !glfwInit() ) {
        puts( "Failed to start GLFW" );
        fflush( stdout );
        exit( 1 );
    }

    // Enable GLFW Error Callback
    glfwSetErrorCallback( glfwErrorCallback );

    // Initialize GLFW window
    window = glfwCreateWindow( 1, 1, "VNI", nullptr, nullptr );

    // Close if window failed
    if( !window ) {
        puts( "Failed to create GLFW window." );
        fflush( stdout );
        exit( 1 );
    }

    // Use this window
    glfwMakeContextCurrent( window );

    // Set the monitor and vidmode
    monitor = glfwGetPrimaryMonitor();
    vidmode = glfwGetVideoMode( monitor );

    // Set the window to fill half the monitor, and center it
    glfwSetWindowSize( window, vidmode->width / 2, vidmode->height / 2 );
    glfwSetWindowPos( window, vidmode->width / 4, vidmode->height / 4 );
    glfwGetFramebufferSize( window, &window_width, &window_height );

    // Assign this window object as the user pointer
    glfwSetWindowUserPointer( window, this );

    // Set the callback functions
    glfwSetKeyCallback( window, key_callback );
    glfwSetWindowSizeCallback( window, win_resize_callback );
    glfwSetCharCallback( window, char_callback );
    glfwSetCursorPosCallback( window, cursor_move_callback );
    glfwSetMouseButtonCallback( window, click_callback );

    // Enable OpenGL and its Debug
    gladLoadGL();
    glDebugMessageCallback( glMessageCallback, 0 );

    // Configure OpenGL
    glEnable( GL_DEBUG_OUTPUT );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );
    // glEnable( GL_FRAMEBUFFER_SRGB ); Uhhh this thing...
    glLineWidth( 3 );

    // Set Vsync
    glfwSwapInterval( 1 );

    // Initialize Debug Drawing
    DebugDraw::init_assets();

    // Get the ratio to draw the fbo
    // NOTE Changing the FBO Ratio requires reformatting all GUIs
    fbo_ratio = ( float )vidmode->width / vidmode->height;

    // Clamp the ratio to a reasonable bounds
    fbo_ratio = glm_clamp( fbo_ratio, 1.5, 2 );

    // Create the FBO, this will fill the entire drawing ratio area
    render_fbo.allocate( vidmode->height * fbo_ratio, vidmode->height );

    // Assign a renderable texture to the render fbo
    render_fbo.create_depth_attachment();
    render_fbo_texture.from_FBO( render_fbo, 0, GL_NEAREST );
    render_fbo.bind_default( window_width, window_height );

    // Initialize GUI assets
    GUI::init_assets();
    GUI::ratio = fbo_ratio;

    // Initialize VN Assets
    VNAssets::init();

}

void Window::run() {

    // Timing variables to limit FPS
    std::chrono::time_point<std::chrono::steady_clock> start_time, stop_time;
    std::chrono::duration<double> elapsed_time;
    double update_time = 1.0 / ( FPS );

    // Set the start position for the interpreter
    VNI::open_file("main");
    VNI::main_interpreter.switch_file("main", true);
    VNI::main_interpreter.jump( 0 , true);

    while( !glfwWindowShouldClose( window ) ){
        start_time = std::chrono::steady_clock::now();

        // Poll GLFW events
        glfwPollEvents();

        if( !paused ) {

            // Update the menus
            Menu::update();

            // Update the interpreter, this will read all possible line up to a wait
            VNI::update(update_time);

            // Update asset animations
            VNAssets::update(update_time);

            // Render
            render_fbo.bind();
            glClearColor( 0.5, 0.5, 0.5, 1 );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glEnable( GL_DEPTH_TEST );
            glEnable( GL_CULL_FACE );

            // Render to the FBO
            VNAssets::draw();

            // Draw any debug lines
            DebugDraw::draw( view );

            // Determine where to draw the render FBO

            // Switch to the window fbo
            FBO::bind_default( window_width, window_height );

            // Get render dimensions
            glfwGetFramebufferSize( window, &window_width, &window_height );

            // Limit render fbo by width
            if( window_width > window_height * fbo_ratio ) {
                glViewport( ( window_width - window_height * fbo_ratio ) / 2, 0, window_height * fbo_ratio, window_height );
            }

            // Limit render fbo by height
            else {
                glViewport( 0, ( window_height - window_width / fbo_ratio ) / 2, window_width, window_width / fbo_ratio );
            }

            // Clear the window FBO
            glClearColor( 0, 0, 0, 0 );
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
            glDisable( GL_DEPTH_TEST );
            glDisable( GL_CULL_FACE );

            // Draw the render FBO to the window FBO
            Shader::unbind();
            render_fbo_texture.bind( 0 );
            glBegin( GL_QUADS );
            glColor3f( 1, 1, 1 );
            glTexCoord2f( 0, 0 );
            glVertex2f( -1, -1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 1, 0 );
            glVertex2f( 1, -1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 1, 1 );
            glVertex2f( 1, 1 );

            glColor3f( 1, 1, 1 );
            glTexCoord2f( 0, 1 );
            glVertex2f( -1, 1 );
            glEnd();

            // Draw the GUI
            glEnable(GL_BLEND);
            glDisable( GL_DEPTH_TEST );
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            GUI::draw();
            glDisable(GL_BLEND);

            // Swap buffers
            glfwSwapBuffers( window );

        }


        // Sleep until the FPS time is met
        stop_time = std::chrono::steady_clock::now();
        elapsed_time = stop_time - start_time;

        if( elapsed_time.count() < update_time ) {
            std::this_thread::sleep_until( stop_time + ( std::chrono::duration<double> )update_time - elapsed_time );
        }
    }

    close();
};


void Window::close() {
    GUI::close_assets();
    DebugDraw::close_assets();
    VNI::stop();
    VNAssets::close();
    puts("Closing Window");
    fflush(stdout);
}

void key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    // Exit the window with alt + escape
    if( action == GLFW_PRESS && key == GLFW_KEY_ESCAPE && mods & GLFW_MOD_ALT )
        glfwSetWindowShouldClose( window, GLFW_TRUE );

    // Pass to GUI
    if( action == GLFW_REPEAT ) {
        GUI::key_input( key, mods );
    }
    else if( action == GLFW_PRESS ) {

        // Enter key should always resume the VNI
        if(key == GLFW_KEY_ENTER)
            VNI::resume();

        // Default VNI debug/control keys use ctrl + key, these are hard-coded (for now?)
        if(mods & GLFW_MOD_CONTROL){
            switch(key){
                case GLFW_KEY_R:
                    VNI::recompile(); break;
            }
        }
        GUI::key_input( key, mods );
    }
}

void win_resize_callback( GLFWwindow *window, int width, int height ) {
    // Window *w = static_cast<Window*>( glfwGetWindowUserPointer( window ) );
}

void char_callback( GLFWwindow *window, unsigned int  codepoint ) {
    // Pass text input to the GUI
    GUI::char_input( codepoint );
}

void cursor_move_callback( GLFWwindow *window, double x, double y ) {
    Window *w = static_cast<Window *>( glfwGetWindowUserPointer( window ) );

    int width, height;
    glfwGetWindowSize( window, &width, &height );
    float r = w->get_ratio();

    // Offset the input to that of the subwindow
    if( width > height * r ) {
        x -= ( width - r * height ) / 2;
        width = height * r;
    }
    else {
        y -= ( height - width / r ) / 2;
        height = width / r;
    }

    // Convert to a 0 to 1 scale and flip y
    x /= width;
    y = 1 - ( y / height );

    // Pass to GUI
    GUI::highlight( x, y );
}

void click_callback( GLFWwindow *window, int key, int action, int mods ) {
    Window *w = static_cast<Window *>( glfwGetWindowUserPointer( window ) );

    // Only allow down-presses
    if( action != GLFW_PRESS )
        return;

    double x, y;
    int width, height;
    glfwGetCursorPos( window, &x, &y );
    glfwGetWindowSize( window, &width, &height );
    float r = w->get_ratio();

    // Offset the input to that of the subwindow
    if( width > height * r ) {
        x -= ( width - r * height ) / 2;
        width = height * r;
    }
    else {
        y -= ( height - width / r ) / 2;
        height = width / r;
    }

    // Convert to a 0 to 1 scale and flip y
    x /= width;
    y = 1 - ( y / height );

    // Pass to GUI
    GUI::select( x, y );
}

