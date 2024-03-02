#ifndef VIEW_H
#define VIEW_H

#include "definitions.h"
#include "library/glad_common.h"
#include <cglm/cglm.h>



class View {
    private:
        mat4 combinedTransform;
        mat4 inverseCombined;
        mat4 transform;
        mat4 inverseTransform;
        mat4 perspective;
        bool transformUpdated;
        bool perspectiveUpdated;
        bool combinedUpdated;
        float ratio;
        vec4 frustum_planes[6];

    public:
        vec3 pos = GLM_VEC3_ZERO_INIT;
        versor rot = GLM_QUAT_IDENTITY_INIT;

        View();
        virtual ~View();
        mat4& getCombinedTransform();
        mat4& getTransform();
        void getTransform(mat4 &dest);
        mat4& getInverseTransform( );
        mat4& getPerspective( );
        void update();
        void setRatio( int w, int h );
        void setFOV(float fov);
        inline float getFOV(){ return fov;};
        float getRatio();
        vec4* get_frustum_planes(float distance_factor);

    private:
        float fov = 90;
        void updateTransform();
        void updatePerspective();
};

#endif /* VIEW_H */

