/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#ifndef SPINE_RENDER_SKELETONDRAWABLE_H_
#define SPINE_RENDER_SKELETONDRAWABLE_H_

#include <spine/spine.h>
#include "GLBatchRender.h"

namespace spine {

struct GLBlendMode {
    GLBlendMode() = default;
    GLBlendMode(unsigned int src, unsigned int dst) : src(src), dst(dst) {}

    unsigned int src;
    unsigned int dst;
};

class SkeletonDrawable {
public:
    SkeletonDrawable(SpineRender::GLBatchRender *render, SkeletonData *skeleton, AnimationStateData *stateData = nullptr);
    ~SkeletonDrawable();

    void update(float deltaTime);
    void draw();

    void setUsePremultipliedAlpha(bool usePMA) {
        usePremultipliedAlpha = usePMA;
    };
    bool getUsePremultipliedAlpha() {
        return usePremultipliedAlpha;
    };

    Skeleton *getSkeleton() const {
        return skeleton;
    }
    void setSkeleton(Skeleton *sk) {
        skeleton = sk;
    }
    float getTimeScale() const {
        return timeScale;
    }
    void setTimeScale(float scale) {
        timeScale = scale;
    }

    AnimationState *getState() const {
        return state;
    }
    void setState(AnimationState *s) {
        state = s;
    }

private:
    void drawOpengl();

private:
    mutable bool ownsAnimationStateData;
    mutable Vector<float> worldVertices;
    mutable Vector<float> tempUvs;
    mutable Vector<Color> tempColors;
    mutable Vector<unsigned short> quadIndices;
    mutable SkeletonClipping clipper;
    mutable bool usePremultipliedAlpha;

    Skeleton *skeleton;
    AnimationState *state;
    float timeScale;
    Vector<SpineRender::OpenGLVertex> vertexArray;
    VertexEffect *vertexEffect;

    SpineRender::OpenGLRenderState _states;
    GLBlendMode _blendMode;
    SpineRender::GLBatchRender *_render;
};

class OpenGLTextureLoader : public TextureLoader {
public:
    OpenGLTextureLoader() = default;
    void load(AtlasPage &page, const String &path) override;
    void unload(void *texture) override;
};

}
#endif //SPINE_RENDER_SKELETONDRAWABLE_H_
