/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#ifndef SPINE_RENDER_GLBATCHRENDER_H_
#define SPINE_RENDER_GLBATCHRENDER_H_

#if __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
        #include <OpenGLES/ES2/gl.h>
    #elif TARGET_OS_MAC
        #define SPINE_MAC
        #include <OpenGL/gl3.h>
    #else
        // not support
    #endif
#else
    #include <GLES2/gl2.h>
#endif

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }

namespace SpineRender {

struct OpenGLTexture {
    int width = 0;
    int height = 0;
    unsigned int textureId = 0;
    unsigned int minFilter = 0;
    unsigned int magFilter = 0;
    unsigned int uWrap = 0;
    unsigned int vWrap = 0;
};

struct OpenGLRenderState {
    unsigned int blendSrc = 0;
    unsigned int blendDst = 0;
    OpenGLTexture texture;
};

struct OpenGLVertex {
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
    float u;
    float v;
};

class GLBatchRender {
public:
    GLBatchRender() : inited_(false), width_(1), height_(1) {};

    bool create(int width, int height);
    void draw(OpenGLVertex *vertices, int vertexCnt, OpenGLRenderState *state);
    void destroy();

    static bool createTexture(const char *path, OpenGLTexture *texture);
    static void releaseTexture(OpenGLTexture *texture);

    static unsigned int createTexture(int width, int height, unsigned char *buffer);

private:
    bool initGL();
    
private:
    bool inited_;
    int width_, height_;
    GLuint shaderProgram_;
    GLuint vbo_;
    GLuint texLoc_;

    OpenGLRenderState currState_;
    
#ifdef SPINE_MAC
    GLuint vao_;
#endif
};

}

#endif //SPINE_RENDER_GLBATCHRENDER_H_
