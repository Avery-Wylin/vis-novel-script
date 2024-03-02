#include "View.h"
#include <cglm/cglm.h>
#include <cstdio>
#include <cmath>
#include <cglm/io.h>

#define TAU 6.283185307179586

// CONSTRUCTORS
View::View() {
    perspectiveUpdated = true;
    transformUpdated = true;
    ratio = 2.0f;
}

View::~View() {

}


/* Returns the combined transformation matrix.
 * Useful for world-space rendered objects.
 */
mat4& View::getTransform() {
    updateTransform();
    return transform;
}

void View::getTransform(mat4& dest) {
    updateTransform();
    glm_mat4_copy(transform,dest);
}

mat4& View::getInverseTransform( ){
    updateTransform();
    return inverseTransform;
}

/* Returns the combined transformation matrix.
 * Useful for world-space rendered objects.
 */
mat4& View::getCombinedTransform() {
    updateTransform();
    if( combinedUpdated ) {
        glm_mul( perspective, inverseTransform, combinedTransform );
        combinedUpdated = false;
    }
    // glm_mat4_copy( combinedTransform, dest );
    return combinedTransform;
}

/* Returns only the perspective matrix.
 * Useful for screen-space rendering aspect correction or backgrounds.
 */
mat4& View::getPerspective() {
    updatePerspective();
    return perspective;
}

vec4* View::get_frustum_planes(float distance_factor){
    getCombinedTransform();
    mat4 ftr;
    glm_perspective( ( fov ) * 3.1415f / 180, ratio, VIEW_NEAR, VIEW_FAR*distance_factor, ftr );
    glm_mul( ftr, inverseTransform, ftr );
    glm_frustum_planes(ftr, frustum_planes);
    return frustum_planes;
}

void View::update(){
    transformUpdated = true;
}

void View::setRatio( int w, int h ) {
    ratio = w / ( float ) h;
    perspectiveUpdated = true;
}

void View::setFOV(float fov){
    this-> fov = fov;
    perspectiveUpdated = true;
}

float View::getRatio() {
    return ratio;
}

// PRIVATE
void View::updateTransform() {
    updatePerspective();
    if( transformUpdated ) {

        // Update the view transform
        glm_quat_mat4(rot, transform);
        glm_vec3_copy(pos, transform[3]);

        // Calculate the Inverse
        glm_mat4_copy( transform, inverseTransform );
        glm_inv_tr( inverseTransform );

        // Mark as not needing updated
        transformUpdated = false;
        combinedUpdated = true;
    }
}

void View::updatePerspective() {
    if( perspectiveUpdated ) {
        // perspective = glm::perspective ( ( 140 ) *3.1415f/180, ratio, NEAR, FAR );
        glm_perspective( ( fov ) * 3.1415f / 180, ratio, VIEW_NEAR, VIEW_FAR, perspective );
        perspectiveUpdated = false;
        combinedUpdated = true;
    }
}
