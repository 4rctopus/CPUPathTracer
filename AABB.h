#ifndef LIONTEST_AABB_H
#define LIONTEST_AABB_H

#include <limits>
#include "Ray.h"

const float infinity = std::numeric_limits<float>::infinity();

class AABB{
public:
     glm::vec3 minimum;
     glm::vec3 maximum;

     AABB(){
         minimum = glm::vec3(infinity);
         maximum = glm::vec3(-infinity);
     }
     AABB(glm::vec3 pmin, glm::vec3 pmax){
         minimum = pmin;
         maximum = pmax;
     }

     AABB add(const AABB& other){
         glm::vec3 newMin = glm::vec3(fmin(minimum.x, other.minimum.x),
                                      fmin(minimum.y, other.minimum.y),
                                      fmin(minimum.z, other.minimum.z));

         glm::vec3 newMax = glm::vec3(fmax(maximum.x, other.maximum.x),
                                      fmax(maximum.y, other.maximum.y),
                                      fmax(maximum.z, other.maximum.z));

         return AABB( newMin, newMax);
     }

     bool intersect(const Ray& ray, float tmax){
         // Overall start and end of overlap interval.
         float tmin = 0;
         // Go Through the 3 coordinates.
         for(int i = 0; i < 3; ++i){
             // Intersection t with bounds of interval in this coordinate.
             float tmin0 = (minimum[i] - ray.start[i]) / ray.dir[i];
             float tmax0 = (maximum[i] - ray.start[i]) / ray.dir[i];
             float t0 = fmin( tmin0, tmax0 );
             float t1 = fmax( tmin0, tmax0 );
             // t0 and t1 is the interval in current coordinate, t0 < t1.

             // New overall overlap.
             tmin = fmax( t0, tmin );
             tmax = fmin( t1, tmax );
             // There is no longer an overlap.
             if( tmax <= tmin )
                 return false;
         }
         return true;
     }

     // Optimized implementaion from Andrew Kensler (Pixar)
     bool intersectFast(const Ray& ray, float tmax){
        float tmin = 0;
        for (int i = 0; i < 3; i++) {
            auto invD = 1.0f / ray.dir[i];
            auto t0 = (minimum[i] - ray.start[i]) * invD;
            auto t1 = (maximum[i] - ray.start[i]) * invD;
            if (invD < 0.0f) std::swap(t0, t1);
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin)
                return false;
        }
        return true;
    }

};

#endif //LIONTEST_AABB_H
