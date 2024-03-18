#ifndef ARMATURE_H
#define ARMATURE_H

#include "definitions.h"
#include <cglm/vec3.h>
#include <cglm/quat.h>
#include <cglm/mat4.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "View.h"
#include "ArmatureConstraints.h"

class Armature;
class ArmatureInfo;
struct Animation;

struct KeyPos {
    vec3 pos = GLM_VEC3_ZERO_INIT;
    float t = 0;
};

struct KeyRot {
    versor rot =  GLM_QUAT_IDENTITY_INIT;
    float t = 0;
};

struct AnimChannel {
    uint8_t joint;
    std::vector<KeyPos> positions;
    std::vector<KeyRot> rotations;

    // Get the value at a given time t in the channel
    void value_pos(float t, vec3 v);
    void value_rot(float t, versor v);
};

struct Animation {
    std::string name;
    float duration = 0;
    std::vector<AnimChannel> channels;

    // Set pose to match that at time t
    void pose_set(Armature &a, float time);

    // Add scaled pos at time t on to the current pose
    void pose_add(Armature &a, float time, float mix_value);

    // Mix the current pose with the pose at time t
    void pose_mix(Armature &a, float time, float mix_value);
};

struct PlayData {
    /*
     * End Methods:
     * END - stop playing the animation
     * LOOP - repeats the animation until stopped
     * BACK - does a single back before ending
     * BACK_LOOPED - loops the animation in reverse when reaching the end
     * CLAMPED - keeps applying the last pose of the animation
     */
    static const uint8_t END = 0, LOOP = 1, BACK = 2, BACK_LOOPED = 3, CLAMPED = 4;

    /*
     * Blend Methods:
     * SET - Sets the keyframvalue, overwriting the animations below it
     * ADD - Adds the animation on top of other animations below it
     * MIX - Mixes the animation with those below it by its mix value
     */
    static const uint8_t SET = 0, ADD = 1, MIX = 2;

    /*
     * Overwrite Behaviors:
     * WAIT - Does not touch the animation of the same ID until there is no matching animation
     * UPDATE - Updates the values of the animation, but does not change the play time
     * OVERWRITE - Overwrites the animation, essentially resetting it
     */
    static const uint8_t WAIT = 0, UPDATE = 1, OVERWRITE = 2;

    float blend_factor = .5;
    float speed = 1;
    float time = 0;
    float start = 0;
    float stop = 0;
    uint8_t animation_id = 0;
    uint8_t end_method = END;
    uint8_t blend_method = SET;
};



struct Joint{

    std::string name;
    std::vector<uint8_t> children;
    uint8_t parent;
    uint8_t constraint_id = 0;
    uint8_t animation_applications = 0;

    mat4 tr_inverse_rest = GLM_MAT4_IDENTITY_INIT;
    mat4 tr = GLM_MAT4_IDENTITY_INIT;

    vec3 pos = GLM_VEC3_ZERO_INIT;
    versor rot = GLM_QUAT_IDENTITY_INIT;

};


class Armature {
    friend ArmatureInfo;
    friend Animation;

    ArmatureInfo *info = nullptr;
    std::vector<std::unique_ptr<Constraint>> constraints;

    // Applies all playing animations
    void update_animations(float t);

public:

    // This is used for constraints, this does NOT change the actual scale
    float scale_constraint = 1;

    Armature(){
        // Add the null constraint since the default constraint id is 0
        constraints.push_back(std::unique_ptr<Constraint>(new Constraint()));
    }

    std::vector<Joint> joints;
    std::unique_ptr<mat4> transform_buffer = nullptr;
    std::vector<PlayData> play_data;

    // Assigns the current transform for all joints as the new rest transform
    void assign_rest();

    // Applies all animations, constraints, and updates the transform buffer
    void update(float t);

    // Assigns an armature to an armature info
    void assign_info(ArmatureInfo *info);

    // Copies an armature to another
    void copy(Armature &dest);

    // Adds a new playing animation
    void play_animation(uint8_t anim,float speed, float start, float stop, uint8_t end_method, uint8_t blend_method, float blend_factor, uint8_t overwrite_method, bool add_bottom = false);

    // Returns whether or not the animation is being played
    bool is_playing(uint8_t anim, uint8_t &play_id);

    // Poses the armature to a specific animation time, can be mixed with the current pose
    void pose(uint8_t anim, float time, float mix_factor);

    // Poses using a mixture of 2 posed animations
    void mix( uint8_t anim1, float t1, uint8_t anim2, float t2, float factor);

    // Resets the pose to 0, displays the rest pose
    void reset_pose();

    // Add a softbody constraint (can't be removed)
    void constraint_softbody(uint8_t joint_id, SoftbodySettings settings);
    void constraint_track(uint8_t joint_id, vec3 focus, uint8_t axis, bool negate);

    // Must be initialized, otherwise will crash
    inline Joint& get_root(){return joints[0];}
    inline Joint& get_joint(uint8_t joint_id){return joints[joint_id];}
    inline ArmatureInfo* get_info() const{ return info; }
    inline bool empty(){return !transform_buffer || !info || joints.empty();}
    inline Constraint* get_constraint(uint8_t id){ if( id == 0 || id >= constraints.size())return nullptr;return constraints[id].get();}


};

class ArmatureInfo {
    Armature armature;

    // Place an empty animation at the start of the list for null
    std::vector<Animation> animations =  {Animation()};

    std::unordered_map<std::string, uint8_t> joint_names;
    std::unordered_map<std::string, uint8_t> animation_names;

public:

    inline Armature &get_aramture(){
        return armature;
    }

    inline Animation &get_animation(uint8_t id){
        return animations[id];
    }

    inline uint8_t get_animation_id(const std::string& name) const{
        try{
            return animation_names.at(name);
        }
        catch(std::out_of_range &oor){
            return 0;
        }
    }

    inline uint8_t get_joint_id(const std::string& name) const{
        try{
            return joint_names.at(name);
        }
        catch(std::out_of_range &oor){
            return 0;
        }
    }

    bool load(const std::string& filename);


};


#endif /* ARMATURE_H */

