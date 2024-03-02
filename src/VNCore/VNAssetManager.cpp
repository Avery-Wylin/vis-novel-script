#include "VNAssetManager.h"
#include <Audio.h>

vec3 x_axis = {1, 0, 0};
vec3 y_axis = {0, 1, 0};
vec3 z_axis = {0, 0, 1};



namespace VNAssets {

    View view;
    KeyframeFloat key_fov;
    KeyframePos key_pos;
    KeyframePos key_focus;
    vec3 focus = GLM_VEC3_ZERO_INIT;
    Shader image_shader;
    VAO image_vao;

    std::vector<ShaderContainer> shaders;
    std::vector<ModelContainer> models;
    std::vector<ImageContainer> images;
    std::vector<ObjectInstance> objects;

    unordered_map<std::string, uint32_t> shader_names;
    unordered_map<std::string, uint32_t> model_names;
    unordered_map<std::string, uint32_t> image_names;
    unordered_map<std::string, uint32_t> object_names;

    unordered_map<std::string, ArmatureInfo> armature_infos;


    void init(){
        // Load the image shader
        image_shader.load("image");
        float pos_vals[] = {-.5,-.5, .5,-.5, .5,.5, -.5,.5};
        float uv_vals[] = {0,1, 1,1, 1,0, 0,0};
        image_vao.load_attrb_float(ATTRB_POS, 0, 0, 2, 8, pos_vals );
        image_vao.load_attrb_float(ATTRB_UV, 0, 0, 2, 8, uv_vals );

        // Load a null shader, this is never used
        shaders.push_back(ShaderContainer());
    }

    void close(){
        // Free loaded GL assets
        image_shader.free();
        image_vao.free();
        for(ShaderContainer &s:shaders){
            s.shader->free();
        }
        for(ModelContainer &m:models){
            m.vao->free();
        }
    }

    ModelContainer *get_model( const std::string &name ) {
        try {
            uint32_t id = model_names.at( name );
            return &models[id];
        }
        catch( std::out_of_range &oor ) {
            printf( "Model %s not found\n", name.c_str() );
            fflush( stdout );
            return nullptr;
        }
    }

    ImageContainer *get_image( const std::string &name  ) {
        try {
            uint32_t id = image_names.at( name );
            return &images[id];
        }
        catch( std::out_of_range &oor ) {
            printf( "Image %s not found\n", name.c_str() );
            fflush( stdout );
            return nullptr;
        }
    }

    ShaderContainer *get_shader( const std::string &name  ) {
        try {
            uint32_t id = shader_names.at( name );
            return &shaders[id];
        }
        catch( std::out_of_range &oor ) {
            printf( "Shader %s not found\n", name.c_str() );
            fflush( stdout );
            return nullptr;
        }
    }

    ObjectInstance *get_object( const std::string &name  ) {
        try {
            uint32_t id = object_names.at( name );
            return &objects[id];
        }
        catch( std::out_of_range &oor ) {
            printf( "Object %s not found\n", name.c_str() );
            fflush( stdout );
            return nullptr;
        }
    }


    ArmatureInfo *get_armature_info( const std::string &name  ) {
        try {
            return &armature_infos.at( name );
        }
        catch( std::out_of_range &oor ) {
            printf( "Armature %s not found\n", name.c_str() );
            fflush( stdout );
            return nullptr;
        }
    }

    ShaderContainer *create_shader( const std::string &name  ) {
        if( shader_names.contains( name ) )
            return &shaders[shader_names[name]];

        shader_names[name] = shaders.size();
        shaders.push_back( ShaderContainer() );
        return &shaders.back();
    }

    ImageContainer *create_image( const std::string &name  ) {
        if( image_names.contains( name ) )
            return &images[image_names[name]];

        image_names[name] = images.size();
        images.push_back( ImageContainer() );
    }

    ModelContainer *create_model( const std::string &name  ) {
        if( model_names.contains( name ) )
            return &models[model_names[name]];

        model_names[name] = models.size();
        models.push_back( ModelContainer() );
    }

    ObjectInstance *create_object( const std::string &name  ) {
        if( object_names.contains( name ) )
            return &objects[object_names[name]];

        object_names[name] = objects.size();
        objects.push_back( ObjectInstance() );
    }

    ArmatureInfo *load_armature( const std::string &name , const std::string &filename ) {
        if(!armature_infos[name].load( filename )){
            armature_infos.erase(name);
            return nullptr;
        }
        return &armature_infos[name];
    }

    bool link_model_to_shader( const std::string &model, const std::string &shader ) {
        uint32_t m_id, s_id;

        try {
            m_id = VNAssets::model_names.at( model );
            s_id = VNAssets::shader_names.at( shader );
        }
        catch( std::out_of_range &oor ) {
            return false;
        }

        // Treat the default shader as null and do not link
        if(s_id == 0)
            return false;

        // Remove old owner if not null
        uint32_t shader_id = models[m_id].shader_id;
        if( shader_id != 0 ) {
            std::vector<uint32_t> &ms = shaders[shader_id].models;
            ms.erase( std::remove( ms.begin(), ms.end(), m_id ) );
        }

        // Add new owner
        shaders[s_id].models.push_back( m_id );
        models[m_id].shader_id = s_id;
        return true;
    }

    bool link_object_to_model( const std::string &object, const std::string &model ) {
        uint32_t o_id, m_id;

        try {
            o_id = VNAssets::object_names.at( object );
            m_id = VNAssets::model_names.at( model );
        }
        catch( std::out_of_range &oor ) {
            return false;
        }

        // Remove owner if model
        if( objects[o_id].owner_type == ObjectInstance::OWNER_MODEL) {
            std::vector<uint32_t> &os = models[objects[o_id].owner].objects;
            os.erase( std::remove( os.begin(), os.end(), o_id ) );
        }
        // Remove owner if image
        else if( objects[o_id].owner_type == ObjectInstance::OWNER_IMG){
            std::vector<uint32_t> &os = images[objects[o_id].owner].objects;
            os.erase( std::remove( os.begin(), os.end(), o_id ) );
        }


        // Add new owner
        models[m_id].objects.push_back( o_id );
        objects[o_id].owner_type = ObjectInstance::OWNER_MODEL;
        objects[o_id].owner = m_id;
        return true;
    }

    bool link_object_to_image( const std::string &object, const std::string &image ) {
        uint32_t o_id, i_id;

        try {
            o_id = VNAssets::object_names.at( object );
            i_id = VNAssets::image_names.at( image );
        }
        catch( std::out_of_range &oor ) {
            return false;
        }

        // Remove owner if model
        if( objects[o_id].owner_type == ObjectInstance::OWNER_MODEL) {
            std::vector<uint32_t> &os = models[objects[o_id].owner].objects;
            os.erase( std::remove( os.begin(), os.end(), o_id ) );
        }
        // Remove owner if image
        else if( objects[o_id].owner_type == ObjectInstance::OWNER_IMG){
            std::vector<uint32_t> &os = images[objects[o_id].owner].objects;
            os.erase( std::remove( os.begin(), os.end(), o_id ) );
        }


        // Add new owner
        images[i_id].objects.push_back( o_id );
        objects[o_id].owner_type = ObjectInstance::OWNER_IMG;
        objects[o_id].owner = i_id;
        return true;
    }

    void update( float t ) {

        // Update camera animations
        float fov = VNAssets::view.getFOV();
        VNAssets::key_fov.update( fov, t );
        VNAssets::view.setFOV( fov );

        VNAssets::key_pos.update( VNAssets::view.pos, t );
        VNAssets::key_focus.update( VNAssets::focus, t );

        glm_quat_forp( view.pos, focus, y_axis, view.rot );

        Audio::set_listener_transform(view);

        // Update object animations
        for(ObjectInstance &o : objects){
            o.update(t);
        }

    }


    // This is called by the window thread which contains the GL Context
    void draw() {
        view.update();

        // Skip the first null shader
        for( uint32_t i = 1; i < shaders.size(); ++i ) {
            shaders[i].draw();
        }

        // Draw Images
        glEnable(GL_BLEND);
        glDisable(GL_CULL_FACE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        Shader::bind(image_shader);
        Shader::uniformMat4f( UNIFORM_CAMERA, VNAssets::view.getInverseTransform() );
        Shader::uniformMat4f( UNIFORM_PROJ, VNAssets::view.getPerspective() );
        Shader::uniformVec3f( UNIFORM_CAM_POS, VNAssets::view.pos );
        image_vao.bind();
        for(ImageContainer &img : images){
            img.draw();
        }

        glDisable(GL_BLEND);
    }

}

void ObjectInstance::update( float t ) {
    if(!enabled)
        return;
    key_position.update(position, t);
    key_rotation.update(rotation, t);
    key_scale.update(scale, t);
}

void ShaderContainer::draw() {
    if( needs_compiled ) {
        shader->load( filename );
        needs_compiled = false;
    }

    if( !Shader::bind( *shader ) )
        return;

    Shader::uniformMat4f( UNIFORM_CAMERA, VNAssets::view.getCombinedTransform() );
    Shader::uniformVec3f( UNIFORM_CAM_POS, VNAssets::view.pos );

    for( uint32_t model_id : models ) {
        VNAssets::models[model_id].draw();
    }
}

void ModelContainer::draw() {
    if( mesh.updated ) {
        mesh.to_VAO( vao.get() );
        mesh.updated = false;
    }

    mat4 tr;
    vao->bind();
    for( uint32_t object_id : objects ) {
        ObjectInstance &o = VNAssets::objects[object_id];

        if( !o.enabled )
            continue;

        glm_quat_mat4( o.rotation, tr );
        glm_scale_uni( tr, o.scale );
        glm_vec3_copy( o.position, tr[3] );
        Shader::uniformMat4f( UNIFORM_TRANSFORM, tr );
        glDrawElements( GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0 );
    }
}

void ImageContainer::draw() {
    if(needs_loaded){
        image->load_png(filename, GL_LINEAR);
        needs_loaded = false;
    }
    image->bind(0);


    // Set the image ratio, the largest dimension is always size 1
    vec3 dim = {1,1,1};
    if(image->get_ratio() <= 1){
        dim[1] = image->get_ratio();
    }
    else{
        dim[0] = 1/image->get_ratio();
    }

    for( uint32_t object_id : objects ) {
        ObjectInstance &o = VNAssets::objects[object_id];

        if( !o.enabled )
            continue;

        Shader::uniformVec3f( UNIFORM_TRANSFORM, o.position );
        dim[2] = o.scale;
        Shader::uniformVec3f( UNIFORM_FACTOR, dim );
        glDrawArrays( GL_QUADS, 0, 4 );
    }
}

// NOTE Im leaving this in for now because it goes through the process of how to draw an armature

/*
void Character::draw() {
    // Skip draw if not visible
    if( !enabled )
        return;

    // Update the mesh if changed
    if( mesh.updated ) {
        mesh.to_VAO( vao.get() );
        mesh.updated = false;
    }

    if(image_updated){
        image->load_png(image_name, GL_LINEAR);
        image_updated = false;
    }

    // Bind the image texture if present
    image->bind(0);
    vao->bind();

    // Load the armature if present
    if(!armature.empty()){
        Shader::uniformMat4fArray(UNIFORM_JOINTS,armature.get_transform_buffer(), armature.getJoints().size() );
    }
    // If there is not an armature present, do not load the full uniform, only load the root
    else{
        mat4 tr;
        glm_quat_mat4(object.rotation, tr);
        glm_scale_uni(tr, object.scale);
        glm_vec3_copy(object.position, tr[3]);
        Shader::uniformMat4f(UNIFORM_TRANSFORM, tr);
        Shader::uniformMat4f(UNIFORM_JOINTS, tr);
    }

    glDrawElements( GL_TRIANGLES, vao->get_index_count(), GL_UNSIGNED_INT, 0 );
}

void Character::update(float t) {
    // Do not updated disabled characters
    if(!enabled)
        return;

    // Update object animations
    object.update(t);

    if(!armature.empty()){
        armature.set_root_transform(object.position, object.rotation, object.scale);
        armature.update(t);
    }
}
*/





