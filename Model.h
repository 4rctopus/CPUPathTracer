#pragma once

#include <vector>
#include <string>
//#include <glm/vec3.hpp>
//#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp> 

#include <iostream>
#include <fstream>

#include "Object.h"

class Model{
public: 
    std::vector <glm::vec3> vertices;
    std::vector <glm::vec2> uvs;
    std::vector <glm::vec3> normals;

    std::vector <glm::ivec3> vertexFaces;
    std::vector <glm::ivec3> normalFaces;
    std::vector <glm::ivec3> uvFaces;


    bool hasnormals = false;


    void addRectangle( glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4){
        vertices.push_back(p1);
        vertices.push_back(p3);
        vertices.push_back(p2);

        vertices.push_back(p4);
        vertices.push_back(p2);
        vertices.push_back(p3);
    }

    void loadBox(glm::vec3 p = glm::vec3(-1, -1, -1), glm::vec3 s = glm::vec3(2, 2, 2)){
        hasnormals = false;

        addRectangle( p, p + glm::vec3(0, s.y, 0), p + glm::vec3(0, 0, s.z), p + glm::vec3(0, s.y, s.z) );
        addRectangle( p, p + glm::vec3(0, 0, s.z), p + glm::vec3(s.x, 0, 0), p + glm::vec3(s.x, 0, s.z) );
        addRectangle( p, p + glm::vec3(s.x, 0, 0), p + glm::vec3(0, s.y, 0), p + glm::vec3(s.x, s.y, 0) );
        addRectangle( p + glm::vec3(0, s.y, 0), p + glm::vec3(s.x, s.y, 0), p + glm::vec3(0, s.y, s.z), p + s );
        addRectangle( p + glm::vec3(s.x, 0, 0), p + glm::vec3(s.x, 0, s.z), p + glm::vec3(s.x, s.y, 0), p + s );
        addRectangle( p + glm::vec3(0, 0, s.z), p + glm::vec3(0, s.y, s.z), p + glm::vec3(s.x, 0, s.z), p + s );

        for( int i = 0; i < 12; ++i ) {
            glm::ivec3 index( 3*i+1, 3*i + 2, 3*i + 3);
            vertexFaces.push_back(index);
        }
    }


    void loadOBJ( std::string  path, int dataNr = 3 ){
        std::ifstream fin(path);

        while( !fin.eof() ){
            std::string start;
            fin >> start;
            if( start == "v" ){
                glm::vec3 vert;
                fin >> vert.x >> vert.y >> vert.z;
                vertices.push_back( vert );
            }
            if( start == "vn" ){
                glm::vec3 normal;
                fin >> normal.x >> normal.y >> normal.z;
                normals.push_back( normal );
            }
            if( start == "vt" ){
                glm::vec2 tex;
                fin >> tex.x >> tex.y;
                uvs.push_back( tex );
            }
            if( start == "f" ){
                char s;
                glm::ivec3 index;
                glm::ivec3 normalIndex;
                glm::ivec3 uvIndex;

                if(dataNr == 3)
                    fin >>
                    index.x >> s >> uvIndex.x >> s >> normalIndex.x >>
                    index.y >> s >> uvIndex.y >> s >> normalIndex.y >>
                    index.z >> s >> uvIndex.z >> s >> normalIndex.z;
                else if(dataNr == 2)
                    fin >> index.x >> s >> uvIndex.x >> index.y >> s >> uvIndex.y >> index.z >> s >> uvIndex.z;
                else if(dataNr == 1)
                    fin >> index.x >> index.y >> index.z;

                vertexFaces.push_back(index);
                normalFaces.push_back(normalIndex);
                uvFaces.push_back(uvIndex);
            }

            fin.ignore( 100000, '\n' ); // Skip rest of line
        }

        fin.close();        
    }

    void add( Material* material, std::vector<Object*>& objects,
              glm::vec3 position = glm::vec3(0.0), glm::vec3 scale = glm::vec3(1.0), float rotX = 0, float rotY = 0, float rotZ = 0){

        glm::mat4 model = glm::identity<glm::mat4>();
        model = glm::scale(model, scale );
        model = glm::rotate(model, rotX, glm::vec3(1.0f, 0.0f, 0.0f ) );
        model = glm::rotate(model, rotY, glm::vec3(0.0f, 1.0f, 0.0f ) );
        model = glm::rotate(model, rotZ, glm::vec3(0.0f, 0.0f, 1.0f ) );
        model = glm::translate(model, position * (1.0f / scale) );



        glm::mat4 inverseModel = glm::inverse(model);



        for( int i = 0; i < vertexFaces.size(); ++i){
            auto face = vertexFaces[i];

            glm::vec4 v1(vertices[face.x - 1], 1.0f); v1 = model * v1;
            glm::vec4 v2(vertices[face.y - 1], 1.0f); v2 = model * v2;
            glm::vec4 v3(vertices[face.z - 1], 1.0f); v3 = model * v3;


            if (hasnormals) {
                auto normalFace = normalFaces[i];
                glm::vec4 n1(normals[normalFace.x - 1], 0.0f); n1 = n1 * inverseModel;
                glm::vec4 n2(normals[normalFace.y - 1], 0.0f); n2 = n2 * inverseModel;
                glm::vec4 n3(normals[normalFace.z - 1], 0.0f); n3 = n3 * inverseModel;


                objects.push_back(
                new Triangle(
                    glm::vec3(v1.x, v1.y, v1.z),
                    glm::vec3(v2.x, v2.y, v2.z),
                    glm::vec3(v3.x, v3.y, v3.z),
                    material,
                    glm::vec3(n1.x, n1.y, n1.z),
                    glm::vec3(n2.x, n2.y, n2.z),
                    glm::vec3(n3.x, n3.y, n3.z)) );
            }else{
                objects.push_back(
                new Triangle(
                    glm::vec3(v1.x, v1.y, v1.z),
                    glm::vec3(v2.x, v2.y, v2.z),
                    glm::vec3(v3.x, v3.y, v3.z),
                    material));
            }
        }
    }

    void testPrint(){
        std::cout << "Vertices: " << vertices.size() << "\n";
        for( auto vert : vertices ){
            std::cout << vert.x << " " << vert.y << " " << vert.z << "\n";
        }

        std::cout << "Faces: " << vertexFaces.size() << "\n";
        for( auto index : vertexFaces ){
            std::cout << index.x << " " << index.y << " " << index.z << "\n";
        }
    }
};