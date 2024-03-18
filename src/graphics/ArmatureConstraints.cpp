#include "ArmatureConstraints.h"
#include "Armature.h"


void ConstraintSoftbody::update(Joint &j, Armature& arm){

    // Declare the head position of this constraint
    vec3 head;

    // If the contraint has a parent, use parent tail as head
    if( parent_softbody )
        glm_vec3_copy( parent_softbody->tail, head);

    // Use joint transform otherwise
    else
        glm_vec3_copy(j.tr[3], head);


    // Set the absolute velocity to the difference in tail position
    vec3 abs_vel;
    glm_vec3_sub(tail, last_tail, abs_vel);

    // Localize the absolute velocity by removing the head component
    vec3 local_vel = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub(abs_vel, head, local_vel);

    // Add the previous animation position (acts as animated velocity)
    glm_vec3_add(local_vel, last_anim_pos, local_vel);

    // Save the head as the last animated position for the next frame
    glm_vec3_copy(head, last_anim_pos);

    // Remove the local component from the absolute velocity
    glm_vec3_sub(abs_vel, local_vel, abs_vel);

    // Save the tail before updating it
    glm_vec3_copy(tail, last_tail);

    /*
     * Update the tail position.
     * This approximation applies air drag to the absolute velocity
     * friction to the local velocity
     * and gravity acceleration
     */

    tail[0] += ((1-settings.drag)) * abs_vel[0] + (1-settings.friction) * local_vel[0];
    tail[1] += ((1-settings.drag)) * abs_vel[1] + (1-settings.friction) * local_vel[1] - settings.gravity * timestep_sqr;
    tail[2] += ((1-settings.drag)) * abs_vel[2] + (1-settings.friction) * local_vel[2];

    // Length elasticity

    // Extract tail in local space
    vec3 ideal_tail;
    glm_vec3_sub(tail, head, ideal_tail);

    // Scale the tail to match that of the joint length
    glm_vec3_scale(ideal_tail, arm.scale_constraint*settings.joint_length / (glm_vec3_norm(ideal_tail) + GLM_FLT_EPSILON), ideal_tail);

    // Place tail back in global space
    glm_vec3_add(ideal_tail, head, ideal_tail);

    // Lerp correction using elasticity as factor
    glm_vec3_sub(ideal_tail, tail, ideal_tail);
    glm_vec3_scale(ideal_tail, settings.elasticity, ideal_tail);
    glm_vec3_add(tail, ideal_tail, tail);

    // Restoration using rigidity

    // Reuse ideal tail position, extract as the +y-basis column of the transform
    // Rigidity will attempt to return to the original rotation
    glm_vec3_scale(j.tr[1], arm.scale_constraint*settings.joint_length, ideal_tail);
    glm_vec3_add(ideal_tail, j.tr[3], ideal_tail);

    // Lerp correction using rigidity as factor
    glm_vec3_sub(ideal_tail, tail, ideal_tail);
    glm_vec3_scale(ideal_tail, settings.rigidity, ideal_tail);
    glm_vec3_add(tail, ideal_tail, tail);
}

void ConstraintSoftbody::apply(Joint &j, Armature& arm){

    // Align each joint to its tail

    versor required_rot;

    vec3 head;
    // If there is a parent, use its tail
    if(parent_softbody){
        glm_vec3_copy(parent_softbody->tail, head);
    }
    // Otherwise use the joint transform
    else{
        glm_vec3_copy(j.tr[3], head);
    }

    // Find the required world_space rotation from the current joint

    // Find vector to tail
    vec3 to_tail;
    glm_vec3_sub(tail, head, to_tail);

    // Normalize to_tail (forward should already be normalized)
    glm_vec3_normalize(to_tail);
    glm_quat_from_vecs(j.tr[1], to_tail, required_rot);

    mat4 m;
    glm_quat_mat4(required_rot, m);
    glm_vec3_zero(j.tr[3]);
    glm_mat4_mul(m, j.tr, j.tr);
    glm_vec3_copy(head, j.tr[3]);
}

std::unique_ptr<Constraint> ConstraintSoftbody::get_copy(){
    return std::make_unique<ConstraintSoftbody>(*this);
}

void ConstraintTrack::update(Joint &j, Armature& arm){
    // This is the axis that the constraint pivots around
    vec3 up;
    up[0] = fx%3 == 1;
    up[1] = fx%3 == 2;
    up[2] = fx%3 == 0;

    versor r;
    vec3 pos;
    glm_mat4_mulv3(j.tr_inverse_rest, up, 0, up);

    // Deconstructed forp, uses +x forward instead of +z forward, fx changes the axis
    vec3 dir;
    glm_vec3_sub(focus, j.tr[3], dir);
    mat3 m;
    if(neg)
        glm_vec3_negate(dir);
    glm_vec3_normalize_to(dir, m[(fx)%3]);
    glm_vec3_crossn(up, m[(fx)%3], m[(1+fx)%3]);
    glm_vec3_cross(m[(fx)%3], m[(1+fx)%3], m[(2+fx)%3]);
    glm_mat3_quat(m, r);

    glm_vec3_copy(j.tr[3],pos);
    glm_quat_mat4(r, j.tr);
    glm_vec3_copy(pos, j.tr[3]);
}

void ConstraintTrack::apply(Joint &j, Armature& arm){

}

std::unique_ptr<Constraint> ConstraintTrack::get_copy(){
    return std::make_unique<ConstraintTrack>(*this);
}
