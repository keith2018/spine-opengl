/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

package com.github.keith2018.animation.spine;

import android.graphics.SurfaceTexture;
import android.opengl.GLUtils;
import android.util.Log;

import java.util.concurrent.atomic.AtomicBoolean;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;


public class GLProducerThread extends Thread {
    private static final String TAG = "GLProducerThread";

    private AtomicBoolean mShouldRender;
    private SurfaceTexture mSurfaceTexture;
    private GLRenderer mRenderer;

    private EGL10 mEgl;
    private EGLDisplay mEglDisplay = EGL10.EGL_NO_DISPLAY;
    private EGLContext mEglContext = EGL10.EGL_NO_CONTEXT;
    private EGLSurface mEglSurface = EGL10.EGL_NO_SURFACE;
    private GL mGL;

    private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
    private static final int EGL_OPENGL_ES2_BIT = 4;

    private static final float FrameTime = 1000.0f / 30;
    private long lastFrameTime;


    public interface GLRenderer {
        boolean init();
        void drawFrame(float dt);
        void destroy();
    }

    public GLProducerThread(SurfaceTexture surfaceTexture, GLRenderer renderer, AtomicBoolean shouldRender) {
        mSurfaceTexture = surfaceTexture;
        mRenderer = renderer;
        mShouldRender = shouldRender;
    }

    public void stopRender() {
        mShouldRender.set(false);
    }

    private void initGL() {
        mEgl = (EGL10) EGLContext.getEGL();

        mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
        if (mEglDisplay == EGL10.EGL_NO_DISPLAY) {
            throw new RuntimeException("eglGetdisplay failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        int[] version = new int[2];
        if (!mEgl.eglInitialize(mEglDisplay, version)) {
            throw new RuntimeException("eglInitialize failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        int[] configAttribs = {
                EGL10.EGL_BUFFER_SIZE, 32,
                EGL10.EGL_ALPHA_SIZE, 8,
                EGL10.EGL_BLUE_SIZE, 8,
                EGL10.EGL_GREEN_SIZE, 8,
                EGL10.EGL_RED_SIZE, 8,
                EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL10.EGL_SURFACE_TYPE, EGL10.EGL_WINDOW_BIT,
                EGL10.EGL_NONE
        };

        int[] numConfigs = new int[1];
        EGLConfig[] configs = new EGLConfig[1];
        if (!mEgl.eglChooseConfig(mEglDisplay, configAttribs, configs, 1, numConfigs)) {
            throw new RuntimeException("eglChooseConfig failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        int[] contextAttribs = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL10.EGL_NONE
        };
        mEglContext = mEgl.eglCreateContext(mEglDisplay, configs[0], EGL10.EGL_NO_CONTEXT, contextAttribs);
        mEglSurface = mEgl.eglCreateWindowSurface(mEglDisplay, configs[0], mSurfaceTexture, null);
        if (mEglSurface == EGL10.EGL_NO_SURFACE || mEglContext == EGL10.EGL_NO_CONTEXT) {
            int error = mEgl.eglGetError();
            if (error == EGL10.EGL_BAD_NATIVE_WINDOW) {
                throw new RuntimeException("eglCreateWindowSurface returned  EGL_BAD_NATIVE_WINDOW. ");
            }
            throw new RuntimeException("eglCreateWindowSurface failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
            throw new RuntimeException("eglMakeCurrent failed : " +
                    GLUtils.getEGLErrorString(mEgl.eglGetError()));
        }

        mGL = mEglContext.getGL();
    }

    private void destroyGL() {
        mEgl.eglDestroyContext(mEglDisplay, mEglContext);
        mEgl.eglDestroySurface(mEglDisplay, mEglSurface);
        mEglContext = EGL10.EGL_NO_CONTEXT;
        mEglSurface = EGL10.EGL_NO_SURFACE;
    }

    public void run() {
        initGL();

        boolean initOk = false;
        if (mRenderer != null) {
            initOk = mRenderer.init();
        }

        if (initOk) {
            lastFrameTime = System.currentTimeMillis();
            while (mShouldRender != null && mShouldRender.get()) {
                long currentTime = System.currentTimeMillis();
                float elapsed = (currentTime - lastFrameTime);
                lastFrameTime = currentTime;

                if (mRenderer != null) {
                    mRenderer.drawFrame(elapsed * .001f);
                    mEgl.eglSwapBuffers(mEglDisplay, mEglSurface);
                }

                if (elapsed < FrameTime) {
                    try {
                        Thread.sleep((long) (FrameTime - elapsed));
                    } catch (Throwable t) {
                        Log.e(TAG, "" + t);
                    }
                }

            }
        }

        if (mRenderer != null) {
            mRenderer.destroy();
        }
        destroyGL();
    }
}
