#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <cmath>

#include "Object.h"


class Camera {
public:
    glm::vec3 worldUp = glm::vec3(0, 0, 1);

    glm::vec3 eye, lookat, right, up;
    float fov;
    float aperture = 0.0;
    float lensRadius;
    glm::vec3 rightn, upn;

    int width, height;
    void init( int pwidth, int pheight ){ width = pwidth; height = pheight; }


    void set(){
        glm::vec3 lookDir = eye - lookat;
        float focus = length( lookDir );
        right = glm::normalize(glm::cross(worldUp, lookDir)) * focus * tanf(fov / 2) * (float)width / (float)height;
        up = glm::normalize(glm::cross(lookDir, right)) * focus * tanf(fov/2);

        rightn = glm::normalize(right);
        upn = glm::normalize(up);

        lensRadius = aperture / 2;
    }

    void set( glm::vec3 peye, glm::vec3 plookat, float pfov ){
        fov = pfov; eye = peye; lookat = plookat;
        set();
    }

    Ray getRay( float x, float y ) {
        // Don't generateNewDir random number if there is no need.
        if( aperture < 0.00001){
            glm::vec3 dir = lookat + right * (2.0f * (x) / width - 1) + up*(2.0f *(y) / height - 1 ) - eye;
            return Ray(eye, dir);
        }

        glm::vec3 randv = lensRadius * randomUnitDiskVec3();
        glm::vec3 offset = upn * randv.x + rightn * randv.y;

        glm::vec3 dir = lookat + right * (2.0f * (x) / width - 1) + up*(2.0f *(y) / height - 1 ) - eye - offset;
        return Ray(eye + offset, dir);
    }
};

#endif