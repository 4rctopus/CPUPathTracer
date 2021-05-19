#ifndef LIONTEST_BVHNODE_H
#define LIONTEST_BVHNODE_H

#include "AABB.h"
#include "Object.h"


bool compare( const Object* a, const Object* b, int i){
    AABB boxa, boxb;

    a->getAABB(boxa);
    b->getAABB(boxb);

    return boxa.minimum[i] < boxb.minimum[i];
}

bool compareX(const Object* a, const Object* b) {
    return compare(a, b, 0);
}
bool compareY(const Object* a, const Object* b) {
    return compare(a, b, 1);
}
bool compareZ(const Object* a, const Object* b) {
    return compare(a, b, 2);
}


class BVHnode : public Object{
public:
    Object* left;
    Object* right;
    AABB box;

    // End is not inclusive.
    void build(std::vector<Object*>& objects, int start, int end){
        int axis = randomInt(0, 2);
        auto compar = compareX;
        if( axis == 1 ) compar = compareY;
        else if( axis == 2) compar = compareZ;

        int len = end - start;
        if(len == 1) {
            // Add to both children, because we don't check for null children in intersect.
            left = objects[start];
            right = objects[start];
        }else {
            std::sort(objects.begin() + start, objects.begin() + end, compar);

            int mid = start + len / 2;

            if (len == 2) {
                left = objects[start];
                right = objects[start + 1];
            } else {
                left = new BVHnode;
                left->build(objects, start, mid);
                right = new BVHnode;
                right->build(objects, mid, end);
            }
        }

        // Make bounding box for this node, by uniting children.
        AABB leftBox, rightBox;
        left->getAABB(leftBox);
        right->getAABB(rightBox);
        box = leftBox.add(rightBox);
    }

    bool destroy(){
        if(left->destroy()) delete left;
        if(right->destroy()) delete right;
        return true;
    }

    virtual Hit intersect( const Ray& ray, float tMax ){
        if(!box.intersectFast(ray, tMax)){
            return Hit();
        }

        // Presuming both left and right exist.
        Hit leftHit = left->intersect(ray, tMax);
        Hit rightHit = right->intersect(ray, leftHit.valid ? leftHit.t : tMax);

        if( leftHit.t < rightHit.t ) return leftHit.valid ? leftHit : rightHit;
        else return rightHit.valid ? rightHit : leftHit;
    }

    bool getAABB(AABB& aabb) const{
        aabb = box;
        return true;
    }

    // Get the depth of the BVH, for testing purposes.
    int getDepth(){
        return std::max( left->getDepth(), right->getDepth() ) + 1;
    }
};



#endif //LIONTEST_BVHNODE_H
