/*

 Copyright (c) 2015, Runa Media LLC
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#include "jni.h"

#include <android/log.h>

#include "SipControllerCore.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "talk/app/webrtc/androidvideocapturer.h"
#include "webrtc/modules/video_capture/video_capture_internal.h"
#include "webrtc/modules/video_render/video_render_internal.h"

using namespace rtcsip;

class SipControllerHandler;

static JavaVM *g_jvm = NULL;
static jobject g_sipControllerJava;
static jclass g_registrationEventEnum;
static jclass g_callEventEnum;
static jclass g_errorTypeEnum;
static SipControllerCore *g_sipControllerCore = NULL;
static WebRtcEngine *g_webRtcEngine = NULL;
static SipControllerHandler *g_sipControllerHandler = NULL;
static webrtc::VideoRendererInterface *g_localRenderer = NULL;
static webrtc::VideoRendererInterface *g_remoteRenderer = NULL;

extern "C" {
JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_init
    (JNIEnv *env, jobject obj, jobject context, jobject j_settings);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setHasAudio
    (JNIEnv *, jobject, jboolean hasAudio);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setHasVideo
    (JNIEnv *, jobject, jboolean hasVideo);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_registerUser
    (JNIEnv *env, jobject, jstring j_username, jstring j_password);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_unregisterUser
    (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_makeCall
    (JNIEnv *env, jobject, jstring j_sipUri);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_answer
    (JNIEnv *, jobject);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_endCall
    (JNIEnv *env, jobject, jboolean destroyLocalStream);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setLocalView
    (JNIEnv *, jobject, jlong j_renderer_pointer);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setRemoteView
    (JNIEnv *, jobject, jlong j_renderer_pointer);

JNIEXPORT jlong JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_nativeWrapVideoRenderer
        (JNIEnv *jni, jobject, jobject j_callbacks);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_freeWrappedVideoRenderer
        (JNIEnv*, jclass, jlong j_p);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_nativeCopyPlane(
        JNIEnv *jni, jclass, jobject j_src_buffer, jint width, jint height,
        jint src_stride, jobject j_dst_buffer, jint dst_stride);
}

class SipControllerHandler : public SipRegistrationHandler, public SipCallHandler, public SipLogHandler,
                             public SipErrorHandler
{
public:
    virtual void handleRegistration(SipRegistrationEvent event, std::string user)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        jclass j_sipControllerClass = env->GetObjectClass(g_sipControllerJava);

        jmethodID j_midOnRegistration = env->GetMethodID(j_sipControllerClass, "onRegistration",
                                                         "(Lcom/runamedia/rtc/demoapp/SipController$RegistrationEvent;Ljava/lang/String;)V");
        jfieldID j_fidRegistrationEvent;
        if (event == SipRegistrationEvent::Registered)
            j_fidRegistrationEvent = env->GetStaticFieldID(g_registrationEventEnum, "REGISTERED", "Lcom/runamedia/rtc/demoapp/SipController$RegistrationEvent;");
        else if (event == SipRegistrationEvent::NotRegistered)
            j_fidRegistrationEvent = env->GetStaticFieldID(g_registrationEventEnum, "NOT_REGISTERED", "Lcom/runamedia/rtc/demoapp/SipController$RegistrationEvent;");
        else
            return;

        jobject j_registrationEvent = env->GetStaticObjectField(g_registrationEventEnum, j_fidRegistrationEvent);

        jstring j_user = env->NewStringUTF(user.c_str());

        env->CallVoidMethod(g_sipControllerJava, j_midOnRegistration, j_registrationEvent, j_user);

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }

    virtual void handleCall(SipCallEvent event, std::string user)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        jclass j_sipControllerClass = env->GetObjectClass(g_sipControllerJava);

        jmethodID j_midOnCall = env->GetMethodID(j_sipControllerClass, "onCall",
                                                 "(Lcom/runamedia/rtc/demoapp/SipController$CallEvent;Ljava/lang/String;)V");
        jfieldID j_fidCallEvent;
        if (event == SipCallEvent::IncomingCall)
            j_fidCallEvent = env->GetStaticFieldID(g_callEventEnum, "INCOMING_CALL", "Lcom/runamedia/rtc/demoapp/SipController$CallEvent;");
        else if (event == SipCallEvent::TerminateCall)
            j_fidCallEvent = env->GetStaticFieldID(g_callEventEnum, "TERMINATE_CALL", "Lcom/runamedia/rtc/demoapp/SipController$CallEvent;");
        else if (event == SipCallEvent::CallAccepted)
            j_fidCallEvent = env->GetStaticFieldID(g_callEventEnum, "CALL_ACCEPTED", "Lcom/runamedia/rtc/demoapp/SipController$CallEvent;");
        else
            return;

        jobject j_callEvent = env->GetStaticObjectField(g_callEventEnum, j_fidCallEvent);

        jstring j_user = env->NewStringUTF(user.c_str());

        env->CallVoidMethod(g_sipControllerJava, j_midOnCall, j_callEvent, j_user);

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }

    virtual void handleLog(std::string log)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        jclass j_sipControllerClass = env->GetObjectClass(g_sipControllerJava);

        jmethodID j_midOnLog = env->GetMethodID(j_sipControllerClass, "onLog", "(Ljava/lang/String;)V");

        jstring j_log = env->NewStringUTF(log.c_str());

        env->CallVoidMethod(g_sipControllerJava, j_midOnLog, j_log);

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }

    virtual void handleError(SipErrorType type, std::string error)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        jclass j_sipControllerClass = env->GetObjectClass(g_sipControllerJava);

        jmethodID j_midOnError = env->GetMethodID(j_sipControllerClass, "onError",
                                                  "(Lcom/runamedia/rtc/demoapp/SipController$ErrorType;Ljava/lang/String;)V");
        jfieldID j_fidErrorType;
        if (type == SipErrorType::WebRtcError)
            j_fidErrorType = env->GetStaticFieldID(g_errorTypeEnum, "WEBRTC_ERROR", "Lcom/runamedia/rtc/demoapp/SipController$ErrorType;");
        else if (type == SipErrorType::SipConnectionError)
            j_fidErrorType = env->GetStaticFieldID(g_errorTypeEnum, "SIP_CONNECTION_ERROR", "Lom/runamedia/rtc/demoapp/SipController$ErrorType;");
        else if (type == SipErrorType::SipSessionError)
            j_fidErrorType = env->GetStaticFieldID(g_errorTypeEnum, "SIP_SESSION_ERROR", "Lom/runamedia/rtc/demoapp/SipController$ErrorType;");
        else
            return;

        jobject j_errorType = env->GetStaticObjectField(g_errorTypeEnum, j_fidErrorType);

        jstring j_error = env->NewStringUTF(error.c_str());

        env->CallVoidMethod(g_sipControllerJava, j_midOnError, j_errorType, j_error);

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }
};

class ScopedLocalRefFrame {
public:
    explicit ScopedLocalRefFrame(JNIEnv* jni) : jni_(jni) {
        jni_->PushLocalFrame(0);
    }
    ~ScopedLocalRefFrame() {
        jni_->PopLocalFrame(NULL);
    }

private:
    JNIEnv* jni_;
};

template<class T>
class ScopedGlobalRef {
public:
    ScopedGlobalRef(JNIEnv* jni, T obj)
            : obj_(static_cast<T>(jni->NewGlobalRef(obj))) {}
    ~ScopedGlobalRef() {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        env->DeleteGlobalRef(obj_);

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }
    T operator*() const {
        return obj_;
    }
private:
    T obj_;
};

class NativeHandleImpl {
public:
    NativeHandleImpl() : texture_object_(NULL), texture_id_(-1) {}

    void* GetHandle() {
        return texture_object_;
    }
    int GetTextureId() {
        return texture_id_;
    }
    void SetTextureObject(void *texture_object, int texture_id) {
        texture_object_ = reinterpret_cast<jobject>(texture_object);
        texture_id_ = texture_id;
    }

private:
    jobject texture_object_;
    int32_t texture_id_;
};

class JavaVideoRendererWrapper : public webrtc::VideoRendererInterface
{
public:
    JavaVideoRendererWrapper(JNIEnv* jni, jobject j_callbacks)
        : j_callbacks_(jni, j_callbacks),
          j_render_frame_id_(jni->GetMethodID(
                  jni->GetObjectClass(j_callbacks), "renderFrame",
                  "(Lcom/runamedia/rtc/demoapp/VideoRenderer$I420Frame;)V")),
          j_frame_class_(jni,
                         jni->FindClass("com/runamedia/rtc/demoapp/VideoRenderer$I420Frame")),
          j_i420_frame_ctor_id_(jni->GetMethodID(
                  *j_frame_class_, "<init>", "(III[I[Ljava/nio/ByteBuffer;)V")),
          j_texture_frame_ctor_id_(jni->GetMethodID(
                  *j_frame_class_, "<init>",
                  "(IIILjava/lang/Object;I)V")),
          j_byte_buffer_class_(jni, jni->FindClass("java/nio/ByteBuffer"))
    { }

    virtual ~JavaVideoRendererWrapper() {}

    virtual void RenderFrame(const cricket::VideoFrame* video_frame) override
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        {
            ScopedLocalRefFrame local_ref_frame(env);
            jobject j_frame = (video_frame->GetNativeHandle() != nullptr)
                              ? CricketToJavaTextureFrame(video_frame)
                              : CricketToJavaI420Frame(video_frame);
            env->CallVoidMethod(*j_callbacks_, j_render_frame_id_, j_frame);
        }

        if (isAttached)
            g_jvm->DetachCurrentThread();
    }

    virtual bool CanApplyRotation() override { return true; }

private:
    jobject CricketToJavaI420Frame(const cricket::VideoFrame* frame)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        jintArray strides = env->NewIntArray(3);
        jint* strides_array = env->GetIntArrayElements(strides, NULL);
        strides_array[0] = frame->GetYPitch();
        strides_array[1] = frame->GetUPitch();
        strides_array[2] = frame->GetVPitch();
        env->ReleaseIntArrayElements(strides, strides_array, 0);
        jobjectArray planes = env->NewObjectArray(3, *j_byte_buffer_class_, NULL);
        jobject y_buffer = env->NewDirectByteBuffer(
                const_cast<uint8*>(frame->GetYPlane()),
                frame->GetYPitch() * frame->GetHeight());
        jobject u_buffer = env->NewDirectByteBuffer(
                const_cast<uint8*>(frame->GetUPlane()), frame->GetChromaSize());
        jobject v_buffer = env->NewDirectByteBuffer(
                const_cast<uint8*>(frame->GetVPlane()), frame->GetChromaSize());
        env->SetObjectArrayElement(planes, 0, y_buffer);
        env->SetObjectArrayElement(planes, 1, u_buffer);
        env->SetObjectArrayElement(planes, 2, v_buffer);
        jobject i420_frame = env->NewObject(
                *j_frame_class_, j_i420_frame_ctor_id_,
                frame->GetWidth(), frame->GetHeight(),
                static_cast<int>(frame->GetVideoRotation()),
                strides, planes);

        if (isAttached)
            g_jvm->DetachCurrentThread();

        return i420_frame;
    }

    jobject CricketToJavaTextureFrame(const cricket::VideoFrame* frame)
    {
        JNIEnv *env;
        bool isAttached = false;

        if (g_jvm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK)
        {
            g_jvm->AttachCurrentThread(&env, NULL);
            isAttached = true;
        }

        NativeHandleImpl* handle =
                reinterpret_cast<NativeHandleImpl*>(frame->GetNativeHandle());
        jobject texture_object = reinterpret_cast<jobject>(handle->GetHandle());
        int texture_id = handle->GetTextureId();
        jobject texture_frame = env->NewObject(
                *j_frame_class_, j_texture_frame_ctor_id_,
                frame->GetWidth(), frame->GetHeight(),
                static_cast<int>(frame->GetVideoRotation()),
                texture_object, texture_id);

        if (isAttached)
            g_jvm->DetachCurrentThread();

        return texture_frame;
    }

    ScopedGlobalRef<jobject> j_callbacks_;
    jmethodID j_render_frame_id_;
    ScopedGlobalRef<jclass> j_frame_class_;
    jmethodID j_i420_frame_ctor_id_;
    jmethodID j_texture_frame_ctor_id_;
    ScopedGlobalRef<jclass> j_byte_buffer_class_;
};

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_init
        (JNIEnv *env, jobject obj, jobject context, jobject j_settings)
{
    g_sipControllerJava = env->NewGlobalRef(obj);

    webrtc::VoiceEngine::SetAndroidObjects(g_jvm, context);
    webrtc::SetRenderAndroidVM(g_jvm);
    webrtc::SetCaptureAndroidVM(g_jvm, context);

    SipServerSettings serverSettings;

    jclass j_serverSettingsClass = env->GetObjectClass(j_settings);

    jfieldID j_fidDomain = env->GetFieldID(j_serverSettingsClass, "domain", "Ljava/lang/String;");

    jstring j_domain = static_cast<jstring>(env->GetObjectField(j_settings, j_fidDomain));

    const char *domain = env->GetStringUTFChars(j_domain, NULL);

    serverSettings.domain = domain;

    env->ReleaseStringUTFChars(j_domain, domain);

    g_webRtcEngine = new WebRtcEngine();

    g_sipControllerCore = new SipControllerCore(serverSettings, g_webRtcEngine);
    g_sipControllerHandler = new SipControllerHandler();
    g_sipControllerCore->registerRegistrationHandler(g_sipControllerHandler);
    g_sipControllerCore->registerCallHandler(g_sipControllerHandler);
    g_sipControllerCore->registerLogHandler(g_sipControllerHandler);
    g_sipControllerCore->registerErrorHandler(g_sipControllerHandler);

}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setHasAudio
        (JNIEnv *, jobject, jboolean hasAudio)
{
    g_webRtcEngine->setHasAudio(hasAudio);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setHasVideo
        (JNIEnv *, jobject, jboolean hasVideo)
{
    g_webRtcEngine->setHasVideo(hasVideo);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_registerUser
        (JNIEnv *env, jobject, jstring j_username, jstring j_password)
{
    const char *username = env->GetStringUTFChars(j_username, NULL);
    const char *password = env->GetStringUTFChars(j_password, NULL);

    g_sipControllerCore->registerUser(username, password);

    env->ReleaseStringUTFChars(j_username, username);
    env->ReleaseStringUTFChars(j_password, password);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_unregisterUser
        (JNIEnv *, jobject)
{
    g_sipControllerCore->unregisterUser();
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_makeCall
        (JNIEnv *env, jobject, jstring j_sipUri)
{
    g_webRtcEngine->setLocalRenderer(g_localRenderer);
    g_webRtcEngine->setRemoteRenderer(g_remoteRenderer);

    const char *sipUri = env->GetStringUTFChars(j_sipUri, NULL);

    g_sipControllerCore->createSession(sipUri);

    env->ReleaseStringUTFChars(j_sipUri, sipUri);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_answer
        (JNIEnv *, jobject)
{
    g_webRtcEngine->setLocalRenderer(g_localRenderer);
    g_webRtcEngine->setRemoteRenderer(g_remoteRenderer);

    g_sipControllerCore->acceptSession();
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_endCall
        (JNIEnv *env, jobject, jboolean destroyLocalStream)
{
    g_sipControllerCore->terminateSession(destroyLocalStream);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setLocalView
        (JNIEnv *, jobject, jlong j_renderer_pointer)
{
    g_localRenderer = reinterpret_cast<webrtc::VideoRendererInterface*>(j_renderer_pointer);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setRemoteView
        (JNIEnv *, jobject, jlong j_renderer_pointer)
{
    g_remoteRenderer = reinterpret_cast<webrtc::VideoRendererInterface*>(j_renderer_pointer);
}

JNIEXPORT jlong JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_nativeWrapVideoRenderer
        (JNIEnv *jni, jobject, jobject j_callbacks)
{
    rtc::scoped_ptr<JavaVideoRendererWrapper> renderer(
            new JavaVideoRendererWrapper(jni, j_callbacks));
    return (jlong)renderer.release();
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_freeWrappedVideoRenderer
        (JNIEnv*, jclass, jlong j_p)
{
    delete reinterpret_cast<JavaVideoRendererWrapper*>(j_p);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_VideoRenderer_nativeCopyPlane(
        JNIEnv *jni, jclass, jobject j_src_buffer, jint width, jint height,
        jint src_stride, jobject j_dst_buffer, jint dst_stride)
{
    size_t src_size = jni->GetDirectBufferCapacity(j_src_buffer);
    size_t dst_size = jni->GetDirectBufferCapacity(j_dst_buffer);
    CHECK(src_stride >= width) << "Wrong source stride " << src_stride;
    CHECK(dst_stride >= width) << "Wrong destination stride " << dst_stride;
    CHECK(src_size >= src_stride * height)
    << "Insufficient source buffer capacity " << src_size;
    CHECK(dst_size >= dst_stride * height)
    << "Isufficient destination buffer capacity " << dst_size;
    uint8_t *src =
            reinterpret_cast<uint8_t*>(jni->GetDirectBufferAddress(j_src_buffer));
    uint8_t *dst =
            reinterpret_cast<uint8_t*>(jni->GetDirectBufferAddress(j_dst_buffer));
    if (src_stride == dst_stride) {
        memcpy(dst, src, src_stride * height);
    } else {
        for (int i = 0; i < height; i++) {
            memcpy(dst, src, width);
            src += src_stride;
            dst += dst_stride;
        }
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "resiprocate_jni", "JNI_OnLoad");

    g_jvm = jvm;

    JNIEnv* env;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
        return -1;

    jclass j_registrationEventEnum = env->FindClass("com/runamedia/rtc/demoapp/SipController$RegistrationEvent");
    g_registrationEventEnum = reinterpret_cast<jclass>(env->NewGlobalRef(j_registrationEventEnum));

    jclass j_callEventEnum = env->FindClass("com/runamedia/rtc/demoapp/SipController$CallEvent");
    g_callEventEnum = reinterpret_cast<jclass>(env->NewGlobalRef(j_callEventEnum));

    jclass j_errorTypeEnum = env->FindClass("com/runamedia/rtc/demoapp/SipController$ErrorType");
    g_errorTypeEnum = reinterpret_cast<jclass>(env->NewGlobalRef(j_errorTypeEnum));

    return JNI_VERSION_1_6;
}
