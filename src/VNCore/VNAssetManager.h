#ifndef VNASSETMANAGER_H
#define VNASSETMANAGER_H
#include <unordered_map>
#include <vector>
#include <set>
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

    bool enabled = true;
    bool updated = false;
    uint32_t shader = 0, model = 0, parent = 0;
    std::vector<uint32_t> children;
    uint8_t parent_joint = 0;

    Armature armature;
    vec3 position = GLM_VEC3_ZERO_INIT;
    vec3 tex_id = GLM_VEC3_ZERO_INIT;
    versor rotation = GLM_QUAT_IDENTITY_INIT;
    mat4 transform = GLM_MAT4_IDENTITY_INIT;
    float scale = 1;

    KeyframePos key_position;
    KeyframeRot key_rotation;
    KeyframeFloat key_scale;
    KeyframeFloat key_texture_mix;

    void update(float t);
};

struct ModelContainer {
    Mesh mesh;
    std::shared_ptr<VAO> vao;
    std::shared_ptr<TextureArray> image_array;
    std::vector<std::string> image_names;

    bool image_changed = false;

    ModelContainer(){
        vao = std::shared_ptr<VAO>(new VAO());
        image_array = std::shared_ptr<TextureArray>(new TextureArray());

        // Load a default model (a simple square, good for images)
        float pos_vals[] = {-.5,-.5, .5,-.5, .5,.5, -.5,.5};
        float uv_vals[] = {0,1, 1,1, 1,0, 0,0};
        uint32_t index[] = {0,1,2, 0,2,3};
        vao->load_attrb_float(ATTRB_POS, 0, 0, 2, 8, pos_vals );
        vao->load_attrb_float(ATTRB_UV, 0, 0, 2, 8, uv_vals );
        vao->load_index(6, index);
    }
};


struct ShaderContainer {
    std::shared_ptr<Shader> shader;
    std::string filename;
    bool needs_compiled = false;
    bool blend = false;
    bool cull = true;
    bool depth_test = true;

    ShaderContainer(){
        shader = std::shared_ptr<Shader>(new Shader());
    }
};

bool compare_objects(uint32_t a, uint32_t b);

struct Scene {
    // Use a sorted set, objects are sorted by shader and model
    std::set<uint32_t,  bool(*)(uint32_t,uint32_t)> objects = std::set<uint32_t,  bool(*)(uint32_t,uint32_t)>(compare_objects);

    void add_object( uint32_t id );
    void remove_object( uint32_t id);
    void update( float t );
    void draw();
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
    extern Scene *active_scene;


    ShaderContainer *get_shader( const std::string& name );
    ShaderContainer *get_shader( uint32_t id );
    ModelContainer *get_model( const std::string& name );
    ModelContainer *get_model( uint32_t id );
    ObjectInstance *get_object( const std::string& name );
    ShaderContainer *null_shader();
    ModelContainer *null_model();
    ObjectInstance *null_object();
    ArmatureInfo *get_armature_info( const std::string& name );

    void scene_create(const std::string& name);
    void scene_delete(const std::string& name);
    bool scene_select(const std::string& name);
    bool scene_add(const std::string& object);
    bool scene_remove(const std::string& object);

    ShaderContainer* create_shader(const std::string& name);
    ModelContainer* create_model(const std::string& name);
    ObjectInstance* create_object(const std::string& name);
    ArmatureInfo* load_armature(const std::string& name, const std::string &filename);


    bool unparent(const std::string &object);
    bool parent_object(const std::string& child, const std::string& parent);
    bool parent_joint(const std::string& child, const std::string& parent, const std::string& joint);

    void init();
    void close();
    void update(float t);
    void draw();
}

#endif // VNASSETMANAGER_H
