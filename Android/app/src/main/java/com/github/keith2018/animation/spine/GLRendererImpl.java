/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

package com.github.keith2018.animation.spine;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLES20;

public class GLRendererImpl implements GLProducerThread.GLRenderer {

    static {
        System.loadLibrary("native-lib");
    }

    private Context ctx;
    private int mWidth = 0;
    private int mHeight = 0;
    private long spineCtrl = 0;

    public GLRendererImpl(Context ctx) {
        this.ctx = ctx;
    }

    public void setViewport(int width, int height) {
        mWidth = width;
        mHeight = height;
    }


    @Override
    public boolean init() {
        spineCtrl = nativeLoadSpine(ctx.getAssets(), mWidth, mHeight);
        return spineCtrl != 0;
    }

    @Override
    public void drawFrame(float dt) {
        GLES20.glClearColor(0, 0, 0, 0);
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glViewport(0, 0, mWidth, mHeight);

        if (spineCtrl != 0) {
            nativeDrawFrame(spineCtrl, dt);
        }
    }

    @Override
    public void destroy() {
        if (spineCtrl != 0) {
            nativeDestroy(spineCtrl);
        }
    }


    private native long nativeLoadSpine(AssetManager assetManager, int width, int height);
    private native void nativeDrawFrame(long spineCtrl, float dt);
    private native void nativeDestroy(long spineCtrl);
}
