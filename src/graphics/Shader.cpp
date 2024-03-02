#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "library/glad_common.h"
#include <cglm/cglm.h>
#include <inttypes.h>

using std::string;
using std::ifstream;
using std::stringstream;

static Shader* active;

Shader::Shader() {
    program_id = 0;
    v_shader_id = 0;
    f_shader_id = 0;
}

void Shader::load( string v_shader_file, string f_shader_file ) {
    free();
    if(f_shader_file.empty()){
        f_shader_file = v_shader_file;
    }
    v_shader_name = v_shader_file;
    f_shader_name = f_shader_file;
    v_shader_id = glCreateShader( GL_VERTEX_SHADER );
    f_shader_id = glCreateShader( GL_FRAGMENT_SHADER );
    loadFromFile( (std::string)DIR_SHADERS + v_shader_name + ".vert", v_shader_id );
    loadFromFile( (std::string)DIR_SHADERS + f_shader_name + ".frag", f_shader_id );
    program_id = glCreateProgram();
    glAttachShader( program_id, v_shader_id );
    glAttachShader( program_id, f_shader_id );
    linkDefaultAttributes();
    glLinkProgram( program_id );
    linkDefaultUniforms();
    glValidateProgram( program_id );
}

Shader::~Shader() {
    free();
}

void Shader::linkDefaultUniforms() {
    uniform_locations[UNIFORM_TRANSFORM] = glGetUniformLocation( program_id, "transform" );
    uniform_locations[UNIFORM_CAMERA] = glGetUniformLocation( program_id, "camera" );
    uniform_locations[UNIFORM_PROJ] = glGetUniformLocation( program_id, "proj" );
    uniform_locations[UNIFORM_CAM_POS] = glGetUniformLocation( program_id, "cam_pos" );
    uniform_locations[UNIFORM_COLOR] = glGetUniformLocation( program_id, "object_color" );
    uniform_locations[UNIFORM_COLOR2] = glGetUniformLocation( program_id, "object_color2" );
    uniform_locations[UNIFORM_FACTOR] = glGetUniformLocation( program_id, "factor" );
    uniform_locations[UNIFORM_ZENITH] = glGetUniformLocation( program_id, "zenith" )  ;
    uniform_locations[UNIFORM_HORIZON] = glGetUniformLocation( program_id, "horizon" ) ;
    uniform_locations[UNIFORM_SUN] = glGetUniformLocation( program_id, "sun" ) ;
    uniform_locations[UNIFORM_SUN_DIR] = glGetUniformLocation( program_id, "sun_dir" ) ;
    uniform_locations[UNIFORM_FOG] = glGetUniformLocation( program_id, "fog_data" ) ;
    uniform_locations[UNIFORM_JOINTS] = glGetUniformLocation( program_id, "joints" ) ;
}

void Shader::linkUniform( std::string uniformName, Uniform uniform ) {
    if( uniform >= 0 && uniform < NUM_UNIFORMS )
        uniform_locations[uniform] =  glGetUniformLocation( program_id, uniformName.c_str() );
}

void Shader::uniformMat4f( Uniform uniform, const mat4 &mat ) {
    glUniformMatrix4fv( active->uniform_locations[uniform], 1, false, mat[0] );
}

void Shader::uniformMat4fArray( Uniform uniform, mat4 *matrices, unsigned int count ) {
    glUniformMatrix4fv( active->uniform_locations[uniform], count, false, matrices[0][0]);
}

void Shader::uniformVec4f( Uniform uniform, const vec4 &vec ) {
    glUniform4fv( active->uniform_locations[uniform], 1, vec );
}

void Shader::uniformVec3f( Uniform uniform, const vec3 &vec ) {
    glUniform3fv( active->uniform_locations[uniform], 1, vec );
}

void Shader::uniformVec2f( Uniform uniform, const vec2 &vec ) {
    glUniform2fv( active->uniform_locations[uniform], 1, vec );
}

void Shader::uniformFloat( Uniform uniform, float f ) {
    glUniform1f( active->uniform_locations[uniform], f );
}

void Shader::uniformInt( Uniform uniform, int i ) {
    glUniform1i( active->uniform_locations[uniform], i );
}

void Shader::uniformUint( Uniform uniform, uint32_t i ) {
    glUniform1ui( active->uniform_locations[uniform], i );
}

bool Shader::bind(Shader &shader) {
    active = &shader;
    if( glIsProgram( active->program_id ) == GL_TRUE ){
        glUseProgram( active->program_id );
        return true;
    }
    else{
        glUseProgram(0);
        return false;
    }
}

void Shader::unbind() {
    active = nullptr;
    glUseProgram( 0 );
}

void Shader::recompile() {
    loadFromFile( (std::string)DIR_SHADERS + v_shader_name + ".vert", v_shader_id );
    loadFromFile( (std::string)DIR_SHADERS + f_shader_name + ".frag", f_shader_id );
    if( program_id == 0 ) {
        program_id = glCreateProgram();
    }
    glAttachShader( program_id, v_shader_id );
    glAttachShader( program_id, f_shader_id );
    linkDefaultAttributes();
    glLinkProgram( program_id );
    linkDefaultUniforms();
    glValidateProgram( program_id );
}

void Shader::linkDefaultAttributes() {
    glBindAttribLocation( program_id, ATTRB_POS, "pos" );
    glBindAttribLocation( program_id, ATTRB_NORM, "normal" );
    glBindAttribLocation( program_id, ATTRB_UV, "uv" );
    glBindAttribLocation( program_id, ATTRB_COL, "vertex_color" );
    glBindAttribLocation( program_id, ATTRB_WEIGHTS, "weights" );
    glBindAttribLocation( program_id, ATTRB_JOINTS, "joint_ids" );
    glBindAttribLocation( program_id, ATTRB_SK_POS, "sk_pos" );
    glBindAttribLocation( program_id, ATTRB_SK_NORM, "sk_normal" );
}

void Shader::loadFromFile( string filePath, int shaderID ) {
    if( shaderID == 0 ) {
        std::printf( "Shader not given an ID\n" );
        return;
    }

    ifstream reader;
    stringstream src;

    reader.open( filePath, std::ios::in );
    //reader.seekg(std::ios::end);

    if( reader.is_open() ) {
        src << reader.rdbuf();
        reader.close();
        string srcString = src.str();
        const char *srcChar = srcString.c_str();
        glShaderSource( shaderID, 1, &srcChar, NULL );
        glCompileShader( shaderID );
        int status;
        glGetShaderiv( shaderID, GL_COMPILE_STATUS, &status );
        if( status == GL_FALSE ) {
            std::printf( "Shader %s failed to compile\n", filePath.c_str() );
            char info[500];
            glGetShaderInfoLog( shaderID, 500, NULL, info );
            std::printf( "%s", info );
            return;
        }


    }
    else {
        std::cout << "Could not read shader" << filePath << std::endl;
    }


}

void Shader::free() {
    if(active == this)
        active = nullptr;
    if( program_id != 0 ) {
        glDetachShader( program_id, v_shader_id );
        glDetachShader( program_id, f_shader_id );
        glDeleteShader( v_shader_id );
        glDeleteShader( f_shader_id );
        glDeleteProgram( program_id );
        program_id = 0;
    }
}
