#ifndef TEXTUREDQUAD_H
#define TEXTUREDQUAD_H

#include <GL/glew.h>

#include "Texture.h"

class TexturedQuad{
public:
    Texture texture;
    unsigned int vao;
    unsigned int vbo;

    void init(){
	    glGenVertexArrays( 1, &vao );
	    glBindVertexArray( vao );

        glGenBuffers( 1, &vbo );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );

        float vertices[] = { -1, -1, 
                              1, -1,  
                              1,  1,
                             -1, 1 };

        glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    void setTexture( int width, int height, std::vector<glm::vec4>& image ){
        texture.create( width, height, image );
    }

    void draw(){
        glBindVertexArray( vao );        
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4 );
    }

    ~TexturedQuad(){ glDeleteBuffers(1, &vbo); }
};

#endif