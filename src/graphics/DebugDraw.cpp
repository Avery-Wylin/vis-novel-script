#include "DebugDraw.h"

namespace DebugDraw{
    VAO line_vao;
    Shader debug_shader;

    uint32_t indicies[MAX_INDICES];
    float pos[MAX_POINTS];
    uint8_t col[MAX_POINTS];
    unsigned int point_count;
    unsigned int index_count;
    uint8_t red = 255, green = 255, blue = 255;
}

void DebugDraw::init_assets() {
    line_vao.allocate();
    debug_shader.load( "debug" );
}

void DebugDraw::close_assets() {
    line_vao.free();
    debug_shader.free();
}

void DebugDraw::draw( View &view ) {
    glEnable( GL_PRIMITIVE_RESTART_FIXED_INDEX );
    glEnable( GL_PRIMITIVE_RESTART );

    Shader::bind(debug_shader);
    Shader::uniformMat4f( UNIFORM_CAMERA, view.getCombinedTransform() );

    line_vao.load_attrb_byte( ATTRB_COL, 0, 0, 3, point_count, true, col );
    line_vao.load_attrb_float( ATTRB_POS, 0, 0, 3, point_count, pos );
    line_vao.load_index( index_count, indicies );
    line_vao.bind();
    glDrawElements( GL_LINE_STRIP, index_count, GL_UNSIGNED_INT, 0 );

    line_vao.unbind();
    glDisable( GL_PRIMITIVE_RESTART_FIXED_INDEX );
    glDisable( GL_PRIMITIVE_RESTART );

    point_count = 0;
    index_count = 0;
}

void DebugDraw::color( uint8_t r, uint8_t g, uint8_t b ) {
    red = r;
    green = g;
    blue = b;
}

void DebugDraw::line( float x1, float y1, float z1, float x2, float y2, float z2 ) {
    if( index_count + 3 > MAX_INDICES || point_count + 6 > MAX_POINTS )
        return;
    pos[point_count + 0] = x1;
    pos[point_count + 1] = y1;
    pos[point_count + 2] = z1;
    pos[point_count + 3] = x2;
    pos[point_count + 4] = y2;
    pos[point_count + 5] = z2;

    col[point_count + 0] = red;
    col[point_count + 1] = green;
    col[point_count + 2] = blue;
    col[point_count + 3] = red;
    col[point_count + 4] = green;
    col[point_count + 5] = blue;

    indicies[index_count] = point_count / 3;
    indicies[index_count + 1] = point_count / 3 + 1;
    indicies[index_count + 2] = UINT32_MAX;
    point_count += 6;
    index_count += 3;
}

void DebugDraw::polygon( versor v, vec3 position, float radius, unsigned int resolution, unsigned int plane ) {
    if( index_count + resolution + 2 > MAX_INDICES || point_count + 3 * resolution > MAX_POINTS )
        return;
    unsigned int p = point_count;
    vec3 *dest;
    for( unsigned int i = 0; i < resolution; ++i ) {
        pos[point_count + ( 0 + plane ) % 3] = radius * cos( ( float )i / resolution * 6.2832 );
        pos[point_count + ( 1 + plane ) % 3] = 0;
        pos[point_count + ( 2 + plane ) % 3] = radius * sin( ( float )i / resolution * 6.2832 );

        dest = reinterpret_cast<vec3 *>( &pos[point_count] );
        glm_quat_rotatev(v, *dest, *dest);
        glm_vec3_add(*dest, position, *dest);

        col[point_count + 0] = red;
        col[point_count + 1] = green;
        col[point_count + 2] = blue;

        indicies[index_count] = point_count / 3;

        ++index_count;
        point_count += 3;
    }

    indicies[index_count] = p / 3;
    indicies[index_count + 1] = UINT32_MAX;
    index_count += 2;
}


void DebugDraw::box( versor v, vec3 position, float x, float y, float z, float a, float b, float c ) {
    if( index_count + 20 > MAX_INDICES || point_count + 24 > MAX_POINTS )
        return;

    pos[point_count + 0] = x;
    pos[point_count + 1] = y;
    pos[point_count + 2] = z;

    pos[point_count + 3] = a;
    pos[point_count + 4] = y;
    pos[point_count + 5] = z;

    pos[point_count + 6] = a;
    pos[point_count + 7] = y;
    pos[point_count + 8] = c;

    pos[point_count + 9] = x;
    pos[point_count + 10] = y;
    pos[point_count + 11] = c;

    pos[point_count + 12] = x;
    pos[point_count + 13] = b;
    pos[point_count + 14] = z;

    pos[point_count + 15] = a;
    pos[point_count + 16] = b;
    pos[point_count + 17] = z;

    pos[point_count + 18] = a;
    pos[point_count + 19] = b;
    pos[point_count + 20] = c;

    pos[point_count + 21] = x;
    pos[point_count + 22] = b;
    pos[point_count + 23] = c;

    vec3 *dest;
    for( int i = 0; i < 24; i += 3 ) {
        dest = reinterpret_cast<vec3 *>( &pos[point_count + i] );
        glm_quat_rotatev(v, *dest, *dest);
        glm_vec3_add(*dest, position, *dest);
        col[point_count + i] = red;
        col[point_count + i + 1] = green;
        col[point_count + i + 2] = blue;
    }

    unsigned int p = point_count / 3;

    indicies[index_count + 0] = p;
    indicies[index_count + 1] = p + 1;
    indicies[index_count + 2] = p + 2;
    indicies[index_count + 3] = p + 3;
    indicies[index_count + 4] = p;
    indicies[index_count + 5] = p + 4;
    indicies[index_count + 6] = p + 5;
    indicies[index_count + 7] = p + 6;
    indicies[index_count + 8] = p + 7;
    indicies[index_count + 9] = p + 4;
    indicies[index_count + 10] = UINT32_MAX;

    indicies[index_count + 11] = p + 1;
    indicies[index_count + 12] = p + 5;
    indicies[index_count + 13] = UINT32_MAX;

    indicies[index_count + 14] = p + 2;
    indicies[index_count + 15] = p + 6;
    indicies[index_count + 16] = UINT32_MAX;

    indicies[index_count + 17] = p + 3;
    indicies[index_count + 18] = p + 7;
    indicies[index_count + 19] = UINT32_MAX;

    point_count += 24;
    index_count += 20;

    axis( v, position );

}

void DebugDraw::box(float x, float y, float z, float a, float b, float c ) {
    if( index_count + 20 > MAX_INDICES || point_count + 24 > MAX_POINTS )
        return;

    pos[point_count + 0] = x;
    pos[point_count + 1] = y;
    pos[point_count + 2] = z;

    pos[point_count + 3] = a;
    pos[point_count + 4] = y;
    pos[point_count + 5] = z;

    pos[point_count + 6] = a;
    pos[point_count + 7] = y;
    pos[point_count + 8] = c;

    pos[point_count + 9] = x;
    pos[point_count + 10] = y;
    pos[point_count + 11] = c;

    pos[point_count + 12] = x;
    pos[point_count + 13] = b;
    pos[point_count + 14] = z;

    pos[point_count + 15] = a;
    pos[point_count + 16] = b;
    pos[point_count + 17] = z;

    pos[point_count + 18] = a;
    pos[point_count + 19] = b;
    pos[point_count + 20] = c;

    pos[point_count + 21] = x;
    pos[point_count + 22] = b;
    pos[point_count + 23] = c;

    for( int i = 0; i < 24; i += 3 ) {
        col[point_count + i] = red;
        col[point_count + i + 1] = green;
        col[point_count + i + 2] = blue;
    }

    unsigned int p = point_count / 3;

    indicies[index_count + 0] = p;
    indicies[index_count + 1] = p + 1;
    indicies[index_count + 2] = p + 2;
    indicies[index_count + 3] = p + 3;
    indicies[index_count + 4] = p;
    indicies[index_count + 5] = p + 4;
    indicies[index_count + 6] = p + 5;
    indicies[index_count + 7] = p + 6;
    indicies[index_count + 8] = p + 7;
    indicies[index_count + 9] = p + 4;
    indicies[index_count + 10] = UINT32_MAX;

    indicies[index_count + 11] = p + 1;
    indicies[index_count + 12] = p + 5;
    indicies[index_count + 13] = UINT32_MAX;

    indicies[index_count + 14] = p + 2;
    indicies[index_count + 15] = p + 6;
    indicies[index_count + 16] = UINT32_MAX;

    indicies[index_count + 17] = p + 3;
    indicies[index_count + 18] = p + 7;
    indicies[index_count + 19] = UINT32_MAX;

    point_count += 24;
    index_count += 20;
}

void DebugDraw::axis( versor v, vec3 position ) {
    if( index_count + 9 > MAX_INDICES || point_count + 12 > MAX_POINTS )
        return;
    pos[point_count + 0] = 0;
    pos[point_count + 1] = 0;
    pos[point_count + 2] = 0;
    col[point_count + 0] = 255;
    col[point_count + 1] = 255;
    col[point_count + 2] = 255;

    pos[point_count + 3] = 1;
    pos[point_count + 4] = 0;
    pos[point_count + 5] = 0;
    col[point_count + 3] = 255;
    col[point_count + 4] = 0;
    col[point_count + 5] = 0;

    pos[point_count + 6] = 0;
    pos[point_count + 7] = 1;
    pos[point_count + 8] = 0;
    col[point_count + 6] = 0;
    col[point_count + 7] = 255;
    col[point_count + 8] = 0;

    pos[point_count + 9] = 0;
    pos[point_count + 10] = 0;
    pos[point_count + 11] = 1;
    col[point_count + 9] = 0;
    col[point_count + 10] = 0;
    col[point_count + 11] = 255;

    vec3 *dest;
    for( int i = 0; i < 12; i += 3 ) {
        dest = reinterpret_cast<vec3 *>( &pos[point_count + i] );
        glm_quat_rotatev(v, *dest, *dest);
        glm_vec3_add(*dest, position, *dest);
    }

    unsigned int p = point_count / 3;

    indicies[index_count + 0] = p;
    indicies[index_count + 1] = p + 1;
    indicies[index_count + 2] = UINT32_MAX;

    indicies[index_count + 3] = p;
    indicies[index_count + 4] = p + 2;
    indicies[index_count + 5] = UINT32_MAX;

    indicies[index_count + 6] = p;
    indicies[index_count + 7] = p + 3;
    indicies[index_count + 8] = UINT32_MAX;

    point_count += 12;
    index_count += 9;
}



