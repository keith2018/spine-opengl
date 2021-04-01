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
import android.os.Bundle;
import android.view.TextureView;

import java.util.concurrent.atomic.AtomicBoolean;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity implements TextureView.SurfaceTextureListener {

    private TextureView mTextureView;
    private GLRendererImpl mRenderer;
    private GLProducerThread mProducerThread = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTextureView = findViewById(R.id.gl_textureView);
        mTextureView.setSurfaceTextureListener(this);

        mRenderer = new GLRendererImpl(this);
    }

    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mRenderer.setViewport(width, height);
        mProducerThread = new GLProducerThread(surface, mRenderer, new AtomicBoolean(true));
        mProducerThread.start();
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
    }

    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mProducerThread.stopRender();
        mProducerThread = null;
        return true;
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
    }
}