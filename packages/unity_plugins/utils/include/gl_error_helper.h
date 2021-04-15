#pragma once

#include "glog/logging.h"
#include <GL/glew.h>
#include <string>

namespace unity_plugins {

inline void glCheckError(const std::string& file, int line) {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }

    std::string type = "";
    switch (err) {
    case GL_INVALID_ENUM:
        type = "GL_INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        type = "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        type = "GL_INVALID_OPERATION";
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        type = "GL_INVALID_FRAMEBUFFER_OPERATION";
        break;
    case GL_OUT_OF_MEMORY:
        type = "GL_OUT_OF_MEMORY";
        break;
    case GL_STACK_UNDERFLOW:
        type = "GL_STACK_UNDERFLOW";
        break;
    case GL_STACK_OVERFLOW:
        type = "GL_STACK_OVERFLOW";
        break;
    default:
        type = "Unknown";
        break;
    }

    LOG(ERROR) << "GLError in " << file << " at " << line << ": " << type << " (" << err << ")";
}

#define GL_CHECK_ERROR glCheckError(__FILE__, __LINE__);
}
