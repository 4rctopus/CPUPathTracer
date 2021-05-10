#pragma once

#include <chrono>

inline double randomFloat() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    static unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    static std::mt19937 generator(seed);
    return distribution(generator);
}

inline double randomFloat(float min, float max) {
    return min + (max - min) * randomFloat();
}

inline glm::vec3 randomVec3() {
    return glm::vec3(randomFloat(), randomFloat(), randomFloat());
}

inline glm::vec3 randomVec3(float min, float max) {
    return glm::vec3(randomFloat(min, max), randomFloat(min, max), randomFloat(min, max));
}

inline float length2(glm::vec3 v){
    return glm::dot(v, v);
}

inline glm::vec3 randomInSphereVec3(){
    while(true){
        glm::vec3 dir = randomVec3(-1, 1);
        if( length2(dir) <= 1)
            return dir;
    }
}

inline glm::vec3 randomInHemisphereVec3(const glm::vec3& normal){
    glm::vec3 inSphere = randomInSphereVec3();
    if(glm::dot(inSphere, normal) > 0.0)
        return inSphere;
    else
        return -inSphere;
}

inline glm::vec3 randomUnitVec3(){
    return glm::normalize(randomInSphereVec3());
}

inline glm::vec3 refract(const glm::vec3& in, const glm::vec3& n, float ratio){
    float cost = fmin(glm::dot(-in, n), 1.0f);
    glm::vec3 perp = ratio * (in + cost * n);
    glm::vec3 para = - n * glm::sqrt( fabs(1-length2(perp)) );
    return perp + para;
}

inline glm::vec3 randomUnitDiskVec3(){
    while(true){
        glm::vec3 v = glm::vec3(randomFloat(-1, 1), randomFloat(-1, 1), 0);
        if( length2(v) <= 1)
            return v;
    }
}

inline int randomInt(int min, int max){
    return static_cast<int>(randomFloat(min, max+1));
}

inline glm::vec3 randomCosineVec3() {
    auto r1 = randomFloat();
    auto r2 = randomFloat();

    auto phi = 2 * glm::pi<float>() * r1;
    auto x = cos(phi)*sqrt(r2);
    auto y = sin(phi)*sqrt(r2);
    auto z = sqrt(1-r2);

    return glm::vec3(x, y, z);
}

glm::vec3 randomCosineVec3While(){
    float x, y, z;
    while(true){
        x = 2*randomFloat() - 1;
        y = 2*randomFloat() - 1;
        if( x*x + y*y <= 1) break;
    }
    z = sqrt(1 - x*x - y*y);

    return glm::vec3( x, y, z );
}


class ONB{
public:
    glm::vec3 n, s, t;
    ONB(const glm::vec3& pn){
        n = pn;
        glm::vec3 a = glm::vec3(1, 0, 0);
        // If a and n may be parallel, change a
        if( glm::abs(n.x) > 0.9 ) a = glm::vec3(0, 1, 0);
        t = glm::normalize( glm::cross(n, a) );
        s = glm::cross(n, t);
    };

    glm::vec3 get(glm::vec3 v){
        return v.x * s + v.y * t + v.z * n;
    }
};