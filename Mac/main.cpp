/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include "SpineGLContext.h"
#include "SpineController.h"
#include "utils/Logger.h"

#include <chrono>
#include <thread>

#define WIDTH   300
#define HEIGHT  400

#define JSON_PATH   "../../test/boy/spineboy-ess.json"
#define ATLAS_PATH  "../../test/boy/spineboy.atlas"
#define SKIN_NAME   "default"
#define ANIM_NAME   "idle"
#define POS_X       120
#define POS_Y       500
#define SCALE       1.0

#define FPS 30
#define LOOP_TIME_SECOND 20

int main(int argc, char *argv[]) {
    SpineGLContext glContext;
    bool glOk = glContext.create(WIDTH, HEIGHT);
    if (!glOk) {
        LOG_ERROR("create gl context failed");
        return -1;
    }

    SpineController spineCtrl(WIDTH, HEIGHT);
    bool createOk = spineCtrl.spineCreate(ATLAS_PATH, JSON_PATH, SKIN_NAME, POS_X, POS_Y, SCALE);
    if (!createOk) {
        LOG_ERROR("load spine resources failed");
        return -1;
    }

    createOk = spineCtrl.spineSetAnimation(ANIM_NAME);
    if (!createOk) {
        LOG_ERROR("set spine animation failed");
        return -1;
    }

    float frameIdx = 0;
    float delay = 1000.0f / FPS;
    std::chrono::system_clock::time_point a, b;

    while (frameIdx < FPS * LOOP_TIME_SECOND) {
        a = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> work_time = a - b;

        if (work_time.count() < delay) {
            std::chrono::duration<double, std::milli> delta_ms(delay - work_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
        }

        b = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> sleep_time = b - a;

        glContext.beforeDrawFrame();
        spineCtrl.spineDraw(delay / 1000.0f);
        glContext.afterDrawFrame();

        frameIdx++;
    }
    spineCtrl.spineDestroy();
    glContext.destroy();

    return 0;
}