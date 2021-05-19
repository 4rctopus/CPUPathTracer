#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <cmath>
#include <vector>
#include <algorithm>

#include "RandomVector.h"
#include "Material.h"
#include "Ray.h"
#include "AABB.h"


struct DirectionalLight {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 direction;
};

struct Light {
    glm::vec3 power;
    glm::vec3 position;
};


class Object{
public:
    Material * material;
    virtual Hit intersect( const Ray& ray, float tMax ) = 0;
    virtual bool getAABB(AABB& aabb) const = 0;

    // Bounding volume hierarchy building.
    virtual void build(std::vector<Object*>& objects, int start, int end){}
    virtual bool destroy(){ return false; }; // Only BVHnodes have to be deallocated.
    virtual int getDepth(){ return 0;}

    virtual float pdf(glm::vec3 origin, const glm::vec3& toObject){ return 1.0; }
    virtual glm::vec3 randomPoint(){ return glm::vec3(1, 0, 0); }
};

class Triangle : public Object {
    public:
    glm::vec3 p1, p2, p3;
    glm::vec3 n1, n2, n3;
    glm::vec3 n;
    glm::mat4 P;
    glm::vec3 AB;
    glm::vec3 AC;

    bool hasnormals = false;
    
    Triangle( glm::vec3 pp1, glm::vec3 pp2, glm::vec3 pp3, Material* pmaterial,
              glm::vec3 pn1 = glm::vec3(-1000), glm::vec3 pn2 = glm::vec3(-1000), glm::vec3 pn3 = glm::vec3(-1000) ){
        p1 = pp1; p2 = pp2; p3 = pp3;
        n1 = pn1; n2 = pn2; n3 = pn3;
        if( n1.x > -999 ) hasnormals = true;
        // Normal.
        n = glm::normalize( glm::cross( p2 - p1, p3 - p1) );

        AB = p2 - p1;
        AC = p3 - p1;

        P = {{ AB.x, AC.x, n.x, p1.x },
             { AB.y, AC.y, n.y, p1.y },
             { AB.z, AC.z, n.z, p1.z },
             { 0,    0,    0,   1 }};
        P = glm::inverse( P );

        material = pmaterial;
    }

    Hit intersect( const Ray& ray, float tMax) {
        Hit hit;

        glm::vec4 o = glm::vec4(ray.start.x, ray.start.y, ray.start.z, 1);
        glm::vec4 d = glm::vec4(ray.dir.x, ray.dir.y, ray.dir.z, 0);

        o = o * P;
        d = d * P;

        hit.t = -o.z / d.z;

        if( hit.t < 0 || hit.t >= tMax) return hit; // Not valid.

        float u = o.x + hit.t * d.x;
        float v = o.y + hit.t * d.y;

        if( u >= 0 && v >= 0 && u + v <= 1 ){
            hit.position = u * AB + v * AC + p1;
            float w = 1 - u - v;
            hit.object = this;

            hit.normal = n;
            if( hasnormals )
                hit.normal = glm::normalize( u * n2 + v * n3 + w * n1 );

            hit.valid = true;
            return hit;
        }else{
            return hit;
        }
    }

    // Moller Trumbore
    Hit intersectMollerTrumbore( const Ray& ray, float tMax){
        Hit hit;
        glm::vec3 v0v1 = p2-p1;
        glm::vec3 v0v2 = p3-p1;
        glm::vec3 pvec = glm::cross(v0v2, ray.dir);
        float det = glm::dot(v0v1, pvec);

        if (glm::abs(det) < eps) return hit;

        float invDet = 1 / det;

        glm::vec3 tvec = ray.start - p1;
        float u = glm::dot(tvec, pvec) *  invDet;
        if (u < 0 || u > 1) return hit;

        glm::vec3 qvec = glm::cross( v0v1, tvec);
        float v = glm::dot(ray.dir, qvec) * invDet;
        if (v < 0 || u + v > 1) return hit;

        float t = glm::dot(v0v2, qvec) * invDet;

        if( t < 0 || t >= tMax) return hit; // Not valid.

        hit.t = t;
        hit.position = ray.start + ray.dir * t;
        hit.normal = n;
        if( hasnormals )
            hit.normal = glm::normalize( u * n2 + v * n3 + (1-u-v) * n1 );
        hit.object = this;
        hit.valid = true;

        return hit;
    }

    Hit intersectSimple( const Ray& ray, float tMax ){
        Hit hit;
        // Intersection with the plane of the triangle.
        hit.t = (glm::dot(n, p1) - glm::dot(n, ray.start)) / glm::dot(n, ray.dir);

        if(hit.t < 0 || hit.t >= tMax) return hit; // Not valid.

        // Point on the plane.
        glm::vec3 p = ray.start + hit.t * ray.dir;

        // Point inside triangle.
        float val1 = glm::dot( glm::cross(p2 - p1, p - p1), n );
        if( val1 < 0 ){ hit.t = -1; return hit;}
        float val2 = glm::dot( glm::cross(p3 - p2, p - p2), n );
        if( val2 < 0 ){ hit.t = -1; return hit;}
        float val3 = glm::dot( glm::cross(p1 - p3, p - p3), n );
        if( val3 < 0 ){ hit.t = -1; return hit;}

        hit.position = p;
        hit.object = this;
        hit.normal = n;
        hit.valid = true;
        return hit;
    }


    bool getAABB( AABB& aabb) const {
        float xmin = std::min({p1.x, p2.x, p3.x});
        float ymin = std::min({p1.y, p2.y, p3.y});
        float zmin = std::min({p1.z, p2.z, p3.z});

        float xmax = std::max({p1.x, p2.x, p3.x});
        float ymax = std::max({p1.y, p2.y, p3.y});
        float zmax = std::max({p1.z, p2.z, p3.z});

        aabb = AABB(glm::vec3(xmin, ymin, zmin) - glm::vec3(eps), glm::vec3(xmax, ymax, zmax) + glm::vec3(eps));
        return true;
    }
};

// For lights, so it is easier to generateNewDir points on surface.
class RectangleZ : public Object {
public:
    Triangle tri1;
    Triangle tri2;
    glm::vec3 p1, p2;

    RectangleZ(glm::vec3 p1, glm::vec3 p2, Material* mat)
        : p1(p1), p2(p2),
        tri1( p1, glm::vec3(p1.x, p2.y, p1.z), glm::vec3(p2.x, p1.y, p1.z), mat ),
        tri2( glm::vec3(p2.x, p1.y, p1.z), glm::vec3(p1.x, p2.y, p1.z), p2, mat ){};


    Hit intersect( const Ray& ray, float tMax ){
        Hit hit1 = tri1.intersect(ray, tMax);
        Hit hit2 = tri2.intersect(ray, tMax);
        if( hit1.t < hit2.t ) return hit1.valid ? hit1 : hit2;
        else return hit2.valid ? hit2 : hit1;
    };

    bool getAABB(AABB& aabb) const {
        tri1.getAABB(aabb);
        AABB aabb2;
        tri2.getAABB(aabb2);
        aabb = aabb.add(aabb2);
        return true;
    }

    float pdf(glm::vec3 origin, const glm::vec3 &newRayDir) override{
        Hit hit = this->intersect(Ray(origin + newRayDir * 0.001f, newRayDir ), infinity );

        if( !hit.valid ) return 0.0;

        glm::vec3 toObject = hit.position - origin;
        float area = (p2.x - p1.x) * (p2.y - p1.y);
        float dist2 = length2(toObject);
        float cosAlpha = glm::abs( glm::dot(newRayDir, hit.normal) );


        return dist2 / (cosAlpha * area);
    }

    glm::vec3 randomPoint() override{
        return glm::vec3(randomFloat(p1.x, p2.x), randomFloat(p1.y, p2.y), p1.z);
    }
};

class RectangleX : public Object{
public:
    Triangle tri1;
    Triangle tri2;
    glm::vec3 p1, p2;

    RectangleX(glm::vec3 p1, glm::vec3 p2, Material* mat)
    : p1(p1), p2(p2),
    tri1( p1, glm::vec3(p1.x, p2.y, p1.z), glm::vec3(p1.x, p1.y, p2.z), mat ),
    tri2( glm::vec3(p1.x, p1.y, p2.z), glm::vec3(p1.x, p2.y, p1.z), p2, mat ){};

    Hit intersect( const Ray& ray, float tMax ){
        Hit hit1 = tri1.intersect(ray, tMax);
        Hit hit2 = tri2.intersect(ray, tMax);
        if( hit1.t < hit2.t ) return hit1.valid ? hit1 : hit2;
        else return hit2.valid ? hit2 : hit1;
    };

    bool getAABB(AABB& aabb) const {
        tri1.getAABB(aabb);
        AABB aabb2;
        tri2.getAABB(aabb2);
        aabb = aabb.add(aabb2);
        return true;
    }

    float pdf(glm::vec3 origin, const glm::vec3 &newRayDir) override{
        Hit hit = this->intersect(Ray(origin + newRayDir * 0.001f, newRayDir ), infinity );

        if( !hit.valid ) return 0.0;

        glm::vec3 toObject = hit.position - origin;
        float area = (p2.z - p1.z) * (p2.y - p1.y);
        float dist2 = length2(toObject);
        float cosAlpha = glm::abs( glm::dot(newRayDir, hit.normal) );

        return dist2 / (cosAlpha * area);
    }


    glm::vec3 randomPoint() override{
        return glm::vec3(p1.x, randomFloat(p1.y, p2.y), randomFloat(p1.z, p2.z));
    }
};


class Sphere : public Object {
public:
    glm::vec3 center;
    float radius;

    Sphere(const glm::vec3& pcenter, float pradius, Material* pmaterial) {
		center = pcenter;
		radius = pradius;
		material = pmaterial;
	}

    Hit intersect( const Ray& ray, float tMax ){
        Hit hit;
        glm::vec3 dist = ray.start - center;
        float a = glm::dot(ray.dir, ray.dir);
        float b = glm::dot(dist, ray.dir) * 2.0f;
        float c = glm::dot(dist, dist) - radius * radius;
        float discr = b * b - 4.0f * a * c;
        if (discr < 0) return hit;
        float sqDiscr = sqrtf(discr);
        float t1 = (-b + sqDiscr) / 2.0f / a;
        float t2 = (-b - sqDiscr) / 2.0f / a;
        if(t1 <= 0 ) return hit; // Not valid.
        hit.t = (t2 > 0) ? t2 : t1;
        if( hit.t >= tMax ) return hit; // Not valid.
        hit.position = ray.start + ray.dir * hit.t;
        hit.normal = (hit.position - center) * (1.0f / radius);
        hit.object = this;
        hit.valid = true;
        return hit;
    }

    bool getAABB(AABB& aabb) const {
        aabb = AABB(center - glm::vec3(radius), center + glm::vec3(radius));
        return true;
    }
};