#ifndef PROGRAM_H
#define PROGRAM_H

#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

#include "Texture.h"

using namespace std;

class Program {
    int id = 0;
    int vertexShader = 0, fragmentShader = 0;


    void getShaderError(int handle) {
        int logLength;
        glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string log(logLength, 0);
            glGetShaderInfoLog(handle, logLength, NULL, &log[0]);
            std::cout << log << "\n";
        }
    }

    void getProgramError(int handle) {
        int logLength;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string log(logLength, 0);
            glGetProgramInfoLog(handle, logLength, NULL, &log[0]);
            std::cout << log << "\n";
        }
    }

    bool checkShader(int shader, std::string message) {
        int ok;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            std::cout << message << ":\n";
            getShaderError(shader);
            return false;
        }
        return true;
    }

    bool checkLinking() {
        int ok;
        glGetProgramiv(id, GL_LINK_STATUS, &ok);
        if (!ok) {
            std::cout << "Failed to link shader program:\n";
            getProgramError(id);
            return false;
        }
        return true;
    }

public:
    int getId(){ return id; }

    bool create(const char* vertexShaderSource, const char* fragmentShaderSource) {
        // Create vertex shader.
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) { std::cout << "Error creating vertex shader!\n"; return false; }
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        if (!checkShader(vertexShader, "Vertex shader error")) return false;

        // Create fragment shader.
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) { std::cout << "Error creating fragment shader!\n"; return false; }
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        if (!checkShader(vertexShader, "Fragment shader error")) return false;

        // Create shader program.
        id = glCreateProgram();
        if (!id) { std::cout << "Error creating shader program!\n"; return false; }
        glAttachShader(id, vertexShader);
        glAttachShader(id, fragmentShader);

        glBindFragDataLocation(id, 0, "fragmentColor");

        glLinkProgram(id);
        if (!checkLinking()) return false;

        return true;
    }

    void createFromFile(const string &vert, const string &frag) {
		ifstream fin( frag );
		stringstream fragmentStringstream;
		fragmentStringstream << fin.rdbuf();

		ifstream fin2(vert);
		stringstream vertexStringstream;
		vertexStringstream << fin2.rdbuf();

        create(vertexStringstream.str().c_str(), fragmentStringstream.str().c_str() );
	}

    // Get the address of a GPU uniform variable.
    int getLocation(const std::string& name) {
		int location = glGetUniformLocation(id, name.c_str());
		if (location < 0) std::cout << "Uniform " << name << " cannot be set\n";
		return location;
	}

    void setUniform(const Texture& texture, const std::string& samplerName, unsigned int textureUnit = 0) {
		int location = getLocation(samplerName);
		if (location >= 0) {
			glUniform1i(location, textureUnit);
			glActiveTexture(GL_TEXTURE0 + textureUnit);
			glBindTexture(GL_TEXTURE_2D, texture.id);
		}
	}

    void setUniform(int i, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0) glUniform1i(location, i);
    }

    void setUniform(float f, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0) glUniform1f(location, f);
    }

    ~Program() { if (id > 0) glDeleteProgram(id); }
};

#endif