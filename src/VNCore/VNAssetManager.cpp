#include "VNAssetManager.h"
#include <Audio.h>
#include <queue>

vec3 x_axis = {1, 0, 0};
vec3 y_axis = {0, 1, 0};
vec3 z_axis = {0, 0, 1};



namespace VNAssets {

    View view;
    KeyframeFloat key_fov;
    KeyframePos key_pos;
    KeyframePos key_focus;
    vec3 focus = GLM_VEC3_ZERO_INIT;
    Scene *active_scene = nullptr;


    std::vector<ShaderContainer> shaders;
    std::vector<ModelContainer> models;
    std::vector<ObjectInstance> objects;

    std::unordered_map<std::string, Scene> scenes;
    std::unordered_map<std::string, uint32_t> shader_names;
    std::unordered_map<std::string, uint32_t> model_names;
    std::unordered_map<std::string, uint32_t> object_names;

    std::unordered_map<std::string, ArmatureInfo> armature_infos;


    void init(){

        // Load null values used as the default
        shaders.push_back(ShaderContainer());
        models.push_back(ModelContainer());
        objects.push_back(ObjectInstance());
    }

    void close(){
        // Free loaded GL assets
        for(ShaderContainer &s:shaders){
            s.shader->free();
        }
        for(ModelContainer &m:models){
            m.vao->free();
        }
    }

    void scene_create( const std::string &name ) {
        scenes[name];
    }

    void scene_delete( const std::string &name ) {
        scenes.erase(name);
    }


    bool scene_select(const std::string &name){
        try {
            active_scene = &scenes.at( name );
            return true;
        }
        catch( std::out_of_range &oor ) {
            printf( "Scene %s not found\n", name.c_str() );
            fflush( stdout );
            return false;
        }
    }

    bool scene_add( const std::string &object ) {
        if(!active_scene)
            return false;

        ObjectInstance *obj = get_object(object);
        if(obj){
            active_scene->add_object(obj - &objects[0]);
            return true;
        }
        return false;
    }

    bool scene_remove( const std::string &object ) {
        if(!active_scene)
            return false;

        ObjectInstance *obj = get_object(object);
        if(obj){
            active_scene->remove_object(obj - &objects[0]);
            return true;
        }
        return false;
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

    ModelContainer *get_model( uint32_t id ){
        return &models[id];
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

    ShaderContainer *get_shader( uint32_t id ) {
        return &shaders[id];
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

    ShaderContainer *null_shader(){
        return &shaders[0];
    }

    ModelContainer *null_model(){
        return &models[0];
    }

    ObjectInstance *null_object(){
        return &objects[0];
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

    bool parent_object(const std::string& child, const std::string& parent){
        uint32_t  c_id, p_id;
        try{
            c_id = object_names.at(child);
            p_id = object_names.at(parent);
        }
        catch(std::out_of_range &oor){
            return false;
        }

        // Do not reparent
        if(objects[c_id].parent == p_id)
            return true;

        // If there is an old parent, remove it
        if(objects[c_id].parent != 0){
            std::vector<uint32_t> &children = objects[objects[c_id].parent].children;
            children.erase( std::remove( children.begin(), children.end(), c_id ) );
        }

        // Link parent and child, remove joint parenting
        objects[c_id].parent = p_id;
        objects[c_id].parent_joint = 0;
        objects[p_id].children.push_back(c_id);
        return true;
    }

    bool parent_joint(const std::string& child, const std::string& parent, const std::string& joint){
        uint32_t  c_id, p_id;
        uint8_t j_id = 0;
        try{
            c_id = object_names.at(child);
            p_id = object_names.at(parent);
        }
        catch(std::out_of_range &oor){
            return false;
        }

        ArmatureInfo *info = objects[p_id].armature.get_info();

        // No armature to get joint
        if(!info)
            return false;

        j_id = info->get_joint_id(joint);

        // No joint to parent to
        if(j_id == 0)
            return false;

        // Do not reparent
        if(objects[c_id].parent == p_id && objects[c_id].parent_joint == j_id )
            return true;

        // If there is an old parent, remove it
        if(objects[c_id].parent != 0){
            std::vector<uint32_t> &children = objects[objects[c_id].parent].children;
            children.erase( std::remove( children.begin(), children.end(), c_id ) );
        }

        // Link parent and child, remove joint parenting
        objects[c_id].parent = p_id;
        objects[c_id].parent_joint = j_id;
        objects[p_id].children.push_back(c_id);
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

        if(active_scene)
            active_scene->update(t);

    }

    void draw(){
        if(active_scene)
            active_scene->draw();
    }

}

bool compare_objects(uint32_t a, uint32_t b){
    ObjectInstance &ao = VNAssets::objects[a], &bo =  VNAssets::objects[b];

    if(ao.shader < bo.shader)
            return true;
        else if (ao.shader == bo.shader)
            if(ao.model < bo.model)
                return true;
            else
                return a < b;
        else
            return false;
}

// Updates all objects in a scene and checks for resorting
void Scene::update(float t){

    for(uint32_t o: objects ){
        // Clear update flags
        VNAssets::objects[o].updated = false;
    }

    for(uint32_t o: objects ){

        // Update the object
        VNAssets::objects[o].update(t);

    }
}

void Scene::draw(){
    // Likely to be sorted

    uint32_t shader = 0;
    uint32_t model = 0;
    bool bad_shader = false;

    vec3 dim = {1, 1, 1};

    glDisable(GL_DEPTH_TEST);
    for(uint32_t o : objects){
        ObjectInstance &obj = VNAssets::objects[o];

        if(!obj.enabled || !obj.shader || !obj.model)
            continue;

        // Change the shader
        if(obj.shader != shader){
            shader = obj.shader;
            ShaderContainer &sc = VNAssets::shaders[shader];
            if( sc.needs_compiled ) {
                sc.shader->load( sc.filename );
                sc.needs_compiled = false;
            }

            // Skip drawing with this shader
            bad_shader = !Shader::bind(*sc.shader);

            if(bad_shader)
                continue;

            // Shader Settings
            if(sc.blend){
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
                glDisable(GL_BLEND);

            if(sc.cull)
                glEnable(GL_CULL_FACE);
            else
                glDisable(GL_CULL_FACE);

            if(sc.depth_test)
                glEnable(GL_DEPTH_TEST);
            else
                glDisable(GL_DEPTH_TEST);

            // Load scene uniforms
            Shader::uniformMat4f( UNIFORM_CAMERA, VNAssets::view.getInverseTransform() );
            Shader::uniformMat4f( UNIFORM_PROJ, VNAssets::view.getPerspective() );
            Shader::uniformVec3f( UNIFORM_CAM_POS, VNAssets::view.pos );
        }

        if(bad_shader)
            continue;

        // Change the model
        if(obj.model != model){
            model = obj.model;
            ModelContainer &mc = VNAssets::models[model];

            // Check model for updates, if so, load to VAO
            if( mc.mesh.updated ) {
                mc.mesh.to_VAO( mc.vao.get() );
                mc.mesh.updated = false;
            }

            if( mc.image_changed ) {
                if( !mc.image_names.empty() ){
                    mc.image_array->load_file_list( mc.image_names, GL_LINEAR );
                }
                mc.image_changed = false;
            }

            if( mc.image_array->is_allocated() ) {
                mc.image_array->bind( 0 );

                if( mc.image_array->get_ratio() <= 1 ) {
                    dim[1] = mc.image_array->get_ratio();
                }
                else {
                    dim[0] = 1 / mc.image_array->get_ratio();
                }
            }
            else{
                dim[0] = 1;
                dim[1] = 1;
            }

            // Bind the VAO
            mc.vao->bind();
        }

        // Draw the object

        // Load the armature if present
        if( !obj.armature.empty() )
            Shader::uniformMat4fArray( UNIFORM_JOINTS, ( ( mat4 * )obj.armature.transform_buffer.get() ), obj.armature.joints.size() );

        // If there is not an armature present, do not load the full uniform, only load the root
        else
            Shader::uniformMat4f( UNIFORM_JOINTS, obj.transform );


        Shader:: uniformVec3f(UNIFORM_TEXID, obj.tex_id);
        Shader::uniformMat4f( UNIFORM_TRANSFORM, obj.transform );
        dim[2] = obj.scale;
        Shader::uniformVec3f( UNIFORM_FACTOR, dim );
        glDrawElements( GL_TRIANGLES, VNAssets::models[model].vao->get_index_count(), GL_UNSIGNED_INT, 0 );

    }
}

// Add an object and all children to the scene
void Scene::add_object(uint32_t id){
    std::queue<uint32_t> q;
    q.push(id);
    while(!q.empty()){
        for(uint32_t c : VNAssets::objects[q.front()].children){
            q.push(c);
        }
        objects.insert(q.front());
        q.pop();
    }
}

// Remove an object and all its children
void Scene::remove_object(uint32_t id){
    std::queue<uint32_t> q;
    q.push(id);
    while(!q.empty()){
        for(uint32_t c : VNAssets::objects[q.front()].children){
            q.push(c);
        }
        objects.erase(q.front());
        q.pop();
    }
}


void ObjectInstance::update( float t ) {

    // Update parents
    uint32_t p = parent;
    while(p != 0){
        if(VNAssets::objects[p].updated)
            break;
        VNAssets::objects[p].update(t);
        p = VNAssets::objects[p].parent;
    }

    if(updated)
        return;

    updated = true;

    if(!enabled)
        return;

    key_position.update(position, t);
    key_rotation.update(rotation, t);
    key_scale.update(scale, t);
    key_texture_mix.update(tex_id[2], t);


    glm_quat_mat4( rotation, transform );
    glm_scale_uni( transform, scale);
    glm_vec3_copy( position, transform[3] );


    if(parent){
        if(parent_joint){
            glm_mat4_mul(VNAssets::objects[parent].armature.get_joint(parent_joint).tr, transform, transform);
        }
        else{
            glm_mat4_mul(VNAssets::objects[parent].transform, transform, transform);
        }
    }

    // Copy transform to root joint, scale constraints accordingly
    if(!armature.empty()){
        glm_mat4_scale(transform, armature.scale_constraint);
        glm_mat4_copy(transform, armature.get_joint(0).tr);
        armature.update(t);
    }
}
