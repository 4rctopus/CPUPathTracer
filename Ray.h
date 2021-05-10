#pragma once

#include <limits>

struct Ray {
    glm::vec3 start, dir;
    Ray(glm::vec3 pstart, glm::vec3 pdir) {
        start = pstart;
        dir = glm::normalize( pdir );
    }
};

class Object;

struct Hit {
    glm::vec3 position, normal;
    Object * object;
    float t = std::numeric_limits<float>::infinity();
    bool valid = false;
    bool frontFace = true;
};