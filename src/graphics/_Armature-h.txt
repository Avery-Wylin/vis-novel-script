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

using std::vector;
using std::string;
using std::unordered_map;

static const uint8_t
CONSTRAINT_NULL = 0,
CONSTRAINT_SOFTBODY = 1,    // Use fixed time step verlet physics, chainable
CONSTRAINT_TRACK = 2;       // Track joint to a fixed position in world space using closest rotation

class Armature;

struct Animation {
        struct Channel {
            struct PosKey {
                float t = 0;
                vec3 v = GLM_VEC3_ZERO_INIT;
            };
            struct RotKey {
                float t = 0;
                versor v =  GLM_QUAT_IDENTITY_INIT;
            };

            uint8_t joint;
            vector<PosKey> positions;
            vector<RotKey> rotations;
            void print() const;
        };

    float duration;
    string name;
    vector<Channel> channels;
    void print()const;
    void apply(Armature &a, float time)const;
};

struct AnimationPlayData {
    friend class Armature;
    static const uint8_t TERMINAL = 0, LOOP = 1, BACK = 2, CLAMPED = 3;
    float
    playback_speed = 1.0f,
    current_time = 0.0f;
    const Animation *animation = nullptr;
    uint8_t playback_type = TERMINAL;
};

struct SoftbodyConstraint {
    vec3 tail_pos = GLM_VEC3_ZERO_INIT;
    vec3 last_tail_pos = GLM_VEC3_ZERO_INIT;
    vec3 last_anim_pos = GLM_VEC3_ZERO_INIT;
    float elasticity = 0.7;
    float drag = 0.8;
    float rigidity = 0.3;
    float gravity = 1;
    float joint_length = 0.1;
    float friction = 0.4;
    uint8_t parent_joint = 0;
    uint8_t child_joint = 0;
    uint8_t joint = 0;
};

class ArmatureInfo;

class Armature {
        friend class ArmatureInfo;
        friend struct Animation;

    private:
        ArmatureInfo *info;
        struct Joint {

                vector<uint8_t> children;
                mat4
                inverse_rest = GLM_MAT4_IDENTITY_INIT,
                transform = GLM_MAT4_IDENTITY_INIT;
                vec3 old_world_pos = GLM_VEC3_ZERO_INIT;
                versor old_world_rot = GLM_QUAT_IDENTITY_INIT;
                vec3 local_pos = GLM_VEC3_ZERO_INIT;
                versor local_rot = GLM_QUAT_IDENTITY_INIT;
                uint8_t parent = 0;
                uint8_t constraint_id = 0;
                uint8_t constraint_type = 0;
        };

        vector<Joint> joints;
        vector<AnimationPlayData> playing_animations;
        vector<SoftbodyConstraint> softbody_constraints;
        mat4 *transform_buffer = nullptr;

        void assign_as_rest();
        void apply_animations();
        void update_transform_buffer();

        void update_softbody_positions( SoftbodyConstraint &j);
        void apply_softbody( SoftbodyConstraint &j);

    public:
        Armature(){};
        ~Armature(){
            if(transform_buffer != nullptr){
                delete[] transform_buffer;
                transform_buffer = nullptr;
            }
        };

        float constraint_timestep;

        // Multiple animations can be played at once, however the same animation can't be played
        // The order in which they are played matters since larger orders will override smaller ones
        void assign(ArmatureInfo* info);
        void copy(Armature& dest)const;

        void play_animation( const Animation* anim, uint8_t playback_type, float speed, bool wait = false );
        void stop_animation( const Animation* anim );
        int get_playing_anim_id(const Animation* anim);
        AnimationPlayData* get_playing_anim(const Animation* anim);
        void clear_animations();
        const Animation* get_animation(string name);
        void set_time( float time );
        void update(float t);
        void interpolate(float t);
        void set_root_transform(vec3 pos, versor rot, float scale);

        mat4* get_transform_buffer();

        void add_softbody(string joint_name);
        void add_softbody(string joint_name, float elasticity, float drag, float friction, float rigidity, float gravity, float default_length  = 0.1f);

        void print() const;
        inline const vector<Joint>& getJoints(){return joints;};
        inline bool empty(){return transform_buffer==nullptr;};

};

class ArmatureInfo {
    private:
        Armature armature;
        vector<Animation> animations;
        unordered_map<string, uint8_t> joint_names;
        unordered_map<string, uint32_t> animation_names;
    public:
        inline const Armature &getArmature() {
            return armature;
        };

        ArmatureInfo(){

        }

        ~ArmatureInfo(){

        }

        Animation *get_animation( string &name );
        uint8_t get_anim_id( string &name );
        uint8_t get_joint( string &name );
        bool load( string filename );
};


#endif /* ARMATURE_H */

