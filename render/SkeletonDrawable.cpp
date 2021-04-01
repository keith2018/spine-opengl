/*
 *
 * Spine OpenGL
 *
 * @author 	: keith@robot9.me
 * @date	: 2021/03/30
 *
 */

#include "SkeletonDrawable.h"
#include "utils/Logger.h"

namespace spine {

#ifndef SPINE_MESH_VERTEX_COUNT_MAX
#define SPINE_MESH_VERTEX_COUNT_MAX 1000
#endif

GLBlendMode blend_normal = GLBlendMode(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
GLBlendMode blend_additive = GLBlendMode(GL_SRC_ALPHA, GL_ONE);
GLBlendMode blend_multiply = GLBlendMode(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
GLBlendMode blend_screen = GLBlendMode(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

GLBlendMode blend_normalPma = GLBlendMode(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
GLBlendMode blend_additivePma = GLBlendMode(GL_ONE, GL_ONE);
GLBlendMode blend_multiplyPma = GLBlendMode(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
GLBlendMode blend_screenPma = GLBlendMode(GL_ONE, GL_ONE_MINUS_SRC_COLOR);

SkeletonDrawable::SkeletonDrawable(SpineRender::GLBatchRender *render, SkeletonData *skeletonData, AnimationStateData *stateData) :
    _render(render),
    timeScale(1),
    vertexArray(),
    _blendMode(blend_normal),
    vertexEffect(nullptr), worldVertices(), clipper() {
    vertexArray.setSize(skeletonData->getBones().size() * 4, SpineRender::OpenGLVertex());

    Bone::setYDown(true);
    worldVertices.ensureCapacity(SPINE_MESH_VERTEX_COUNT_MAX);
    skeleton = new(__FILE__, __LINE__) Skeleton(skeletonData);
    tempUvs.ensureCapacity(16);
    tempColors.ensureCapacity(16);

    ownsAnimationStateData = stateData == 0;
    if (ownsAnimationStateData) stateData = new(__FILE__, __LINE__) AnimationStateData(skeletonData);

    state = new(__FILE__, __LINE__) AnimationState(stateData);

    quadIndices.add(0);
    quadIndices.add(1);
    quadIndices.add(2);
    quadIndices.add(2);
    quadIndices.add(3);
    quadIndices.add(0);
}

SkeletonDrawable::~SkeletonDrawable() {
    vertexArray.clear();
    if (ownsAnimationStateData) delete state->getData();
    SAFE_DELETE(state)
    SAFE_DELETE(skeleton)
}

void SkeletonDrawable::update(float deltaTime) {
    skeleton->update(deltaTime);
    state->update(deltaTime * timeScale);
    state->apply(*skeleton);
    skeleton->updateWorldTransform();
}

void SkeletonDrawable::draw() {
    vertexArray.clear();

    // Early out if skeleton is invisible
    if (skeleton->getColor().a == 0) return;

    if (vertexEffect != nullptr) vertexEffect->begin(*skeleton);

    SpineRender::OpenGLVertex vertex;
    SpineRender::OpenGLTexture *texture = nullptr;
    for (unsigned i = 0; i < skeleton->getSlots().size(); ++i) {
        Slot &slot = *skeleton->getDrawOrder()[i];
        Attachment *attachment = slot.getAttachment();
        if (!attachment) continue;

        // Early out if the slot color is 0 or the bone is not active
        if (slot.getColor().a == 0 || !slot.getBone().isActive()) {
            clipper.clipEnd(slot);
            continue;
        }

        Vector<float> *vertices = &worldVertices;
        int verticesCount = 0;
        Vector<float> *uvs = nullptr;
        Vector<unsigned short> *indices = nullptr;
        int indicesCount = 0;
        Color *attachmentColor;

        if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
            auto *regionAttachment = (RegionAttachment *) attachment;
            attachmentColor = &regionAttachment->getColor();

            // Early out if the slot color is 0
            if (attachmentColor->a == 0) {
                clipper.clipEnd(slot);
                continue;
            }

            worldVertices.setSize(8, 0);
            regionAttachment->computeWorldVertices(slot.getBone(), worldVertices, 0, 2);
            verticesCount = 4;
            uvs = &regionAttachment->getUVs();
            indices = &quadIndices;
            indicesCount = 6;
            texture =
                (SpineRender::OpenGLTexture *) ((AtlasRegion *) regionAttachment->getRendererObject())->page->getRendererObject();

        } else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
            auto *mesh = (MeshAttachment *) attachment;
            attachmentColor = &mesh->getColor();

            // Early out if the slot color is 0
            if (attachmentColor->a == 0) {
                clipper.clipEnd(slot);
                continue;
            }

            worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
            texture = (SpineRender::OpenGLTexture *) ((AtlasRegion *) mesh->getRendererObject())->page->getRendererObject();
            mesh->computeWorldVertices(slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
            verticesCount = mesh->getWorldVerticesLength() >> 1;
            uvs = &mesh->getUVs();
            indices = &mesh->getTriangles();
            indicesCount = mesh->getTriangles().size();

        } else if (attachment->getRTTI().isExactly(ClippingAttachment::rtti)) {
            auto *clip = (ClippingAttachment *) slot.getAttachment();
            clipper.clipStart(slot, clip);
            continue;
        } else continue;

        float r = skeleton->getColor().r * slot.getColor().r * attachmentColor->r;
        float g = skeleton->getColor().g * slot.getColor().g * attachmentColor->g;
        float b = skeleton->getColor().b * slot.getColor().b * attachmentColor->b;
        float a = skeleton->getColor().a * slot.getColor().a * attachmentColor->a;
        vertex.r = r;
        vertex.g = g;
        vertex.b = b;
        vertex.a = a;

        Color light;
        light.r = r;
        light.g = g;
        light.b = b;
        light.a = a;

        if (!usePremultipliedAlpha) {
            switch (slot.getData().getBlendMode()) {
                case BlendMode_Normal:
                    _blendMode = blend_normal;
                    break;
                case BlendMode_Additive:
                    _blendMode = blend_additive;
                    break;
                case BlendMode_Multiply:
                    _blendMode = blend_multiply;
                    break;
                case BlendMode_Screen:
                    _blendMode = blend_screen;
                    break;
                default:
                    _blendMode = blend_normal;
            }
        } else {
            switch (slot.getData().getBlendMode()) {
                case BlendMode_Normal:
                    _blendMode = blend_normalPma;
                    break;
                case BlendMode_Additive:
                    _blendMode = blend_additivePma;
                    break;
                case BlendMode_Multiply:
                    _blendMode = blend_multiplyPma;
                    break;
                case BlendMode_Screen:
                    _blendMode = blend_screenPma;
                    break;
                default:
                    _blendMode = blend_normalPma;
            }
        }

        _states.texture = *texture;
        _states.blendSrc = _blendMode.src;
        _states.blendDst = _blendMode.dst;

        if (clipper.isClipping()) {
            clipper.clipTriangles(worldVertices, *indices, *uvs, 2);
            vertices = &clipper.getClippedVertices();
            verticesCount = clipper.getClippedVertices().size() >> 1;
            uvs = &clipper.getClippedUVs();
            indices = &clipper.getClippedTriangles();
            indicesCount = clipper.getClippedTriangles().size();
        }

        if (vertexEffect != 0) {
            tempUvs.clear();
            tempColors.clear();
            for (int ii = 0; ii < verticesCount; ii++) {
                Color vertexColor = light;
                Color dark;
                dark.r = dark.g = dark.b = dark.a = 0;
                int index = ii << 1;
                float x = (*vertices)[index];
                float y = (*vertices)[index + 1];
                float u = (*uvs)[index];
                float v = (*uvs)[index + 1];
                vertexEffect->transform(x, y, u, v, vertexColor, dark);
                (*vertices)[index] = x;
                (*vertices)[index + 1] = y;
                tempUvs.add(u);
                tempUvs.add(v);
                tempColors.add(vertexColor);
            }

            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;
                vertex.x = (*vertices)[index];
                vertex.y = (*vertices)[index + 1];
                vertex.u = (*uvs)[index];
                vertex.v = (*uvs)[index + 1];
                Color vertexColor = tempColors[index >> 1];
                vertex.r = vertexColor.r;
                vertex.g = vertexColor.g;
                vertex.b = vertexColor.b;
                vertex.a = vertexColor.a;
                vertexArray.add(vertex);
            }
        } else {
            for (int ii = 0; ii < indicesCount; ++ii) {
                int index = (*indices)[ii] << 1;
                vertex.x = (*vertices)[index];
                vertex.y = (*vertices)[index + 1];
                vertex.u = (*uvs)[index];
                vertex.v = (*uvs)[index + 1];
                vertexArray.add(vertex);
            }
        }
        clipper.clipEnd(slot);
    }
    drawOpengl();
    clipper.clipEnd();

    if (vertexEffect != nullptr) vertexEffect->end();
}
void SkeletonDrawable::drawOpengl() {
    _render->draw(vertexArray.buffer(), vertexArray.size(), &_states);
}

static unsigned int cvtTextureFilter(TextureFilter filter) {
    switch (filter) {
        case TextureFilter_Nearest: return GL_NEAREST;
        case TextureFilter_Linear: return GL_LINEAR;
        case TextureFilter_MipMapNearestNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter_MipMapLinearNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter_MipMapNearestLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter_MipMapLinearLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }

    return GL_LINEAR;
}

static unsigned int cvtTextureWrap(TextureWrap wrap) {
    switch (wrap) {
        case TextureWrap_MirroredRepeat: return GL_MIRRORED_REPEAT;
        case TextureWrap_ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap_Repeat: return GL_REPEAT;
    }

    return GL_REPEAT;
}

void OpenGLTextureLoader::load(AtlasPage &page, const String &path) {
    auto *texture = new SpineRender::OpenGLTexture();
    bool createOk = SpineRender::GLBatchRender::createTexture(path.buffer(), texture);
    if (createOk) {
        texture->textureId = texture->textureId;

        texture->minFilter = cvtTextureFilter(page.minFilter);
        texture->magFilter = cvtTextureFilter(page.magFilter);
        texture->uWrap = cvtTextureWrap(page.uWrap);
        texture->vWrap = cvtTextureWrap(page.vWrap);

        page.width = texture->width;
        page.height = texture->height;
        page.setRendererObject(texture);
    } else {
        LOG_ERROR("load texture failed: %s", path.buffer());
    }
}

void OpenGLTextureLoader::unload(void *texture) {
    if (!texture) return;
    auto *tex = (SpineRender::OpenGLTexture *)texture;
    SpineRender::GLBatchRender::releaseTexture(tex);
    SAFE_DELETE(tex)
}

#ifndef __ANDROID__
SpineExtension *getDefaultExtension() {
    return new DefaultSpineExtension();
}
#endif

}
