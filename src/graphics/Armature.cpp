#include "Armature.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstring>

/*
 * Anim Channel
 */

void AnimChannel::value_pos(float t, vec3 v){
    // Set the default return key out of range
    uint16_t k = positions.size();

    // Quick check out of range
    if(t > positions.back().t){
        glm_vec3_copy(positions.back().pos, v);
        return;
    }

    // Find the first key that exceeds the time sought
    for(uint16_t i = 0; i < positions.size(); ++i){
        if(positions[i].t > t){
            k = i;
            break;
        }
    }

    // If none found, assume out of range (this should have been caught by the first check)
    if(k == positions.size()){
        glm_vec3_copy(positions.back().pos, v);
        return;
    }

    // Pre-Animation times are considered invalid and thus do nothing
    else if( k > 0){
        // The time t must be greater than the time at k-1 and less than k, lerp between the two
        glm_vec3_lerp(positions[k-1].pos, positions[k].pos, (t-positions[k-1].t) / (positions[k].t - positions[k-1].t), v );
    }
}

void AnimChannel::value_rot(float t, versor v){
    // Set the default return key out of range
    uint16_t k = rotations.size();

    // Quick check out of range
    if(t > rotations.back().t){
        glm_quat_copy(rotations.back().rot, v);
        return;
    }

    // Find the first key that exceeds the time sought
    for(uint16_t i = 0; i < rotations.size(); ++i){
        if(rotations[i].t > t){
            k = i;
            break;
        }
    }

    // If none found, assume out of range (this should have been caught by the first check)
    if(k == rotations.size()){
        glm_quat_copy(rotations.back().rot, v);
        return;
    }

    // Pre-Animation times are considered invalid and thus do nothing
    else if( k > 0){
        // The time t must be greater than the time at k-1 and less than k, lerp between the two
        glm_quat_nlerp(rotations[k-1].rot, rotations[k].rot, (t-rotations[k-1].t) / (rotations[k].t - rotations[k-1].t), v );
    }
}


/*
 * Animation
 */

void Animation::pose_set(Armature &a, float time){
    for(AnimChannel &c : channels){
        c.value_pos(time, a.joints[c.joint].pos);
        c.value_rot(time, a.joints[c.joint].rot);
    }
}

void Animation::pose_add(Armature &a, float time, float mix_value){
    vec3 p;
    versor r;
    vec3 axis;
    float angle;
    for(AnimChannel &c : channels){
        c.value_pos(time, p);
        c.value_rot(time, r);
        glm_vec3_muladds(p, mix_value, a.joints[c.joint].pos);

        // Extract the axis and angle from the rotations
        angle = glm_quat_angle(r);
        glm_quat_axis(r,axis);

        // Scale the angle component only then apply on top of pose
        angle *= mix_value;
        glm_quatv(r, angle, axis);
        glm_quat_mul(a.joints[c.joint].rot, r, a.joints[c.joint].rot);
    }
}

void Animation::pose_mix( Armature &a, float time, float mix_value ) {
    vec3 p;
    versor r;

    for( AnimChannel &c : channels ) {
        c.value_pos( time, p );
        c.value_rot( time, r );
        glm_vec3_lerp( a.joints[c.joint].pos, p, mix_value,  a.joints[c.joint].pos);
        glm_quat_nlerp( a.joints[c.joint].rot, r, mix_value,  a.joints[c.joint].rot);
    }
}

/*
 * Armature
 */

void Armature::assign_rest(){
    if(joints.empty())
        return;

    // Clear the inverse rest of the root
    glm_mat4_identity(joints[0].tr_inverse_rest);

    for(uint8_t i = 1; i < joints.size(); ++i){
        // Copy the inverse rest of the parent to the child
        glm_mat4_copy(joints[joints[i].parent].tr_inverse_rest, joints[i].tr_inverse_rest);

        // Translate and rotate on top of the inverse rest
        glm_translate(joints[i].tr_inverse_rest, joints[i].pos);
        glm_quat_rotate(joints[i].tr_inverse_rest, joints[i].rot, joints[i].tr_inverse_rest);
    }

    // Inverse the rest transform now that they have all updated
    for(uint8_t i = 1; i < joints.size(); ++i){
        glm_inv_tr(joints[i].tr_inverse_rest);
    }
}

void Armature::update(float t){

    // Update all animations
    update_animations(t);

    // Root is ignored, root is set elsewhere
    for(uint8_t i = 1; i < joints.size(); ++i){
        Joint &j = joints[i];

        // Update the local transform
        glm_translate_make(j.tr, j.pos);
        glm_quat_rotate(j.tr, j.rot, j.tr);

        // Apply on top of parent (parents always have a lower index and have already updated)
        glm_mul(joints[j.parent].tr, j.tr, j.tr);

        // Apply constraints (if present)
        if(j.constraint_id != 0){
            constraints[j.constraint_id]->update(j);
            constraints[j.constraint_id]->apply(j);
        }
    }

    // Place into the transform buffer relative to the inverse transform (ignore root, it is not needed)
    for(uint8_t i = 1; i < joints.size(); ++i){
        glm_mat4_mul(joints[i].tr, joints[i].tr_inverse_rest, ((mat4*)transform_buffer.get())[i] );
    }
}


void Armature::assign_info(ArmatureInfo *info){
    info->get_aramture().copy(*this);
}

void Armature::copy(Armature &dest){
    if(empty())
        return;

    // Reallocate transform buffer and copy data
    transform_buffer = std::unique_ptr<mat4>(new mat4[joints.size()]);
    memcpy(dest.transform_buffer.get(), transform_buffer.get(), joints.size()*sizeof(mat4));

    // Delete all constraints
    dest.constraints.clear();

    // Re-add constraints
    for(const auto& c: constraints)
        dest.constraints.push_back(c->get_copy());

    // Copy joints and playing animations
    dest.joints = joints;
    dest.play_data = play_data;
}

void Armature::play_animation(uint8_t anim, float speed, float start, float stop, uint8_t end_method, uint8_t blend_method,  float blend_factor, uint8_t overwrite_method, bool add_bottom){
    if(empty())
        return;

    uint8_t play_id;
    if(is_playing(anim, play_id)){

        // Return if overwrite method is wait
        if(overwrite_method == PlayData::WAIT)
            return;

        // do not change the time, but update the play data, this is for update and overwrite
        play_data[play_id].speed = speed;
        play_data[play_id].end_method = end_method;
        play_data[play_id].blend_method = blend_method;
        play_data[play_id].blend_factor = blend_factor;

        play_data[play_id].start = fmax(0, start);
        play_data[play_id].stop = fmin(play_data[play_id].stop, info->get_animation(anim).duration);
        play_data[play_id].start = fmin(play_data[play_id].stop, play_data[play_id].start);

        // Update time if overwrite
        if(overwrite_method == PlayData::OVERWRITE){
            play_data[play_id].time = start;
        }
    }

    // Create a new animation
    else{
        PlayData pd;
        pd.animation_id = anim;
        pd.speed = speed;
        pd.end_method = end_method;
        pd.blend_method = blend_method;
        pd.time = start;
        pd.blend_factor = blend_factor;

        pd.start = fmax(0, start);
        pd.stop = fmin(pd.stop, info->get_animation(anim).duration);
        pd.start = fmin(pd.stop, pd.start);

        if(add_bottom)
            play_data.insert(play_data.begin(),pd);
        else
            play_data.push_back(pd);
    }
}

bool Armature::is_playing(uint8_t anim, uint8_t &play_id){
    for(uint8_t i = 0; i < play_data.size(); ++i){
        if( play_data[i].animation_id == anim){
            play_id = i;
            return true;
        }
    }
    return false;
}

void Armature::pose(uint8_t anim, float time, float mix_factor){
    if(empty())
        return;
    info->get_animation(anim).pose_mix(*this, time, mix_factor);
}

void Armature::mix( uint8_t anim1, float t1, uint8_t anim2, float t2, float factor){
    if(empty())
        return;
    info->get_animation(anim1).pose_set(*this, t1);
    info->get_animation(anim2).pose_mix(*this, t2, factor);
}

void Armature::reset_pose(){
    for(Joint &j : joints){
        glm_vec3_zero(j.pos);
        glm_quat_identity(j.rot);
    }
    update(0);
}

void Armature::update_animations(float t){
    if(empty())
        return;

    // Update the animation times
    for(uint8_t i = 0; i < play_data.size(); ++i){
        PlayData &pd = play_data[i];
        Animation &anim = info->get_animation(pd.animation_id);


        // Update animation time
        pd.time += t*pd.speed;
        if(pd.time > pd.stop || pd.time < pd.start){

            switch(pd.end_method){

                case PlayData::END:
                    play_data.erase(play_data.begin()+i);
                    --i;
                    continue;

                case PlayData::BACK_LOOPED:
                    pd.speed *= -1;
                case PlayData::LOOP:
                    pd.time = fmod(pd.time-pd.start, pd.stop-pd.start) + pd.start;
                    break;

                case PlayData::BACK:
                    pd.speed *= -1;
                case PlayData::CLAMPED:
                    pd.time = glm_clamp(pd.time, pd.start, pd.stop);
                    break;

            }
        }

        // Apply the animation by posing the armature
        switch(pd.blend_method){
            case PlayData::SET:
                anim.pose_set(*this, pd.time);
                break;
            case PlayData::MIX:
                anim.pose_mix(*this, pd.time, pd.blend_factor);
                break;
            case PlayData::ADD:
                anim.pose_add(*this, pd.time, pd.blend_factor);
                break;
        }
    }
}

void Armature::constraint_softbody(uint8_t joint_id, SoftbodySettings settings){
    if(empty())
        return;
    Joint &j = joints[joint_id];

    // Update the settings of an already existing constraint
    if(j.constraint_id != 0 && constraints[j.constraint_id]->type == Constraint::SOFTBODY){
        ((ConstraintSoftbody*)constraints[j.constraint_id].get())->settings = settings;
        return;
    }
    else if(j.constraint_id != 0){
        printf("Failed to add softbody: joint %s already has a constraint of another type\n", j.name.c_str());
        fflush(stdout);
        return;
    }
    // Joint does not already have a constraint
    else{
        ConstraintSoftbody sb;
        sb.joint = joint_id;
        sb.settings = settings;

        // Use this to mark if a constrained child exists
        uint8_t child_constraint = 0,
        child_id = 0;

        // Forbid making a parent joint if more than one softbody chain would be made
        for(uint8_t c : j.children){
            Joint &child = joints[c];
            std::unique_ptr<Constraint> &cc = constraints[child.constraint_id];

            // If there is a child that isnt root and the child has a softbody constraint
            if(c != 0 && cc->type == Constraint::SOFTBODY){

                // A child was already set, do not add a constraint
                if(child_constraint){
                    printf("Failed to add softbody: joint %s has more than one constrained child\n",j.name.c_str());
                    fflush(stdout);
                    return;
                }

                // Place the child constraint id in to the child_constraint slot
                child_constraint = child.constraint_id;
                child_id = c;
            }
        }

        // Forbid placing a softbody on a parent that already has a chained softbody
        Joint &parent = joints[j.parent];
        std::unique_ptr<Constraint> &pc = constraints[parent.constraint_id];

        if(pc->type == Constraint::SOFTBODY){

            // Add as the softbody parent
            sb.parent_softbody = ((ConstraintSoftbody*)pc.get());

            // Sibling has a softbody
            if(((ConstraintSoftbody*)pc.get())->child_joint != 0){
                printf("Failed to add softbody: joint %s has a sibling with a softbody constraint\n",j.name.c_str());
                fflush(stdout);
                return;
            }
            // Parent has a softbody, but no siblings do
            else{

                // Create relationship
                ((ConstraintSoftbody*)pc.get())->child_joint = joint_id;
                sb.parent_joint = j.parent;

                // Find the rest positions of the joints to use as joint length
                mat4 tr;
                vec3 head, tail;
                glm_mat4_inv(parent.tr_inverse_rest, tr);
                glm_mat4_mulv3(tr, head, 1, head);
                glm_mat4_inv(j.tr_inverse_rest, tr);
                glm_mat4_mulv3(tr, tail, 1 , tail);

                ((ConstraintSoftbody*)pc.get())->settings.joint_length = glm_vec3_distance(head, tail);
            }
        }

        // Parent does not have a softbody constraint (parent == root)
        // All fail cases have been checked

        if(child_constraint){

            // Update the child constraint's relation (note that the child needs the parent constraint linked once sb is added to the constraint list)
            ((ConstraintSoftbody*)constraints[child_constraint].get())->parent_joint = joint_id;
            sb.child_joint = child_id;

            // Use this child to determine joint length
            mat4 tr;
            vec3 head, tail;
            glm_mat4_inv(j.tr_inverse_rest, tr);
            glm_mat4_mulv3(tr, head, 1, head);
            glm_mat4_inv(joints[child_id].tr_inverse_rest, tr);
            glm_mat4_mulv3(tr, tail, 1 , tail);
            sb.settings.joint_length = glm_vec3_distance(head, tail);
        }

        // If there is a single child present, use its position for joint length
        else if(j.children.size() == 1){
            mat4 tr;
            vec3 head, tail;
            glm_mat4_inv(j.tr_inverse_rest, tr);
            glm_mat4_mulv3(tr, head, 1, head);
            glm_mat4_inv(joints[j.children[0]].tr_inverse_rest, tr);
            glm_mat4_mulv3(tr, tail, 1 , tail);
            sb.settings.joint_length = glm_vec3_distance(head, tail);
        }

        // Place into the constraint list
        constraints.push_back(std::make_unique<ConstraintSoftbody>(sb));

        // Link the child to the new constraint (if present)
        if(child_constraint){
            ((ConstraintSoftbody*)constraints[child_constraint].get())->parent_softbody = (ConstraintSoftbody*)constraints.back().get();
        }
    }
}

/*
 * Armature Info
 */

bool ArmatureInfo::load( const std::string &filename ) {
    std::string filepath = ( std::string )DIR_ARMATURES + filename + ".arm";
    std::ifstream filereader;
    filereader.open( filepath, std::ios::out | std::ios::binary );

    if( !filereader ) {
        std::cout << "Unable to read armature file: " << filepath << std::endl;
        return false;
    }

    // Clear old data
    armature = Armature();
    armature.info = this;
    joint_names.clear();
    animation_names.clear();
    animations.clear();

    // Read number of joints
    uint8_t joint_count, name_length, children_count;
    filereader.read( reinterpret_cast<char *>( &joint_count ), 1 );

    if( joint_count > ARMATURE_MAX_JOINTS ) {
        std::cout << "Warning, number of joints " << ( int )joint_count << " exceeds maximum " << ARMATURE_MAX_JOINTS << std::endl;
    }

    char buffer[256];

    // Joints
    for( uint8_t i = 0; i < joint_count; ++i ) {
        Joint joint;

        // Name
        filereader.read( reinterpret_cast<char *>( &name_length ), 1 );
        filereader.read( buffer, name_length );
        joint.name.assign( buffer, name_length );

        // Parent
        filereader.read( reinterpret_cast<char *>( &joint.parent ), 1 );

        // Children
        filereader.read( reinterpret_cast<char *>( &children_count ), 1 );
        filereader.read( buffer, children_count );
        joint.children.assign( buffer, buffer + children_count );

        // Position and Rotation
        filereader.read( reinterpret_cast<char *>( joint.pos ), 12 );
        filereader.read( reinterpret_cast<char *>( joint.rot ), 16 );

        // Add to armature
        if( armature.joints.size() < ARMATURE_MAX_JOINTS ) {
            armature.joints.push_back( joint );
            joint_names[joint.name] = armature.joints.size() - 1;
        }
    }

    armature.transform_buffer = std::unique_ptr<mat4>(new mat4[armature.joints.size()]);

    // Animations
    uint8_t animation_count, channel_count;
    uint32_t poskey_count, rotkey_count;
    filereader.read( reinterpret_cast<char *>( &animation_count ), 1 );
    printf( "animcount:%d\n", animation_count );

    for( uint8_t i = 0; i < animation_count; ++i ) {
        Animation animation;

        // Name
        filereader.read( reinterpret_cast<char *>( &name_length ), 1 );
        filereader.read( buffer, name_length );
        animation.name.assign( buffer, buffer + name_length );
        std::cout << animation.name << std::endl;

        // Duration
        filereader.read( reinterpret_cast<char *>( &animation.duration ), 4 );

        // Channels
        filereader.read( reinterpret_cast<char *>( &channel_count ), 1 );

        for( uint8_t j = 0; j < channel_count; ++j ) {
            AnimChannel channel;

            // Joint
            filereader.read( reinterpret_cast<char *>( &channel.joint ), 1 );

            // Key counts
            filereader.read( reinterpret_cast<char *>( &poskey_count ), 4 );
            filereader.read( reinterpret_cast<char *>( &rotkey_count ), 4 );

            // Position
            channel.positions.resize( poskey_count );

            for( unsigned int k = 0; k < poskey_count; ++k ) {
                filereader.read( reinterpret_cast<char *>( &channel.positions[k].t ), 4 );
                filereader.read( reinterpret_cast<char *>( &channel.positions[k].pos ), 12 );
            }

            // Rotation
            channel.rotations.resize( rotkey_count );

            for( unsigned int k = 0; k < rotkey_count; ++k ) {
                filereader.read( reinterpret_cast<char *>( &channel.rotations[k].t ), 4 );
                filereader.read( reinterpret_cast<char *>( &channel.rotations[k].rot ), 16 );
            }

            animation.channels.push_back( channel );
        }

        animation_names[animation.name] = animations.size();
        animations.push_back( animation );
    }

    filereader.close();

    // Construct armature rest transforms
    armature.assign_rest();

    return true;
}
