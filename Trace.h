#pragma once

#include <vector>
#include <math.h>
#include <SDL2/SDL.h>
#include <random>
#include <map>


#include "Object.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"
#include "PDF.h"

#include "RandomVector.h"

#include "AABB.h"
#include "BVHnode.h"


class Trace {
public:
    BVHnode bvh;
    std::vector<Object*> objects;
    std::vector<Object*> emissiveList;
    std::vector<Light> lights;
    Camera camera;
    DirectionalLight dLight = {{0.2, 0.2, 0.2},
                               {0.7, 0.7, 0.7},
                               {0.5, 0.5, 0.5},
                               {1, 1, 1}};


    std::map<std::string, Material*> materials;

    glm::vec3 backGroundColor1 = glm::vec3(0, 0, 0);
    glm::vec3 backGroundColor2 = glm::vec3(0, 0, 0);

    int width, height;
    int samples = 1;
    int maxDepth = 5;

    int traceFunctionType = 0;
    const float eps = 0.0001f;

    float renderTime = 0.0;

    bool rendering = false;
    int ry = 0;
    std::vector<glm::vec4> pixels;
    unsigned int startTicksLoop = 0;

    std::map<string, void (Trace::*)()> initFunctions;

    // ======== Add box and Rectangle ========
    void addRectangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, Material* mat){
        objects.push_back( new Triangle(  p1, p3, p2, mat ) );
        objects.push_back( new Triangle(  p4, p2, p3, mat ) );
    }

    void addCube( glm::vec3 p, glm::vec3 s, Material* mat ){        
        addRectangle( p, p + glm::vec3(0, s.y, 0), p + glm::vec3(0, 0, s.z), p + glm::vec3(0, s.y, s.z), mat );
        addRectangle( p, p + glm::vec3(0, 0, s.z), p + glm::vec3(s.x, 0, 0), p + glm::vec3(s.x, 0, s.z), mat );
        addRectangle( p, p + glm::vec3(s.x, 0, 0), p + glm::vec3(0, s.y, 0), p + glm::vec3(s.x, s.y, 0), mat );
        addRectangle( p + glm::vec3(0, s.y, 0), p + glm::vec3(s.x, s.y, 0), p + glm::vec3(0, s.y, s.z), p + s, mat );
        addRectangle( p + glm::vec3(s.x, 0, 0), p + glm::vec3(s.x, 0, s.z), p + glm::vec3(s.x, s.y, 0), p + s, mat );
        addRectangle( p + glm::vec3(0, 0, s.z), p + glm::vec3(0, s.y, s.z), p + glm::vec3(s.x, 0, s.z), p + s, mat );
    }
    // ======== Add box and Rectangle ========

    void resize( int pwidth, int pheight ){
        width = pwidth; height = pheight;
        camera.init( width, height );
        pixels.resize(width);
    }

    void initMaterials(){
        materials["green"] = new DiffuseMaterial( glm::vec3( 0.12, 0.45, 0.15 ));
        materials["red"] = new DiffuseMaterial( glm::vec3( 0.65, 0.05, 0.05 ));
        materials["white"] = new DiffuseMaterial( glm::vec3( 0.73, 0.73, 0.73 ));
        materials["emissive"] = new EmissiveMaterial(glm::vec3(15, 15, 15));
        materials["transparent"] = new TransparentMaterial( 1.5 );
        materials["mirror"] = new MirrorMaterial( glm::vec3(0.7, 0.7, 0.7) );
        materials["yellow"] = new DiffuseMaterial(glm::vec3( 0.6, 0.6, 0.2));
        materials["blue"] = new DiffuseMaterial( glm::vec3( 0.2, 0.1, 0.6 ));
        materials["dark"] = new DiffuseMaterial( glm::vec3(0.2, 0.2, 0.2));
        materials["pinkMirror"] = new MirrorMaterial( glm::vec3( 235/255.0, 170/255.0, 230/255.0), 0.3);
        materials["pink"] = new DiffuseMaterial( glm::vec3( 235/255.0, 170/255.0, 230/255.0));
        materials["skyBlue"] = new DiffuseMaterial( glm::vec3( 168/255.0, 204/255.0, 244/255.0 ));
    }

    // ============ Build scene ============
    void initModel(){
        float fov = 45 * glm::pi<float>() / 180;
        camera.set( { 3.5, 0, 1 }, { 0, 0, 1 }, fov );

        backGroundColor1 = glm::vec3(0,0,0);
        backGroundColor2 = glm::vec3(0,0,0);

        addRectangle(glm::vec3(-10, -10, 0), glm::vec3(-10, 10, 0), glm::vec3(10, -10, 0), glm::vec3(10, 10, 0), materials["white"]);

        float x = -3.5;
        float y = -1.5;
        float ys = 3;
        float z = 0.5;
        float zs = 2;
        RectangleX* lightFront = new RectangleX( glm::vec3(x, y, z), glm::vec3(x, y+ys, z+zs), materials["emissive"]);
        objects.push_back(lightFront);
        float s = 3.5;
        float l = 0.7;

        RectangleZ* lightUp = new RectangleZ( glm::vec3(-l,-l,s), glm::vec3(l,l,s), materials["emissive"] );
        objects.push_back(lightUp);

        Model dragon;
        dragon.loadOBJ("dragon.obj");
        dragon.add(materials["yellow"], objects, glm::vec3(0, 0, 0), glm::vec3(0.4), 3.14/2, 3.14/2);

        emissiveList.push_back(lightUp);
        emissiveList.push_back(lightFront);
    }

    void initCornellBoxSides(){
        float fov = 45 * glm::pi<float>() / 180;
        camera.set( { 3.4, 0, 1 }, { 0, 0, 1 }, fov );

        backGroundColor1 = glm::vec3(0,0,0);
        backGroundColor2 = glm::vec3(0,0,0);

        // Make Cornell box.
        float s = 1.0;
        // White Rectangles.
        addRectangle(glm::vec3(-s, -s, 0 ), glm::vec3(-s, s, 0 ), glm::vec3(s, -s, 0), glm::vec3( s, s, 0), materials["white"]);
        addRectangle(glm::vec3(-s, -s, 0 ), glm::vec3(-s, s, 0 ), glm::vec3(-s, -s, s*2), glm::vec3( -s, s, s*2), materials["white"]);
        addRectangle(glm::vec3(-s, -s, s*2 ), glm::vec3(-s, s, s*2), glm::vec3(s, -s, s*2), glm::vec3( s, s, s*2), materials["white"]);
        // Red and Green rectangles.
        addRectangle(glm::vec3(-s, -s, 0 ), glm::vec3(s, -s, 0), glm::vec3(-s, -s, s*2), glm::vec3( s, -s, s*2), materials["red"]);
        addRectangle(glm::vec3(-s, s, 0 ), glm::vec3(s, s, 0), glm::vec3(-s, s, s*2), glm::vec3( s, s, s*2), materials["green"]);
        // Light.
        float l = 0.2;
        float add = -0.01;
        RectangleZ* cornellLight = new RectangleZ( glm::vec3(-l,-l,s*2+add), glm::vec3(l,l,s*2+add), materials["emissive"] );
        objects.push_back(cornellLight);
        emissiveList.push_back(cornellLight);
    }

    void initCornellBoxDefault(){
        initCornellBoxSides();
        Model box;
        box.loadBox(glm::vec3(-0.5, -0.5, 0), glm::vec3(1, 1, 1));
        box.add(materials["white"], objects, glm::vec3(-0.3, -0.3, 0), glm::vec3(0.6, 0.6, 1.1), 0.0, 0.0, 0.3);
        box.add(materials["white"], objects, glm::vec3(0.25, 0.42, 0), glm::vec3(0.6, 0.6, 0.6), 0.0, 0.0, -0.3);
    }

    void initCornellBoxMirrorSphere(){
        initCornellBoxSides();
        Model box;
        box.loadBox(glm::vec3(-0.5, -0.5, 0), glm::vec3(1, 1, 1));
        box.add(materials["mirror"], objects, glm::vec3(-0.3, -0.3, 0), glm::vec3(0.6, 0.6, 1.1), 0.0, 0.0, 0.3);
        objects.push_back( new Sphere( glm::vec3(0.3, 0.3, 0.3), 0.3, materials["transparent"] ) );
    }

    void initCornellBoxDragon(){
        initCornellBoxSides();
        Model dragon;
        dragon.loadOBJ("dragon.obj");
        dragon.add(materials["yellow"], objects, glm::vec3(0, 0.0, 0), glm::vec3(0.2), 3.14/2, 2*3.14/3);
    }

    void initCornellBoxPanther(){
        initCornellBoxSides();
        Model panther;
        panther.loadOBJ("panther.obj");
        panther.add(materials["yellow"], objects, glm::vec3(0, 0, 1), glm::vec3(0.6), 3.14/2, 2*3.14/4.0);
    }

    void initCornellBoxGlassDragon(){
        initCornellBoxSides();
        Model dragon;
        dragon.loadOBJ("dragon.obj");
        dragon.add(materials["transparent"], objects, glm::vec3(0,0.5,0), glm::vec3(0.2), 3.14/2, 2*3.14/3);
    }

    void initTest(){
        float fov = 45 * glm::pi<float>() / 180;
        camera.set( { 0, 13, 3.6 }, { 0, 0, 0 }, fov );

        addCube( glm::vec3( -8, -7, -5 ), glm::vec3( 16, 15, 4.5 ), materials["pink"] ); // Ground

        objects.push_back( new Sphere( glm::vec3(0, 0, 0), 0.5, materials["white"] ) );
        objects.push_back( new Sphere( glm::vec3(0, 0, 2), 0.5, materials["blue"] ) );
        objects.push_back( new Sphere( glm::vec3(0, 2, 0), 0.5, materials["green"] ) );
        objects.push_back( new Sphere( glm::vec3(2, 0, 0), 0.5, materials["red"] ) );

        objects.push_back( new Sphere( glm::vec3(-2, 2, 0), 0.5, materials["transparent"] ) );
        objects.push_back( new Sphere( glm::vec3(-1, 1, 0), 0.5, materials["pinkMirror"] ) );
        objects.push_back( new Sphere( glm::vec3(-1, 0, 0), 0.5, materials["mirror"] ) );
        objects.push_back( new Sphere( glm::vec3(-1, -1, 0), 0.5, materials["mirror"] ) );

        for( int i = -2; i < 2; ++i ) {
            addCube(glm::vec3(-4, i*3, -1), glm::vec3(1, 2, 5), materials["dark"]);
            addCube(glm::vec3(3, i*3, -1), glm::vec3(1, 2, 5), materials["mirror"]);
        }

        backGroundColor1 = glm::vec3(0.3, 0.3, 0.5);
        backGroundColor2 = glm::vec3(0.05, 0.05, 0.2);

        lights.push_back({glm::vec3(10000, 10000, 10000), glm::vec3(30, 0.0001, 30)});
        lights.push_back({glm::vec3(10000, 10000, 10000), glm::vec3(-30, 0.0001, 30)});
    }

    void makeBVH(){ bvh.build(objects, 0, objects.size()); }

    void resetScene(){
        bvh.destroy();
        for( int i = 0; i < objects.size(); ++i)
            delete objects[i];
        objects.clear();
        emissiveList.clear();
        lights.clear();
    }

    void initScene(){
        resize(width, height);

        initMaterials();

        initFunctions["cornell box 1"] = &Trace::initCornellBoxDefault;
        initFunctions["cornell box 2"] = &Trace::initCornellBoxMirrorSphere;
        initFunctions["cornell box dragon"] = &Trace::initCornellBoxDragon;
        initFunctions["cornell box glass dragon"] = &Trace::initCornellBoxGlassDragon;
        initFunctions["lit dragon"] = &Trace::initModel;
        initFunctions["direct lights"] = &Trace::initTest;
        initFunctions["cornell panther"] = &Trace::initCornellBoxPanther;

        initCornellBoxDefault();

        makeBVH();
    }
    // ============ Build scene ============


    // Render row by row, updating the texture right away.
    void startRenderLoop(){
        rendering = true;
        ry = 0;
        startTicksLoop = SDL_GetTicks();
    }

    void renderLoop(Texture &texture){
        if( !rendering ) return;
        // Render one row:
#pragma omp parallel for
        for (int x = 0; x < width; x++) {
            glm::vec3 color = getColor(x, ry);
            pixels[x] = glm::vec4(color.x, color.y, color.z, 1.0f);
        }
        // Send rendered row to texture.
        texture.setRect( 0, ry, width, 1, pixels );

        ry++;
        unsigned int endTicks = SDL_GetTicks();
        renderTime = (endTicks - startTicksLoop) / 1000.0;
        if( ry >= height ){
            rendering = false;
            std::cout << "Render Time: " << renderTime << std::endl;
        }
    }

    // Render the whole image in one go.
    void render(std::vector<glm::vec4>& image){
        rendering = false;
        unsigned int startTicks = SDL_GetTicks();

        for (int y = 0; y < height; y++) {
            std::cout << (float)y / height * 100.0 << "                \r";
#pragma omp parallel for
            for (int x = 0; x < width; x++) {
                glm::vec3 color = getColor(x, y);
                image[y * width + x] = glm::vec4(color.x, color.y, color.z, 1.0f);
            }
        }
        unsigned int endTicks = SDL_GetTicks();
        renderTime = (endTicks - startTicks) / 1000.0;
        std::cout << "100                \r";
    }

    glm::vec3 getColor( int x, int y ){
        glm::vec3 color = glm::vec3(0, 0, 0);
        for (int i = 0; i < samples; ++i) {
            Ray ray = camera.getRay(float(x) + randomFloat(), float(y) + randomFloat());
            color += traceFunction(ray);
        }
        color /= samples;

        return color;
    }

    glm::vec3 traceFunction(const Ray& ray) {
        if(traceFunctionType == 0) return trace(ray);
        else if(traceFunctionType == 1) return traceDirectOnly(ray);
        return glm::vec3( 0, 0, 0);
    }


    glm::vec3 trace(const Ray& ray, int depth = 1){
        if( depth > maxDepth ) return glm::vec3(0, 0, 0);

        Hit hit = firstIntersect(ray);
        if( !hit.valid) return backgroundColor(ray);

        glm::vec3 radiance(0, 0, 0);
        addPointShadow(hit, radiance);

        float pdf = 1.0;
        glm::vec3 attenuation( 0, 0, 0);
        if( hit.object->material->emissive() ) {
            return hit.object->material->emit(hit);
        }

        Ray newRay = hit.object->material->scatter(ray, hit, attenuation, pdf);
        if( attenuation.x < 0 ) return glm::vec3(0, 0, 0); // New ray is wrong.

        // If reflective or refractive pdf is 1.
        if( hit.object->material->noPdf() ){
            radiance += attenuation * trace(newRay, depth + 1);
            return radiance;
        }

        if( emissiveList.empty() ){
            CosinePDF cosinePdf(hit.normal);
            newRay = Ray( hit.position + hit.normal * eps, cosinePdf.generateNewDir());
            pdf = cosinePdf.pdf(hit, newRay.dir);
        }else {
            ObjectListPDF *objectPdf = new ObjectListPDF(emissiveList, hit.position);
            CosinePDF *cosinePdf = new CosinePDF(hit.normal);
            MixturePDF mixturePdf(objectPdf, cosinePdf);

            newRay = Ray(hit.position + hit.normal * eps, mixturePdf.generateNewDir());
            pdf = mixturePdf.pdf(hit, newRay.dir);

            delete objectPdf;
            delete cosinePdf;

            if (pdf < 0.0001) return glm::vec3(0, 0, 0);
        }

        radiance += attenuation * hit.object->material->pdf(ray, hit, newRay) *  trace(newRay, depth + 1) / pdf;
        return radiance;
    }


    void addPointShadow(const Hit& hit, glm::vec3& radiance){
        if( !hit.object->material->diffuse()) return;
        for( auto light : lights ) {
            bool shadow = shadowIntersect(hit, light.position);
            if (!shadow) {
                float dist2 = glm::dot(light.position - hit.position, light.position - hit.position);
                if (dist2 < eps) dist2 = eps;
                glm::vec3 lightRad = light.power / dist2 / 4.0f / 3.1415f;
                glm::vec3 lightDir = glm::normalize(light.position - hit.position);
                float cost = glm::dot(hit.normal, lightDir);

                radiance += hit.object->material->albedo * cost * lightRad;
            }
        }
    }

    // Background color (if no object is hit).
    glm::vec3 backgroundColor(const Ray& ray){
        float h = 0.5 * (ray.dir.z + 1.0);
        return (1 - h) * backGroundColor1 + h * backGroundColor2;
    }

    // Get closest intersection with bvh.
    Hit firstIntersect(const Ray& ray){
        Hit bestHit = bvh.intersect(ray, infinity);
        // If the normal vertexFaces away from us (looking at backface).
        if(bestHit.valid && glm::dot(ray.dir, bestHit.normal) > 0 ) {
            bestHit.normal = bestHit.normal * -1.0f;
            bestHit.frontFace = false;
        }
        return bestHit;
    }

    // Shadow from directional dLight with bvh.
    bool shadowIntersect(Ray ray){
        Hit hit = bvh.intersect(ray, infinity);
        return hit.valid;
    }

    // Shadow intersect with single point light, with bhv.
    bool shadowIntersect( Hit hit, glm::vec3 lightPos){
        Ray ray( hit.position + hit.normal * eps, lightPos - hit.position);
        float dist = glm::length(lightPos - hit.position);
        Hit shadowHit = bvh.intersect(ray, dist);
        return shadowHit.valid;
    }

    // ===============================================================================
    // Get the color from a single ray, no pathtracing.
    glm::vec3 traceDirectOnly( const Ray& ray ){
        Hit hit = firstIntersect(ray);

        if( !hit.valid ){
            return backgroundColor(ray);
        }

        Ray shadowRay( hit.position + hit.normal * eps, dLight.direction );
        bool shadow = shadowIntersect( shadowRay );

        // Blinn Phong.
        glm::vec3 radiance( 0, 0, 0 );
        Material * material = hit.object->material;
        radiance += dLight.ambient * material->ambient;
        if( !shadow ){
            glm::vec3 L = glm::normalize(dLight.direction );
            glm::vec3 H = glm::normalize( L - ray.dir );

            float cost = max( glm::dot( hit.normal, L ), 0.0f );
            float cosd = max( glm::dot( hit.normal, H ), 0.0f );

            radiance += dLight.diffuse * material->albedo * cost;
            radiance += dLight.specular * material->specular * pow(cosd, material->shininess );
        }

        return radiance;
    }

    // Get closest intersection, without bvh.
    Hit firstIntersectNoBVH(const Ray& ray){
        Hit bestHit;
        for( auto object : objects ) {
            Hit hit = object->intersect(ray, bestHit.t);
            if (hit.valid && (hit.t < bestHit.t)) bestHit = hit;
        }

        // If the normal vertexFaces away from us (looking at backface).
        if(bestHit.valid && glm::dot(ray.dir, bestHit.normal) > 0 ) {
            bestHit.normal = bestHit.normal * -1.0f;
            bestHit.frontFace = false;
        }
        return bestHit;
    }

    // Shadow from directional dLight, without bvh.
    bool shadowIntersectNoBVH( Ray ray ){
        for( auto object : objects )
            if( object->intersect(ray, infinity).valid )
                return true;
        return false;
    }

    // Shadow intersect for point dLight, without bhv.
    bool shadowIntersectNoBVH( Hit hit, glm::vec3 lightPos){
        Ray ray( hit.position + hit.normal * eps, lightPos - hit.position);
        float dist = glm::length(lightPos - hit.position);
        for( auto object : objects ){
            Hit occlude = object->intersect(ray, dist + 0.1);
            if( occlude.valid && occlude.t > 0 && occlude.t < dist ){
                return true;
            }
        }
        return false;
    }
};
