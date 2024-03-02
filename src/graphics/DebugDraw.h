#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

#include "VAO.h"
#include <vector>
#include "View.h"

namespace DebugDraw{

    static const unsigned int MAX_POINTS = 3000;
    static const unsigned int MAX_INDICES = 2000;

    extern VAO line_vao;
    extern Shader debug_shader;

    extern uint32_t indicies[MAX_INDICES];
    extern float pos[MAX_POINTS];
    extern uint8_t col[MAX_POINTS];
    extern unsigned int point_count;
    extern unsigned int index_count;
    extern uint8_t red, green, blue;

    void init_assets();
    void close_assets();
    void draw(View &view);

    // Draw functions
    void line(float x1, float y1, float z1, float x2, float y2, float z2);
    void polygon(versor v, vec3 position, float radius, unsigned int resolution, unsigned int plane);
    void box(versor v, vec3 position, float x, float y, float z, float a, float b, float c);
    void box(float x, float y, float z, float a, float b, float c );
    void axis(versor v, vec3 position);
    void color(uint8_t r, uint8_t g, uint8_t b);

}

#endif // DEBUGDRAW_H
