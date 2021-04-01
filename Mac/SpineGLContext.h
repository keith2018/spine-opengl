/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#ifndef SPINE_MAC__SPINEGLCONTEXT_H_
#define SPINE_MAC__SPINEGLCONTEXT_H_

#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

class SpineGLContext {
public:
    bool create(int width, int height);
    void destroy();

    void beforeDrawFrame();
    void afterDrawFrame();

private:
    GLFWwindow *_window;

    int _winWidth, _winHeight;
    int _fbWidth, _fbHeight;

};

#endif //SPINE_MAC__SPINEGLCONTEXT_H_
