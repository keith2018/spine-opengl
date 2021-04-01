/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include "SpineController.h"
#include "utils/Logger.h"

char SpineRender::Logger::buf[MAX_LOG_LENGTH] = {};
void *SpineRender::Logger::logContext = nullptr;
SpineRender::LogFunc SpineRender::Logger::logFunc = nullptr;

spine::SkeletonData *SpineController::spineReadSkeletonJsonData(const spine::String &filename,
                                                                spine::Atlas *atlas,
                                                                float scale) {
    spine::SkeletonJson json(atlas);
    json.setScale(scale);
    auto skeletonData = json.readSkeletonDataFile(filename);
    if (!skeletonData) {
        LOG_ERROR("readSkeletonDataFile failed: %s\n", json.getError().buffer());
        return nullptr;
    }
    return skeletonData;
}

bool SpineController::spineCreate(const char *atlasPath,
                                  const char *jsonPath,
                                  const char *skin,
                                  float posX,
                                  float posY,
                                  float scale,
                                  bool usePMA,
                                  float timeScale) {
    _textureLoader = new spine::OpenGLTextureLoader();
    _atlas = new spine::Atlas(atlasPath, _textureLoader);
    if (_atlas->getPages().size() == 0) {
        LOG_ERROR("Failed to load atlas");
        return false;
    }

    _skeletonData = spineReadSkeletonJsonData(jsonPath, _atlas, scale);
    if (_skeletonData == nullptr) {
        return false;
    }

    _drawable = new spine::SkeletonDrawable(_batchRender, _skeletonData);
    _drawable->setTimeScale(timeScale);
    _drawable->setUsePremultipliedAlpha(usePMA);

    spine::Skeleton *skeleton = _drawable->getSkeleton();
    skeleton->setPosition(posX, posY);
    skeleton->setSkin(skin);
    skeleton->updateWorldTransform();

    return true;
}

bool SpineController::spineSetAnimation(const char *animationName, int trackIndex, bool loop) {
    spine::Animation *animation = _skeletonData->findAnimation(animationName);
    if (animation == nullptr) {
        LOG_ERROR("Failed to find animation: %s", animationName);
        return false;
    }
    _drawable->getState()->setAnimation(trackIndex, animation, loop);

    return true;
}

void SpineController::spineDraw(float dt) {
    if (_drawable) {
        _drawable->update(dt);
        _drawable->draw();
    }
}

void SpineController::spineDestroy() {
    SAFE_DELETE(_skeletonData)
    SAFE_DELETE(_atlas)
    SAFE_DELETE(_drawable)
    SAFE_DELETE(_textureLoader)
}
