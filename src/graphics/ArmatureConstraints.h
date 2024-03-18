#ifndef ARMATURECONSTRAINTS_H
#define ARMATURECONSTRAINTS_H

#include <vector>
#include <inttypes.h>
#include <cglm/vec3.h>
#include <memory>

struct Joint;
class Armature;


struct Constraint{
    static const uint8_t
    NONE = 0,
    SOFTBODY = 1,
    TRACK = 2;

    uint8_t joint = 0;
    uint8_t type = NONE;

    Constraint(){}
    virtual ~Constraint(){}

    virtual void update(Joint &j, Armature& arm){}
    virtual void apply(Joint &j, Armature& arm){}
    inline virtual std::unique_ptr<Constraint> get_copy(){return std::unique_ptr<Constraint>(new Constraint());}
};

const static float timestep = 0.02f;
const static float timestep_sqr = timestep * timestep;

struct SoftbodySettings{
    float elasticity = 0.7;
    float drag = 0.8;
    float friction = 0.4;
    float rigidity = 0.3;
    float gravity = 1;
    float joint_length = 0.1;
};

struct ConstraintSoftbody : public Constraint{

    ConstraintSoftbody(){type = SOFTBODY;}
    virtual ~ConstraintSoftbody(){}

    vec3 tail = GLM_VEC3_ZERO_INIT;
    vec3 last_tail = GLM_VEC3_ZERO_INIT;
    vec3 last_anim_pos = GLM_VEC3_ZERO_INIT;

    SoftbodySettings settings;

    ConstraintSoftbody *parent_softbody = nullptr;
    uint8_t parent_joint = 0;
    uint8_t child_joint = 0;

    virtual void update(Joint &j, Armature& arm) override;
    virtual void apply(Joint &j, Armature& arm) override;
    virtual std::unique_ptr<Constraint> get_copy() override;
};

struct ConstraintTrack : public Constraint{
    ConstraintTrack(){type = TRACK;}
    virtual ~ConstraintTrack(){}

    vec3 focus = GLM_VEC3_ZERO_INIT;
    uint8_t fx = 0;
    bool neg = false;

    virtual void update(Joint &j, Armature& arm) override;
    virtual void apply(Joint &j, Armature& arm) override;
    virtual std::unique_ptr<Constraint> get_copy() override;
};

#endif // ARMATURECONSTRAINTS_H
