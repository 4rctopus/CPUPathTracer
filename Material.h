#pragma once

#include "RandomVector.h"
#include "Ray.h"

const float eps = 0.0001f;

class Material {
public:
    glm::vec3 albedo;

    // For direct lightning, without path tracing.
    glm::vec3 ambient, specular;
    float shininess;

    Material( glm::vec3 pdiffuse, glm::vec3 pspecular, float pshininess ){
        ambient = pdiffuse * glm::pi<float>();
        albedo = pdiffuse;
        specular = pspecular;
        shininess = pshininess;
    }

    virtual Ray scatter( const Ray& ray, const Hit& hit, glm::vec3& attenuation, float& pdf) = 0;

    virtual bool diffuse(){ return false; }
    virtual bool emissive(){ return false; }
    virtual glm::vec3 emit(const Hit& hit){return glm::vec3(0,0,0);}
    virtual float pdf(const Ray& ray, const Hit& hit, const Ray& outRay ){ return 1;}
    virtual bool noPdf(){ return true; }
    virtual bool transparent(){return false;}
};

class DiffuseMaterial : public Material {
public:
    DiffuseMaterial( glm::vec3 pdiffuse ) : Material(pdiffuse, glm::vec3( 0.9, 0.9, 0.9 ), 10){}

    Ray scatter( const Ray& ray, const Hit& hit, glm::vec3& attenuation, float& pdf) override {
        attenuation = albedo;
        return Ray(glm::vec3(0,0,0), glm::vec3(0,0,0)); // Placeholder data.
    }

    bool diffuse() override{return true;}

    float pdf(const Ray &ray, const Hit &hit, const Ray &outRay) override {
        float cos = dot( hit.normal, outRay.dir); // Ray.dir is normalized from the start.
        if( cos < 0) return 0;
        return cos / glm::pi<float>(); //  cos(Θ)/π
    }

    bool noPdf() override{ return false; }
};


class MirrorMaterial : public Material{
public:
    float fuzzy = 0.0;
    MirrorMaterial( glm::vec3 pdiffuse, float pfuzzy = 0.0 ) : Material(pdiffuse, glm::vec3( 0.9, 0.9, 0.9 ), 10){
        fuzzy = pfuzzy;
    }

    Ray scatter( const Ray& ray, const Hit& hit, glm::vec3& attenuation, float& pdf) override {
        attenuation = albedo;
        auto newRayDir = glm::reflect(ray.dir, hit.normal);
        newRayDir += fuzzy * randomInSphereVec3();
        if( glm::dot( newRayDir, hit.normal) < 0 ) attenuation.x = -1;
        return {hit.position + hit.normal * eps, newRayDir};
    }
};

class TransparentMaterial : public Material {
public:
    float refIndex;

    TransparentMaterial( float pratio ) : Material(glm::vec3( 0.9, 0.9, 0.9 ), glm::vec3( 0.9, 0.9, 0.9 ), 10){
        refIndex = pratio;
    }

    virtual bool transparent(){return true;}

    Ray scatter( const Ray& ray, const Hit& hit, glm::vec3& attenuation, float& pdf) override {
        attenuation = glm::vec3(1.0, 1.0, 1.0);

        float refRatio = refIndex;
        if( hit.frontFace ) refRatio = 1.0f / refIndex;

        float cost = fmin(glm::dot(-ray.dir, hit.normal), 1.0);
        float sint = sqrt(1.0 - cost*cost);

        bool cannotRefract = refRatio * sint > 1.0;

        float adjEps = -eps;

        glm::vec3 newRayDir(1, 1, 1);
        if( cannotRefract || Schlick(cost, refRatio) > randomFloat() ){
            newRayDir = glm::reflect(ray.dir, hit.normal);
            adjEps = eps;
        }
        else
            newRayDir = glm::refract(ray.dir, hit.normal, refRatio);

        return Ray(hit.position + hit.normal * adjEps, newRayDir);
    }

    // Schlick's approximation.
    float Schlick( float cost, float refRatio ){
        float r0 = (1-refRatio) / (1+refRatio);
        r0 = r0*r0;
        return r0 + (1-r0) * pow(1-cost, 5);
    }
};

class EmissiveMaterial : public Material {

public:
    EmissiveMaterial( glm::vec3 pdiffuse ) : Material(pdiffuse, glm::vec3( 0.9, 0.9, 0.9 ), 10){}

    bool emissive(){ return true; }
    glm::vec3 emit(const Hit &hit) override{
        if( !hit.frontFace ) return glm::vec3(0,0,0);
        else return albedo;
    }
    Ray scatter( const Ray& ray, const Hit& hit, glm::vec3& attenuation, float& pdf) override {
        attenuation = albedo;
        return Ray(glm::vec3(0,0,0), glm::vec3(1, 1, 1));
    }
};