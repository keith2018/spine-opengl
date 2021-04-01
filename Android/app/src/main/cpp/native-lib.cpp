/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string>

#include <spine/spine.h>
#include <SpineController.h>
#include <utils/Logger.h>

#define TAG "spine-jni"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,     TAG,    __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,     TAG,    __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,    TAG,    __VA_ARGS__)

#define JSON_PATH   "boy/spineboy-ess.json"
#define ATLAS_PATH  "boy/spineboy.atlas"
#define SKIN_NAME   "default"
#define ANIM_NAME   "idle"
#define POS_X       400
#define POS_Y       1500
#define SCALE       3.0

static AAssetManager *g_assetMgr = nullptr;


#define JNI_CLASS_PREFIX(name) Java_com_github_keith2018_animation_spine_GLRendererImpl_##name

extern "C" JNIEXPORT
jlong JNI_CLASS_PREFIX(nativeLoadSpine)(JNIEnv *env, jobject /* this */,
                                        jobject assetManager, jint width, jint height) {
    LOG_INFO("nativeLoadSpine: %d, %d", width, height);
    g_assetMgr = AAssetManager_fromJava(env, assetManager);
    if (g_assetMgr == nullptr) {
        LOG_ERROR("AAssetManager_fromJava return nullptr");
        return 0;
    }

    auto *spineCtrl = new SpineController(width, height);
    bool createOk = spineCtrl->spineCreate(ATLAS_PATH, JSON_PATH, SKIN_NAME, POS_X, POS_Y, SCALE);
    if (!createOk) {
        SAFE_DELETE(spineCtrl)
        LOG_ERROR("load spine resources failed");
        return 0;
    }

    createOk = spineCtrl->spineSetAnimation(ANIM_NAME);
    if (!createOk) {
        SAFE_DELETE(spineCtrl)
        LOG_ERROR("set spine animation failed");
        return 0;
    }

    return reinterpret_cast<long>(spineCtrl);
}

extern "C" JNIEXPORT
void JNI_CLASS_PREFIX(nativeDrawFrame)(JNIEnv *env, jobject /* this */,
                                       jlong spineCtrl, jfloat dt) {
    if (0 == spineCtrl) {
        return;
    }
    auto *ctrl = reinterpret_cast<SpineController *>(spineCtrl);
    ctrl->spineDraw(dt);
}

extern "C" JNIEXPORT
void JNI_CLASS_PREFIX(nativeDestroy)(JNIEnv *env, jobject /* this */,
                                     jlong spineCtrl) {
    LOG_INFO("nativeDestroy");
    if (0 == spineCtrl) {
        return;
    }
    auto *ctrl = reinterpret_cast<SpineController *>(spineCtrl);
    ctrl->spineDestroy();

    SAFE_DELETE(ctrl)
}

void SpineLogFunc(void *context, int level, const char *str) {
    if (str == nullptr || strlen(str) <= 0) {
        return;
    }

    switch (level) {
        case SpineRender::INFO:
            LOGI("%s", str);
            break;
        case SpineRender::WARNING:
            LOGW("%s", str);
            break;
        case SpineRender::ERROR:
        case SpineRender::FATAL:
            LOGE("%s", str);
            break;
        default:
            break;
    }
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    SpineRender::Logger::setLogFunc(nullptr, SpineLogFunc);
    return JNI_VERSION_1_4;
}


namespace spine {

    class AndroidSpineExtension : public DefaultSpineExtension {
    protected:
        char *_readFile(const String &path, int *length) override {
            LOG_INFO("AndroidSpineExtension _readFile: %s", path.buffer());
            if (g_assetMgr == nullptr) {
                *length = 0;
                return nullptr;
            }

            char *readBuffer = nullptr;
            AAsset *assetFile = AAssetManager_open(g_assetMgr, path.buffer(), AASSET_MODE_BUFFER);
            if (assetFile) {
                int totalLen = AAsset_getLength(assetFile);
                if (totalLen > 0) {
                    readBuffer = new char[totalLen + 1];
                    int readLen = AAsset_read(assetFile, readBuffer, totalLen);
                    if (readLen != totalLen) {
                        delete[] readBuffer;
                        readBuffer = nullptr;
                        *length = 0;
                    } else {
                        readBuffer[totalLen] = 0;
                        *length = totalLen;
                    }
                }
                AAsset_close(assetFile);
            } else {
                LOG_ERROR("open assets file failed: %s", path.buffer());
            }

            return readBuffer;
        };
    };

    SpineExtension *getDefaultExtension() {
        return new AndroidSpineExtension();
    }

}
