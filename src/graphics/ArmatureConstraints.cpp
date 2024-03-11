#include "ArmatureConstraints.h"
#include "Armature.h"


void ConstraintSoftbody::update(Joint &j){

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
    tail[0] += ((1-settings.drag)) * abs_vel[0] + (settings.momentum) * local_vel[0];
    tail[1] += ((1-settings.drag)) * abs_vel[1] + (settings.momentum) * local_vel[1] - settings.gravity * timestep_sqr;
    tail[2] += ((1-settings.drag)) * abs_vel[2] + (settings.momentum) * local_vel[2];

    // Length elasticity

    // Extract tail in local space
    vec3 ideal_tail;
    glm_vec3_sub(tail, head, ideal_tail);

    // Scale the tail to match that of the joint length
    glm_vec3_scale(ideal_tail, settings.joint_length / (glm_vec3_norm(ideal_tail) + GLM_FLT_EPSILON), ideal_tail);

    // Place tail back in global space
    glm_vec3_add(ideal_tail, head, ideal_tail);

    // Lerp correction using elasticity as factor
    glm_vec3_sub(ideal_tail, tail, ideal_tail);
    glm_vec3_scale(ideal_tail, settings.elasticity, ideal_tail);
    glm_vec3_add(tail, ideal_tail, tail);

    // Restoration using rigidity

    // Reuse ideal tail position, extract as the +y-basis column of the transform
    // Rigidity will attempt to return to the original rotation
    glm_vec3_copy(j.tr[1], ideal_tail);

    // Lerp correction using rigidity as factor
    glm_vec3_sub(ideal_tail, tail, ideal_tail);
    glm_vec3_scale(ideal_tail, settings.rigidity, ideal_tail);
    glm_vec3_add(tail, ideal_tail, tail);
}

void ConstraintSoftbody::apply(Joint &j){

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

    // Get the y-basis
    vec3 forward;
    glm_vec3_copy(j.tr[1], forward);

    // Find vector to tail
    vec3 to_tail;
    glm_vec3_sub(tail, head, to_tail);

    // Normalize to_tail (forward should already be normalized)
    glm_vec3_normalize(to_tail);
    glm_quat_from_vecs(forward, to_tail, required_rot);

    // Apply world-space rotation as a pre-transformation
    glm_vec3_zero(j.tr[3]);
    glm_quat_rotate(j.tr, to_tail, j.tr);
    glm_vec3_copy(head, j.tr[3]);
}

std::unique_ptr<Constraint> ConstraintSoftbody::get_copy(){
    return std::make_unique<ConstraintSoftbody>(*this);
}
