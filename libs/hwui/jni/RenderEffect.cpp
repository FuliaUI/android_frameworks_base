/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Bitmap.h"
#include "GraphicsJNI.h"
#include "SkImageFilter.h"
#include "SkImageFilters.h"
#include "graphics_jni_helpers.h"
#include "utils/Blur.h"
#include <utils/Log.h>

using namespace android::uirenderer;

static jlong createOffsetEffect(
    JNIEnv* env,
    jobject,
    jfloat offsetX,
    jfloat offsetY,
    jlong inputFilterHandle
) {
    auto* inputFilter = reinterpret_cast<const SkImageFilter*>(inputFilterHandle);
    sk_sp<SkImageFilter> offset = SkImageFilters::Offset(offsetX, offsetY, sk_ref_sp(inputFilter));
    return reinterpret_cast<jlong>(offset.release());
}

static jlong createBlurEffect(JNIEnv* env , jobject, jfloat radiusX,
        jfloat radiusY, jlong inputFilterHandle, jint edgeTreatment) {
    auto* inputImageFilter = reinterpret_cast<SkImageFilter*>(inputFilterHandle);
    sk_sp<SkImageFilter> blurFilter =
            SkImageFilters::Blur(
                    Blur::convertRadiusToSigma(radiusX),
                    Blur::convertRadiusToSigma(radiusY),
                    static_cast<SkTileMode>(edgeTreatment),
                    sk_ref_sp(inputImageFilter),
                    nullptr);
    return reinterpret_cast<jlong>(blurFilter.release());
}

static jlong createBitmapEffect(
    JNIEnv* env,
    jobject,
    jlong bitmapHandle,
    jfloat srcLeft,
    jfloat srcTop,
    jfloat srcRight,
    jfloat srcBottom,
    jfloat dstLeft,
    jfloat dstTop,
    jfloat dstRight,
    jfloat dstBottom
) {
    sk_sp<SkImage> image = android::bitmap::toBitmap(bitmapHandle).makeImage();
    SkRect srcRect = SkRect::MakeLTRB(srcLeft, srcTop, srcRight, srcBottom);
    SkRect dstRect = SkRect::MakeLTRB(dstLeft, dstTop, dstRight, dstBottom);
    sk_sp<SkImageFilter> bitmapFilter =
        SkImageFilters::Image(image, srcRect, dstRect, kLow_SkFilterQuality);
    return reinterpret_cast<jlong>(bitmapFilter.release());
}

static jlong createColorFilterEffect(
    JNIEnv* env,
    jobject,
    jlong colorFilterHandle,
    jlong inputFilterHandle
) {
    auto* colorFilter = reinterpret_cast<const SkColorFilter*>(colorFilterHandle);
    auto* inputFilter = reinterpret_cast<const SkImageFilter*>(inputFilterHandle);
    sk_sp<SkImageFilter> colorFilterImageFilter = SkImageFilters::ColorFilter(
            sk_ref_sp(colorFilter), sk_ref_sp(inputFilter), nullptr);
   return reinterpret_cast<jlong>(colorFilterImageFilter.release());
}

static jlong createBlendModeEffect(
    JNIEnv* env,
    jobject,
    jlong backgroundImageFilterHandle,
    jlong foregroundImageFilterHandle,
    jint blendmodeHandle
) {
    auto* backgroundFilter = reinterpret_cast<const SkImageFilter*>(backgroundImageFilterHandle);
    auto* foregroundFilter = reinterpret_cast<const SkImageFilter*>(foregroundImageFilterHandle);
    SkBlendMode blendMode = static_cast<SkBlendMode>(blendmodeHandle);
    sk_sp<SkImageFilter> xfermodeFilter = SkImageFilters::Blend(
            blendMode,
            sk_ref_sp(backgroundFilter),
            sk_ref_sp(foregroundFilter)
    );
    return reinterpret_cast<jlong>(xfermodeFilter.release());
}

static jlong createChainEffect(
    JNIEnv* env,
    jobject,
    jlong outerFilterHandle,
    jlong innerFilterHandle
) {
    auto* outerImageFilter = reinterpret_cast<const SkImageFilter*>(outerFilterHandle);
    auto* innerImageFilter = reinterpret_cast<const SkImageFilter*>(innerFilterHandle);
    sk_sp<SkImageFilter> composeFilter = SkImageFilters::Compose(
        sk_ref_sp(outerImageFilter),
        sk_ref_sp(innerImageFilter)
    );
    return reinterpret_cast<jlong>(composeFilter.release());
}

static void RenderEffect_safeUnref(SkImageFilter* filter) {
    SkSafeUnref(filter);
}

static jlong getRenderEffectFinalizer(JNIEnv*, jobject) {
    return static_cast<jlong>(reinterpret_cast<uintptr_t>(&RenderEffect_safeUnref));
}

static const JNINativeMethod gRenderEffectMethods[] = {
    {"nativeGetFinalizer", "()J", (void*)getRenderEffectFinalizer},
    {"nativeCreateOffsetEffect", "(FFJ)J", (void*)createOffsetEffect},
    {"nativeCreateBlurEffect", "(FFJI)J", (void*)createBlurEffect},
    {"nativeCreateBitmapEffect", "(JFFFFFFFF)J", (void*)createBitmapEffect},
    {"nativeCreateColorFilterEffect", "(JJ)J", (void*)createColorFilterEffect},
    {"nativeCreateBlendModeEffect", "(JJI)J", (void*)createBlendModeEffect},
    {"nativeCreateChainEffect", "(JJ)J", (void*)createChainEffect}
};

int register_android_graphics_RenderEffect(JNIEnv* env) {
    android::RegisterMethodsOrDie(env, "android/graphics/RenderEffect",
            gRenderEffectMethods, NELEM(gRenderEffectMethods));
    return 0;
}