/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include "GLBatchRender.h"
#include "utils/Logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "utils/stb_image.h"

#include <spine/SpineString.h>
#include <spine/Extension.h>

namespace SpineRender {

#define CHECK_GL_ERROR(tag)   {                         \
    GLenum err = glGetError();                          \
    if(err != GL_NO_ERROR) {                            \
        LOG_ERROR("%s, glGetError: %d", tag, err);      \
    }}                                                  \

static void getShaderCompileError(GLuint shader) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
        char *infoLog = new char[infoLen];
        glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
        LOG_ERROR ("Error compiling shader:\n%s\n", infoLog);
        SAFE_DELETE_ARRAY(infoLog)
    }
}

static void getShaderLinkError(GLuint program) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen > 1) {
        char *infoLog = new char[infoLen];
        glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
        LOG_ERROR ("Error link shader:\n%s\n", infoLog);
        SAFE_DELETE_ARRAY(infoLog)
    }
}

bool GLBatchRender::initGL() {
    if (inited_) {
        return true;
    }
    inited_ = true;

    const char *vertex_shader =
        "#version 100\n"
        "attribute vec2 aPos;\n"
        "attribute vec4 aColor;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec4 ourColor;\n"
        "varying vec2 vTexCoord;\n"
        "uniform vec2 ourSize;\n"
        "void main() {"
        "  gl_Position = vec4((aPos.x-ourSize.x/2.0)/ourSize.x, (ourSize.y/2.0-aPos.y)/ourSize.y, 0.0, 1.0);"
        "  ourColor = aColor;"
        "  vTexCoord = aTexCoord;"
        "}";

    const char *fragment_shader =
        "#version 100\n"
        "precision mediump float;\n"
        "varying vec4 ourColor;\n"
        "varying vec2 vTexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {"
        "  gl_FragColor = ourColor * texture2D(ourTexture, vTexCoord);"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    if (!vs) {
        LOG_ERROR("create shader vs error");
        return false;
    }
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);

    GLint compiled;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        LOG_ERROR("compile shader vs error");
        getShaderCompileError(vs);
        return false;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    if (!fs) {
        LOG_ERROR("create shader fs error");
        return false;
    }

    glShaderSource(fs, 1, &fragment_shader, nullptr);
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        LOG_ERROR("compile shader fs error");
        getShaderCompileError(fs);
        return false;
    }

    shaderProgram_ = glCreateProgram();
    if (!shaderProgram_) {
        LOG_ERROR("create shader program error");
        return false;
    }

    glAttachShader(shaderProgram_, fs);
    glAttachShader(shaderProgram_, vs);
    glLinkProgram(shaderProgram_);

    GLint linked;
    glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &linked);
    if (!linked) {
        LOG_ERROR("link shader error: %d", linked);
        getShaderLinkError(shaderProgram_);
        return false;
    }

    CHECK_GL_ERROR("link shader")

    glUseProgram(shaderProgram_);

    texLoc_ = glGetUniformLocation(shaderProgram_, "ourTexture");
    glUniform1i(texLoc_, 0);

    GLuint sizeLoc = glGetUniformLocation(shaderProgram_, "ourSize");
    glUniform2f(sizeLoc, (GLfloat) width_, (GLfloat) height_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    CHECK_GL_ERROR("set shader uniform")

#ifdef SPINE_MAC
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);
#endif

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    GLuint posSlot = glGetAttribLocation(shaderProgram_, "aPos");
    GLuint colorSlot = glGetAttribLocation(shaderProgram_, "aColor");
    GLuint texCoordSlot = glGetAttribLocation(shaderProgram_, "aTexCoord");

    glEnableVertexAttribArray(posSlot);
    glEnableVertexAttribArray(colorSlot);
    glEnableVertexAttribArray(texCoordSlot);

    glVertexAttribPointer(posSlot, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) nullptr);
    glVertexAttribPointer(colorSlot, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (2 * sizeof(float)));
    glVertexAttribPointer(texCoordSlot, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) (6 * sizeof(float)));

    CHECK_GL_ERROR("create gl buffer")

    return true;
}

bool GLBatchRender::create(int width, int height) {
    width_ = width;
    height_ = height;

    return initGL();
}

void GLBatchRender::draw(OpenGLVertex *vertices, int vertexCnt, OpenGLRenderState *state) {
    if (!inited_) {
        return;
    }
    if (vertices == nullptr || vertexCnt <= 0 || state == nullptr) {
        return;
    }

    // draw
    glUseProgram(shaderProgram_);

    if (currState_.blendSrc != state->blendSrc || currState_.blendDst != state->blendDst) {
        glEnable(GL_BLEND);
        glBlendFunc(state->blendSrc, state->blendDst);

        currState_.blendSrc = state->blendSrc;
        currState_.blendDst = state->blendDst;
    }

    if (state->texture.textureId != 0 && currState_.texture.textureId != state->texture.textureId) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, state->texture.textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, state->texture.uWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, state->texture.vWrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, state->texture.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, state->texture.magFilter);

        currState_.texture = state->texture;
    }

#ifdef SPINE_MAC
    glBindVertexArray(vao_);
#endif

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, vertexCnt * sizeof(OpenGLVertex), vertices, GL_STATIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, vertexCnt);

#if DEBUG
    CHECK_GL_ERROR("draw");
#endif
}

void GLBatchRender::destroy() {
    if (inited_) {
        inited_ = false;

#ifdef SPINE_MAC
        glDeleteVertexArrays(1, &vao_);
#endif
        glDeleteBuffers(1, &vbo_);
        glDeleteProgram(shaderProgram_);
    }
}


bool GLBatchRender::createTexture(const char *path, OpenGLTexture *texture) {
    int w, h, n;
    unsigned char *buffer;
    stbi_set_unpremultiply_on_load(1);
    stbi_convert_iphone_png_to_rgb(1);

    // read file using spine extension
    const spine::String sPath(path);
    int fileLen = 0;
    char *fileContent = spine::SpineExtension::readFile(sPath, &fileLen);
    if (fileContent) {
        buffer = stbi_load_from_memory((stbi_uc *)fileContent, fileLen, &w, &h, &n, 4);
        spine::SpineExtension::free(fileContent, __FILE__, __LINE__);
    }

    if (buffer == nullptr) {
        LOG_ERROR("Failed to load - %s\n", stbi_failure_reason());
        return false;
    }

    // premultiply alpha
    if (n == 4) {
        for (int i = 0; i < w * h * 4; i += 4) {
            float alpha = buffer[i + 3] / 255.0f;
            buffer[i] *= alpha;
            buffer[i + 1] *= alpha;
            buffer[i + 2] *= alpha;
        }
    }

    texture->textureId = createTexture(w, h, buffer);
    texture->width = w;
    texture->height = h;

    stbi_image_free(buffer);
    return true;
}
void GLBatchRender::releaseTexture(OpenGLTexture *texture) {
    if (texture) {
        glDeleteTextures(1, &texture->textureId);
        texture->textureId = 0;
    }
}
unsigned int GLBatchRender::createTexture(int width, int height, unsigned char *buffer) {
    if (width > 0 && height > 0 && buffer != nullptr) {
        GLint currTextureId = 0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &currTextureId);

        GLuint texId;
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_2D, texId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, currTextureId);
        return texId;
    } else {
        LOG_ERROR("createTexture failed");
    }

    return 0;
}

}
