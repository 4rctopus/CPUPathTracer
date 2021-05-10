#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <vector>

#include <glm/glm.hpp>

class Texture{
public:
    unsigned int id = 0;
    void create(int width, int height, const std::vector<glm::vec4>& image, int sampling = GL_LINEAR) {
            if (id == 0) glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);    

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, &image[0]); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampling);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampling);
        }

    ~Texture() {
		if (id > 0) glDeleteTextures(1, &id);
	}

	void setPixel( int X, int Y, const std::vector<glm::vec4> &pixels ){
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, 1, 1, GL_RGBA, GL_FLOAT, &pixels[0]);
    }

    void setRect( int X, int Y, int w, int h, const std::vector<glm::vec4> &pixels) {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, X, Y, w, h, GL_RGBA, GL_FLOAT, &pixels[0]);
    }
};

#endif