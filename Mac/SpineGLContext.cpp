/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include "SpineGLContext.h"
#include "utils/Logger.h"

void errorCb(int error, const char* desc) {
    LOG_ERROR("GLFW error %d: %s", error, desc);
}

bool SpineGLContext::create(int width, int height) {
    if (!glfwInit()) {
        LOG_ERROR("Failed to init GLFW.");
        return false;
    }

    glfwSetErrorCallback(errorCb);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);    // transparent window

    _window = glfwCreateWindow(width, height, "SpineOpenGL", nullptr, nullptr);
    if (!_window) {
        LOG_ERROR("Failed to create window.");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(_window);
    glfwSwapInterval(0);
    glfwSetTime(0);

    glfwGetWindowSize(_window, &_winWidth, &_winHeight);
    glfwGetFramebufferSize(_window, &_fbWidth, &_fbHeight);
    glViewport(0, 0, _fbWidth, _fbHeight);

    return true;
}

void SpineGLContext::destroy() {
    glfwTerminate();
}
void SpineGLContext::beforeDrawFrame() {
    glClearColor(0, 0, 0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, _fbWidth, _fbHeight);
}
void SpineGLContext::afterDrawFrame() {
    glfwSwapBuffers(_window);
    glfwPollEvents();
}
