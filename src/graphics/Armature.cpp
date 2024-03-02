#include "Armature.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstring>

float bouyancy_height = 0;

void Armature::assign( ArmatureInfo *info ) {
    info->getArmature().copy( *this );
}

void Armature::copy( Armature &dest ) const {
    if( dest.info == info )
        return;
    dest.info = info;
    if( dest.transform_buffer != nullptr ) {
        delete[] dest.transform_buffer;
    }
    dest.transform_buffer = new mat4[joints.size()];
    memcpy( dest.transform_buffer, transform_buffer, joints.size()*sizeof( mat4 ) );
    dest.joints = joints;
}

void Armature::assign_as_rest() {
    // Convert all local transforms into rest transforms
    if( joints.empty() )
        return;

    // Find object-space transform of each joint
    // parent's do not need checked because of how the hierarchy is loaded
    glm_mat4_identity( joints[0].inverse_rest );
    for( uint8_t i = 1; i < joints.size(); ++i ) {
        glm_mat4_copy( joints[joints[i].parent].inverse_rest, joints[i].inverse_rest );
        glm_translate( joints[i].inverse_rest, joints[i].local_pos );
        glm_quat_rotate( joints[i].inverse_rest, joints[i].local_rot, joints[i].inverse_rest );
    }

    // Invert the rest transforms
    for( uint8_t i = 1; i < joints.size(); ++i ) {
        glm_inv_tr( joints[i].inverse_rest );
    }

}

void Armature::update_transform_buffer() {

    // For each joint that updated, mark children as updated and calculate new buffer
    for( uint8_t i = 1; i < joints.size(); ++i ) {

        // Update local transform
        glm_vec3_copy(joints[i].transform[3], joints[i].old_world_pos);
        glm_mat4_quat(joints[i].transform, joints[i].old_world_rot);
        glm_mat4_identity( joints[i].transform );
        glm_translate( joints[i].transform, joints[i].local_pos );
        glm_quat_rotate( joints[i].transform, joints[i].local_rot, joints[i].transform );

        // Append on top of parent joint
        glm_mul(
            joints[joints[i].parent].transform,
            joints[i].transform,
            joints[i].transform
        );

        // Softbody constraint
        if( joints[i].constraint_type == CONSTRAINT_SOFTBODY ) {
            update_softbody_positions( softbody_constraints[joints[i].constraint_id] );
            apply_softbody( softbody_constraints[joints[i].constraint_id] );
        }

    }

    // Transform to local space and place into transform buffer
    for( int i = 1; i < joints.size(); ++i ) {
        glm_mat4_mul( joints[i].transform, joints[i].inverse_rest, transform_buffer[i] );
    }
}

void Armature::play_animation( const Animation *anim, uint8_t playback_type, float speed, bool wait ) {
    // Animation not found
    if( anim == nullptr )
        return;
    AnimationPlayData *playanim = get_playing_anim( anim );
    // Animation is already playing, replace it
    if( playanim ) {
        // Wait for the animation to finish, do not restart
        if(wait)
            return;
        playanim->current_time = 0;
        playanim->playback_type = playback_type;
        playanim->playback_speed = speed;
        return;
    }
    // Create a new playing animation
    playing_animations.push_back( AnimationPlayData() );
    playing_animations.back().animation = anim;
    playing_animations.back().playback_type = playback_type;
    playing_animations.back().playback_speed = speed;
}

void Armature::stop_animation( const Animation *anim ) {
    int id = get_playing_anim_id( anim );
    if( id < 0 )
        return;
    playing_animations.erase( playing_animations.begin() + id );
}

int Armature::get_playing_anim_id( const Animation *anim ) {
    for( int i = 0; i < playing_animations.size(); ++i ) {
        if( playing_animations[i].animation == anim )
            return i;
    }
    return -1;
}

AnimationPlayData *Armature::get_playing_anim( const Animation *anim ) {
    for( AnimationPlayData &p : playing_animations ) {
        if( p.animation == anim )
            return &p;
    }
    return nullptr;
}

void Armature::clear_animations() {
    playing_animations.clear();
}

void Armature::set_time( float time ) {
    for( AnimationPlayData &p : playing_animations ) {
        p.current_time = time * p.playback_speed;
    }
}

void Armature::update(float t) {
    for( AnimationPlayData &p : playing_animations ) {
        p.current_time += t * p.playback_speed;
    }

    // WARNING small numbers will mess this up, try to avoid using high FPS as update time, use interpolation instead
    // For this particular projects, a high FPS is not waranted, nor is there any reason to ever change the FPS
    // This number is ideally around .05
    constraint_timestep = t;
    apply_animations();
    update_transform_buffer();
}


void Armature::interpolate(float t){
    vec3 pos;
    versor rot;
    mat4 tr;
    for( int i = 0; i < joints.size(); ++i ) {
       glm_mat4_quat(joints[i].transform, rot);
       glm_vec3_lerp(joints[i].old_world_pos, joints[i].transform[3], t, pos);
       glm_quat_nlerp(joints[i].old_world_rot, rot, t, rot);
       glm_translate_make(tr, pos);
       glm_quat_rotate(tr, rot, tr);
       glm_mat4_mul(tr, joints[i].inverse_rest, transform_buffer[i]);
    }
};

void Armature::set_root_transform(vec3 pos, versor rot, float scale){
    glm_vec3_copy(joints[0].local_pos, joints[0].old_world_pos);
    glm_quat_copy(joints[0].local_rot, joints[0].old_world_rot);
    glm_vec3_copy(pos, joints[0].local_pos);
    glm_quat_copy(rot, joints[0].local_rot);

    glm_quat_mat4(rot, joints[0].transform);
    glm_scale_uni(joints[0].transform, scale);
    glm_vec3_copy(pos, joints[0].transform[3]);
}

void Armature::apply_animations() {
    float t;
    for( unsigned int i = 0; i < playing_animations.size(); ++i ) {
        AnimationPlayData &p = playing_animations[i];

        // End behavior for animations
        if( p.current_time > p.animation->duration || p.current_time < 0 )
            switch( p.playback_type ) {

                // Mdodulate the current animation time to the duration forcing playback to start over
                case AnimationPlayData::LOOP :
                    p.current_time = fmod( p.animation->duration + p.current_time, p.animation->duration );
                    break;

                // Inverse the playback speed when the duration is passed, reverses the animation
                case AnimationPlayData::BACK:
                    p.playback_speed *= -1;
                    break;

                // Clamp the time to the last frame
                case AnimationPlayData::CLAMPED:
                    p.current_time = p.animation->duration;
                    break;

                // Clamp to the duration of the animation from 0 to duration
                default:
                case AnimationPlayData::TERMINAL:
                    p.current_time = fmax( fmin( p.current_time, p.animation->duration ), 0 );
                    // TODO Stop the animation?
                    playing_animations.erase(playing_animations.begin()+i);
                    break;
            }
        p.animation->apply( *this, p.current_time );
    }
}

const Animation *Armature::get_animation( string name ) {
    return info->get_animation( name );
}

void Animation::apply( Armature &a, float time ) const {
    unsigned int poskeyindex, rotkeyindex;
    for( const Channel &c : channels ) {
        poskeyindex = c.positions.size();
        for( unsigned int i = 0; i < c.positions.size(); ++i ) {
            if( c.positions[i].t > time ) {
                poskeyindex = i;
                break;
            }
        }
        // Apply post-animation deformation
        if( poskeyindex == c.positions.size() ) {
            glm_vec3_copy( const_cast<vec3 &>( c.positions.back().v ), a.joints[c.joint].local_pos );
        }
        // Skip pre-animation deformation, apply interpolation
        else if( poskeyindex != 0 ) {
            glm_vec3_lerp(
                const_cast<vec3 &>( c.positions[poskeyindex - 1].v ),
                const_cast<vec3 &>( c.positions[poskeyindex].v ),
                ( time - c.positions[poskeyindex - 1].t ) / ( c.positions[poskeyindex].t - c.positions[poskeyindex - 1].t ),
                a.joints[c.joint].local_pos
            );
        }

        rotkeyindex = c.rotations.size();
        for( unsigned int i = 0; i < c.rotations.size(); ++i ) {
            if( c.rotations[i].t > time ) {
                rotkeyindex = i;
                break;
            }
        }
        // Apply post-animation deformation
        if( rotkeyindex == c.rotations.size() ) {
            glm_quat_copy( const_cast<versor &>( c.rotations.back().v ), a.joints[c.joint].local_rot );
        }
        // Skip pre-animation deformation, apply interpolation
        else if( rotkeyindex != 0 ) {
            glm_quat_nlerp(
                const_cast<versor &>( c.rotations[rotkeyindex - 1].v ),
                const_cast<versor &>( c.rotations[rotkeyindex].v ),
                ( time - c.rotations[rotkeyindex - 1].t ) / ( c.rotations[rotkeyindex].t - c.rotations[rotkeyindex - 1].t ),
                a.joints[c.joint].local_rot
            );
        }

    }
}

mat4 *Armature::get_transform_buffer() {
    return transform_buffer;
}

void Armature::add_softbody( std::string joint_name ) {
    add_softbody( joint_name, 0.8, 0.7, 0.4, 0.3, 1, 0.1 );
}

void Armature::add_softbody( string joint_name, float elasticity, float drag, float friction, float rigidity, float gravity, float default_length ) {
    uint32_t joint_id = info->get_joint( joint_name );
    if( joint_id == 0 ) {
        std::cout << "Could not create softbody constraint for " << joint_name << ": Unknown joint name" << std::endl;
        return;
    }
    if( joints[joint_id].constraint_type == CONSTRAINT_SOFTBODY ) {
        SoftbodyConstraint &softbody = softbody_constraints[joints[joint_id].constraint_id];
        softbody.elasticity = elasticity;
        softbody.drag = drag;
        softbody.friction = friction;
        softbody.rigidity = rigidity;
        softbody.gravity = gravity;
        softbody.joint_length = default_length;
    }
    else if( softbody_constraints.size() < UINT8_MAX  && joints[joint_id].constraint_type == CONSTRAINT_NULL ) {
        SoftbodyConstraint softbody;
        softbody.joint = joint_id;
        softbody.elasticity = elasticity;
        softbody.drag = drag;
        softbody.friction = friction;
        softbody.rigidity = rigidity;
        softbody.gravity = gravity;
        softbody.joint_length = default_length;

        // Forbid making a parent joint with more than one constrained child
        int child_constraint = -1;
        for( uint8_t child_id : joints[joint_id].children ) {
            if( child_id != 0 && joints[child_id].constraint_type == CONSTRAINT_SOFTBODY ) {
                if( child_constraint >= 0 ) {
                    std::cout << "Could not create softbody constraint for " << joint_name << ": Has more than one constrained child" << std::endl;
                    return;
                }
                child_constraint = joints[child_id].constraint_id;
            }
        }

        // Forbid placing a jiggle constraint on a parent that already has a jiggle constraint with a child
        uint8_t parent_constraint = joints[joints[joint_id].parent].constraint_id;
        if( joints[joints[joint_id].parent].constraint_type == CONSTRAINT_SOFTBODY ) {
            if( softbody_constraints[parent_constraint].child_joint != 0 ) {
                std::cout << "Could not create softbody constraint for " << joint_name << ": Parent already has constrained child" << std::endl;
                return;
            }
            else {
                // set child of parent to this if it does not have a child
                softbody_constraints[parent_constraint].child_joint = joint_id;
                softbody.parent_joint = joints[joint_id].parent;

                // update parent length (in the case of multiple children and one is not chosen)
                mat4 tr;
                vec3 head = GLM_VEC3_ZERO_INIT, tail = GLM_VEC3_ZERO_INIT;
                glm_mat4_inv( joints[softbody.parent_joint].inverse_rest, tr );
                glm_mat4_mulv3( tr, head, 1, head );
                glm_mat4_inv( joints[softbody.joint].inverse_rest, tr );
                glm_mat4_mulv3( tr, tail, 1, tail );
                softbody_constraints[parent_constraint].joint_length = glm_vec3_distance( head, tail );
            }
        }
        // If a constrained parent does not exist, keep the parent as null (0)

        // Update child now that the cases were checked
        if( child_constraint >= 0 ) {
            softbody_constraints[child_constraint].parent_joint = joint_id;
            softbody.child_joint = softbody_constraints[child_constraint].joint;

            // Calculate default distance using rest transform
            mat4 tr;
            vec3 head = GLM_VEC3_ZERO_INIT, tail = GLM_VEC3_ZERO_INIT;
            glm_mat4_inv( joints[softbody.joint].inverse_rest, tr );
            glm_mat4_mulv3( tr, head, 1, head );
            glm_mat4_inv( joints[softbody.child_joint].inverse_rest, tr );
            glm_mat4_mulv3( tr, tail, 1, tail );
            softbody.joint_length = glm_vec3_distance( head, tail );
        }
        // Optional calculation of length if only one joint present
        else if( joints[joint_id].children.size() == 1 ) {
            mat4 tr;
            vec3 head = GLM_VEC3_ZERO_INIT, tail = GLM_VEC3_ZERO_INIT;
            glm_mat4_inv( joints[softbody.joint].inverse_rest, tr );
            glm_mat4_mulv3( tr, head, 1, head );
            glm_mat4_inv( joints[joints[softbody.joint].children[0]].inverse_rest, tr );
            glm_mat4_mulv3( tr, tail, 1, tail );
            softbody.joint_length = glm_vec3_distance( head, tail );
            std::cout << joint_name << ' ' << softbody.joint_length << std::endl;
        }

        int insert_index = softbody_constraints.size();
        for( int i = 0; i < softbody_constraints.size(); ++i ) {
            if( softbody_constraints[i].joint > joint_id ) {
                insert_index = i;
                break;
            }
        }

        joints[joint_id].constraint_id =  softbody_constraints.size();
        joints[joint_id].constraint_type = CONSTRAINT_SOFTBODY;

        // Insert with smallest joint id first
        softbody_constraints.insert( softbody_constraints.begin() + insert_index, softbody );

        // Update indices after insertion
        for( int i = insert_index; i < softbody_constraints.size(); ++i ) {
            joints[softbody_constraints[i].joint].constraint_id = i;
        }

    }
    else {
        if( softbody_constraints.size() >= UINT8_MAX )
            std::cout << "Unexpected Error: softbody constraint list full." << std::endl;
        else
            std::cout << "Could not create softbody constraint for " << joint_name << ": already has another constraint" << std::endl;
    }
}

void Armature::update_softbody_positions( SoftbodyConstraint &j ) {
    SoftbodyConstraint *p = j.parent_joint != 0 ? &softbody_constraints[joints[j.parent_joint].constraint_id] : nullptr;

    // Load the head position from parent tail
    vec3 head;
    if( p ) {
        glm_vec3_copy( p->tail_pos, head );
    }
    else {
        glm_vec3_copy( joints[j.joint].transform[3], head );
    }

    vec3 abs_vel, local_vel = GLM_VEC3_ZERO_INIT;
    glm_vec3_sub( j.tail_pos, j.last_tail_pos, abs_vel );
    glm_vec3_sub( abs_vel, head, local_vel );
    glm_vec3_add( local_vel, j.last_anim_pos, local_vel );
    glm_vec3_copy( head, j.last_anim_pos );
    glm_vec3_sub( abs_vel, local_vel, abs_vel );

    // Save position before it is overwritten
    glm_vec3_copy( j.tail_pos, j.last_tail_pos );

    vec3 grav = {0, -j.gravity, 0};

    // Update tail position
    j.tail_pos[0] += (( 1 - j.drag) * abs_vel[0] + ( 1 - j.friction) * local_vel[0]) + grav[0] * constraint_timestep * constraint_timestep;
    j.tail_pos[1] += (( 1 - j.drag) * abs_vel[1] + ( 1 - j.friction) * local_vel[1]) + grav[1] * constraint_timestep * constraint_timestep;
    j.tail_pos[2] += (( 1 - j.drag) * abs_vel[2] + ( 1 - j.friction) * local_vel[2]) + grav[2] * constraint_timestep * constraint_timestep;

    // Length elasticity
    vec3 ideal_tail_pos;
    glm_vec3_sub( j.tail_pos, head, ideal_tail_pos );
    glm_vec3_scale( ideal_tail_pos, j.joint_length / ( glm_vec3_norm( ideal_tail_pos ) + GLM_FLT_EPSILON ), ideal_tail_pos );
    glm_vec3_add( ideal_tail_pos, head, ideal_tail_pos );

    glm_vec3_sub( ideal_tail_pos, j.tail_pos, ideal_tail_pos );
    glm_vec3_scale( ideal_tail_pos, j.elasticity, ideal_tail_pos );
    glm_vec3_add(j.tail_pos, ideal_tail_pos, j.tail_pos);

    // Restoration using rigidity
    ideal_tail_pos[0] =  0;
    ideal_tail_pos[1] =  j.joint_length;
    ideal_tail_pos[2] =  0;
    glm_mat4_mulv3( joints[j.joint].transform, ideal_tail_pos, 1, ideal_tail_pos );

    glm_vec3_sub( ideal_tail_pos, j.tail_pos, ideal_tail_pos );
    glm_vec3_scale( ideal_tail_pos, j.rigidity, ideal_tail_pos );
    glm_vec3_add(j.tail_pos, ideal_tail_pos, j.tail_pos);
}

void Armature::apply_softbody( SoftbodyConstraint &j ) {
    // Align each joint to its virtual jiggle point and translate to head
    versor required_rot, forward = {0, 1, 0}, to_jiggle; // xyz -> yzx
    vec3 head;
    if( j.parent_joint != 0 ) {
        glm_vec3_copy( softbody_constraints[joints[j.parent_joint].constraint_id].tail_pos, head );
    }
    else {
        glm_vec3_copy( joints[j.joint].transform[3], head );
    }

    // Find required world space rotation from current joint transform to jiggle
    glm_mat4_mulv3( joints[j.joint].transform, forward, 0, forward );
    glm_vec3_sub( j.tail_pos, head, to_jiggle );
    glm_vec3_normalize( forward );
    glm_vec3_normalize( to_jiggle );
    glm_quat_from_vecs( forward, to_jiggle, required_rot );

    // Append world-space rotation as a pre-transformation
    mat4 m;
    glm_quat_mat4( required_rot, m );
    glm_vec3_zero( joints[j.joint].transform[3] );
    glm_mat4_mul( m, joints[j.joint].transform, joints[j.joint].transform );
    glm_vec3_copy( head, joints[j.joint].transform[3] );
}

// ARMATURE INFO
Animation *ArmatureInfo::get_animation( string &name ) {
    try {
        return &animations[animation_names.at( name )];
    }
    catch( std::out_of_range &ex ) {
        return nullptr;
    }
}

uint8_t ArmatureInfo::get_anim_id( string &name ) {
    try {
        return animation_names.at( name );
    }
    catch( std::out_of_range &ex ) {
        return 0;
    }
}

uint8_t ArmatureInfo::get_joint( string &name ) {
    try {
        return joint_names.at( name );
    }
    catch( std::out_of_range &ex ) {
        return 0;
    }
}

bool ArmatureInfo::load( string filename ) {
    string filepath = (std::string)DIR_ARMATURES + filename + ".arm";
    std::ifstream filereader;
    filereader.open( filepath, std::ios::out | std::ios::binary );
    if( !filereader ) {
        std::cout << "Unable to read armature file: " << filepath << std::endl;
        return false;
    }

    // Clear old data
    armature.info = this;
    armature.joints.clear();
    delete[] armature.transform_buffer;
    armature.transform_buffer = nullptr;
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
    string joint_name;

    // Joints
    for( uint8_t i = 0; i < joint_count; ++i ) {
        Armature::Joint joint;

        // Name
        filereader.read( reinterpret_cast<char *>( &name_length ), 1 );
        filereader.read( buffer, name_length );
        joint_name.assign( buffer, name_length );

        // Parent
        filereader.read( reinterpret_cast<char *>( &joint.parent ), 1 );

        // Children
        filereader.read( reinterpret_cast<char *>( &children_count ), 1 );
        filereader.read( buffer, children_count );
        joint.children.assign( buffer, buffer + children_count );

        // Position and Rotation
        filereader.read( reinterpret_cast<char *>( joint.local_pos ), 12 );
        filereader.read( reinterpret_cast<char *>( joint.local_rot ), 16 );

        // Add to armature
        if( armature.joints.size() < ARMATURE_MAX_JOINTS ) {
            armature.joints.push_back( joint );
            joint_names[joint_name] = armature.joints.size() - 1;
        }
    }

    armature.transform_buffer = new mat4[armature.joints.size()];

    // Animations
    uint8_t animation_count, channel_count;
    uint32_t poskey_count, rotkey_count;
    filereader.read( reinterpret_cast<char *>( &animation_count ), 1 );
    printf("animcount:%d\n", animation_count);

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
            Animation::Channel channel;

            // Joint
            filereader.read( reinterpret_cast<char *>( &channel.joint ), 1 );

            // Key counts
            filereader.read( reinterpret_cast<char *>( &poskey_count ), 4 );
            filereader.read( reinterpret_cast<char *>( &rotkey_count ), 4 );

            // Position
            channel.positions.resize( poskey_count );
            for( unsigned int k = 0; k < poskey_count; ++k ) {
                filereader.read( reinterpret_cast<char *>( &channel.positions[k].t ), 4 );
                filereader.read( reinterpret_cast<char *>( &channel.positions[k].v ), 12 );
            }

            // Rotation
            channel.rotations.resize( rotkey_count );
            for( unsigned int k = 0; k < rotkey_count; ++k ) {
                filereader.read( reinterpret_cast<char *>( &channel.rotations[k].t ), 4 );
                filereader.read( reinterpret_cast<char *>( &channel.rotations[k].v ), 16 );
            }

            animation.channels.push_back( channel );
        }

        animation_names[animation.name] = animations.size();
        animations.push_back( animation );
    }

    filereader.close();

    // Construct armature rest transforms
    armature.assign_as_rest();

    return true;
}

// PRINT
void Animation::Channel::print() const {
    printf( "\njoint: %u", joint );
    if( positions.size() > 0 ) {
        printf( "\npositions: " );
        for( const PosKey &key : positions ) {
            printf( " { [%.2f] %.2f %.2f %.2f }", key.t, key.v[0], key.v[1], key.v[2] );
        }
    }
    if( rotations.size() > 0 ) {

        printf( "\nrotations:" );
        for( const RotKey &key : rotations ) {
            printf( " { [%.2f] %.2f %.2f %.2f %.2f }", key.t, key.v[0], key.v[1], key.v[2], key.v[3] );
        }
    }
}

void Animation::print() const {
    printf( "Animation: %s", name.c_str() );
    for( const Channel &c : channels )
        c.print();
}

void Armature::print() const {
    uint8_t id = 0;
    for( const Joint &j : joints ) {
        printf( "Joint: %u parent: %u children: ", id, j.parent );
        for( uint8_t c : j.children ) {
            printf( "%u ", c );
        }
        printf( "\n" );
        ++id;
    }
}
