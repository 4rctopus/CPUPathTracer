#pragma once

class PDF{
public:
    virtual float pdf(const Hit& hit, const glm::vec3& newRayDir){return 1.0;}
    virtual glm::vec3 generateNewDir(){ return glm::vec3(0, 0, 0); }
};

class CosinePDF : public PDF {
public:
    ONB onb;
    CosinePDF(const glm::vec3& n) : onb(n) { }


    float pdf(const Hit& hit, const glm::vec3& newRayDir){
        float cos = dot( hit.normal, glm::normalize(newRayDir)); // Ray.dir is normalized from the start.
        if( cos < 0) return 0;
        return cos / glm::pi<float>(); //  cos(Θ)/π
    }

    glm::vec3 generateNewDir(){
        return onb.get(randomCosineVec3());
    }
};


class ObjectPDF : public PDF {
public:
    glm::vec3 origin;
    Object* object;
    ObjectPDF(Object* object, glm::vec3 origin) : object(object), origin(origin){}

    float pdf(const Hit& hit, const glm::vec3& newRayDir){
        return object->pdf(hit.position, newRayDir);
    }

    glm::vec3 generateNewDir() {
        glm::vec3 onObject = object->randomPoint();
        return onObject - origin;
    }
};

class ObjectListPDF : public PDF{
public:
    std::vector<Object*> objects;
    glm::vec3 origin;

    ObjectListPDF(const std::vector<Object*>& pobjects, const glm::vec3& porigin ){
        objects = pobjects;
        origin = porigin;
    };

    float pdf(const Hit& hit, const glm::vec3& newRayDir) override{
        float weight = 1.0/objects.size();
        float sum = 0.0;

        for(Object* object : objects)
            sum += weight * object->pdf(hit.position, newRayDir);

        return sum;
    }

    glm::vec3 generateNewDir() override {
        int i = randomInt(0, objects.size() - 1);
        glm::vec3 onObject = objects[i]->randomPoint();
        return onObject - origin;
    }
};

class MixturePDF : PDF {
public:
    PDF* pdfs[2];

    MixturePDF(PDF* pdf1, PDF* pdf2){
        pdfs[0] = pdf1;
        pdfs[1] = pdf2;
    }

    float pdf(const Hit& hit, const glm::vec3& newRayDir) override{
        return 0.5 * pdfs[0]->pdf(hit, newRayDir) + 0.5 * pdfs[1]->pdf(hit, newRayDir);
    }

    glm::vec3 generateNewDir() override{
        if( randomFloat() < 0.5 ){
            return pdfs[0]->generateNewDir();
        }else{
            return pdfs[1]->generateNewDir();
        }
    }
};