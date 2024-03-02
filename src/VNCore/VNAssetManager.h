#ifndef VNASSETMANAGER_H
#define VNASSETMANAGER_H
#include <unordered_map>
#include <vector>
#include "Mesh.h"
#include "VAO.h"
#include "Shader.h"
#include "Texture.h"
#include "Armature.h"
#include "View.h"
#include <memory>

struct ModelContainer;
struct ShaderContainer;

namespace Interp{
    static const uint8_t
    LINEAR = 0,
    POWER = 1,
    EASE = 2,
    SPRING = 3,
    BOUNCE = 4;

    inline float eval(uint8_t type, float t, float m){
        switch(type){
            case LINEAR: return t;
            case POWER: return pow(t,m);
            case EASE: return t * (3 * (1-t) * (m*(1-2*t)+t) + t*t);
            case SPRING: return 1-( cos(floor(m)*t*3.1415) * (1-t)*(1-t));
            case BOUNCE: return 1-abs( cos(floor(m)*t*3.1415) * (1-t));
        }
    };
};

struct KeyframeRot{
    float length = 0;
    float t = 0;
    versor from = GLM_QUAT_IDENTITY_INIT;
    versor to =  GLM_QUAT_IDENTITY_INIT;
    uint8_t type = Interp::LINEAR;
    float mod = 2;

    void update(versor dest, float time_seconds){
        if(length <=0)
            return;
        t+= time_seconds;
        float v = t/length;
        if( t >= length ){
            t = 0;
            length = 0;
            v = 1;
        }
        v = Interp::eval(type, v, mod);
        glm_quat_nlerp(from, to, v, dest);
    }

};

struct KeyframePos{
    float length = 0;
    float t = 0;
    vec3 from = GLM_VEC3_ZERO_INIT;
    vec3 to = GLM_VEC3_ZERO_INIT;
    uint8_t type = Interp::LINEAR;
    float mod = 2;


    void update(vec3 dest, float time_seconds){
        if(length <=0)
            return;
        t+= time_seconds;
        float v = t/length;
        if( t >= length ){
            t = 0;
            length = 0;
            v = 1;
        }
        v = Interp::eval(type, v, mod);
        glm_vec3_lerp(from, to, v, dest);
    }
};

struct KeyframeFloat{
    float length = 0;
    float t = 0;
    float from = 0;
    float to = 0;
    uint8_t type = Interp::LINEAR;
    float mod = 2;

    void update(float &dest, float time_seconds){
        if(length <=0)
            return;
        t+= time_seconds;
        float v = t/length;
        if( t >= length ){
            t = 0;
            length = 0;
            v = 1;
        }
        v = Interp::eval(type, v, mod);
        dest = (to - from) * v + from;
    }
};

struct ObjectInstance {
    static const uint8_t
    OWNER_NULL = 0,
    OWNER_MODEL = 1,
    OWNER_IMG = 2;

    uint8_t owner_type = OWNER_NULL;
    uint32_t owner = 0;

    bool enabled = true;

    vec3 position = GLM_VEC3_ZERO_INIT;
    versor rotation = GLM_VEC3_ZERO_INIT;
    float scale = 1;

    KeyframePos key_position;
    KeyframeRot key_rotation;
    KeyframeFloat key_scale;

    void update(float t);

};

struct ModelContainer {
    uint32_t shader_id = 0;
    Mesh mesh;
    std::shared_ptr<VAO> vao;
    std::vector<uint32_t> objects;
    void draw();

    ModelContainer(){
        vao = std::shared_ptr<VAO>(new VAO());
    }
};

// Image containers use a special shader and a quad to draw
struct ImageContainer {
    string filename;
    std::shared_ptr<Texture> image;
    std::vector<uint32_t> objects;
    bool needs_loaded = false;
    void draw();

    ImageContainer(){
        image = std::shared_ptr<Texture>(new Texture());
    }
};


struct ShaderContainer {
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    std::string filename;
    bool needs_compiled = false;
    std::vector<uint32_t> models;
    void draw();

    ShaderContainer(){
        shader = std::shared_ptr<Shader>(new Shader());
        texture = std::shared_ptr<Texture>(new Texture());
    }
};

// Assets are ordered in a certain hierarchy
// Shaders are at the top and contain a list of models
// Models contain a mesh and a list of objects
namespace VNAssets {
    extern View view;
    extern KeyframeFloat key_fov;
    extern KeyframePos key_pos;
    extern KeyframePos key_focus;
    extern vec3 focus;
    extern Shader image_shader;
    extern VAO image_vao;


    ShaderContainer *get_shader( const std::string& name );
    ModelContainer *get_model( const std::string& name );
    ImageContainer *get_image( const std::string& name );
    ObjectInstance *get_object( const std::string& name );
    ArmatureInfo *get_armature_info( const std::string& name );

    ShaderContainer* create_shader(const std::string& name);
    ModelContainer* create_model(const std::string& name);
    ImageContainer* create_image(const std::string& name);
    ObjectInstance* create_object(const std::string& name);
    ArmatureInfo* load_armature(const std::string& name, const std::string &filename);

    bool link_model_to_shader(const std::string& model, const std::string& shader);
    bool link_object_to_model(const std::string& object, const std::string& model);
    bool link_object_to_image(const std::string& object, const std::string& image);
    bool link_object_to_image(const std::string& object, const std::string& image);

    void init();
    void close();
    void update(float t);
    void draw();
}

#endif // VNASSETMANAGER_H
