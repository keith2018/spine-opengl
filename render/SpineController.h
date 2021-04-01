/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#ifndef SPINE_RENDER_SPINECONTROLLER_H_
#define SPINE_RENDER_SPINECONTROLLER_H_

#include "SkeletonDrawable.h"
#include "GLBatchRender.h"

class SpineController {
public:
    SpineController(int width, int height) : _batchRender(new SpineRender::GLBatchRender()) {
        _batchRender->create(width, height);
    }

    ~SpineController() {
        _batchRender->destroy();
        delete _batchRender;
        _batchRender = nullptr;
    }

    bool spineCreate(const char *atlasPath,
                     const char *jsonPath,
                     const char *skin = "default",
                     float posX = 0.0f,
                     float posY = 0.0f,
                     float scale = 1.0f,
                     bool usePMA = true,
                     float timeScale = 1.0f);
    bool spineSetAnimation(const char *animationName, int trackIndex = 0, bool loop = true);
    void spineDraw(float dt);
    void spineDestroy();

private:
    static spine::SkeletonData *spineReadSkeletonJsonData(const spine::String &filename,
                                                          spine::Atlas *atlas,
                                                          float scale);

private:
    spine::OpenGLTextureLoader *_textureLoader = nullptr;
    spine::SkeletonData *_skeletonData = nullptr;
    spine::Atlas *_atlas = nullptr;
    spine::SkeletonDrawable *_drawable = nullptr;

    SpineRender::GLBatchRender *_batchRender = nullptr;
};

#endif //SPINE_RENDER_SPINECONTROLLER_H_
