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
#include "talk/media/base/videorenderer.h"
#include "talk/media/devices/videorendererfactory.h"
#include "talk/app/webrtc/androidvideocapturer.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "talk/app/webrtc/java/jni/classreferenceholder.h"
#include "talk/app/webrtc/java/jni/jni_helpers.h"
#include "talk/app/webrtc/java/jni/native_handle_impl.h"
#include "talk/app/webrtc/java/jni/androidvideocapturer_jni.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/modules/video_render/video_render_internal.h"

using namespace rtcsip;
using namespace webrtc_jni;

class SipControllerHandler;

static JavaVM *g_jvm = NULL;
static jobject g_sipControllerJava;
static jclass g_registrationEventEnum;
static jclass g_callEventEnum;
static jclass g_errorTypeEnum;
static SipControllerCore *g_sipControllerCore = NULL;
static WebRtcEngine *g_webRtcEngine = NULL;
static SipControllerHandler *g_sipControllerHandler = NULL;
static cricket::VideoCapturer *g_capturer = NULL;
static webrtc::MediaConstraintsInterface *g_captureConstraints = NULL;
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

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setVideoCapturer
    (JNIEnv *, jobject, jlong j_capturer_pointer, jobject j_capture_constraints);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setLocalView
    (JNIEnv *, jobject, jlong j_renderer_pointer);

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setRemoteView
    (JNIEnv *, jobject, jlong j_renderer_pointer);

JNIEXPORT jlong JNICALL Java_org_webrtc_VideoCapturerAndroid_nativeCreateVideoCapturer
        (JNIEnv* jni, jclass,
         jobject j_video_capturer, jobject j_surface_texture_helper);

JNIEXPORT jobject JNICALL Java_org_webrtc_VideoCapturer_nativeCreateVideoCapturer
    (JNIEnv* jni, jclass, jstring j_device_name);

JNIEXPORT void JNICALL Java_org_webrtc_VideoCapturer_free
    (JNIEnv*, jclass, jlong j_p);

JNIEXPORT jlong JNICALL Java_org_webrtc_VideoRenderer_nativeCreateGuiVideoRenderer
    (JNIEnv* jni, jclass, int x, int y);

JNIEXPORT jlong JNICALL Java_org_webrtc_VideoRenderer_nativeWrapVideoRenderer
    (JNIEnv* jni, jclass, jobject j_callbacks);

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_nativeCopyPlane
    (JNIEnv *jni, jclass, jobject j_src_buffer, jint width, jint height,
    jint src_stride, jobject j_dst_buffer, jint dst_stride);

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_freeGuiVideoRenderer
    (JNIEnv*, jclass, jlong j_p);

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_freeWrappedVideoRenderer
    (JNIEnv*, jclass, jlong j_p);

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_releaseNativeFrame
    (JNIEnv* jni, jclass, jlong j_frame_ptr);
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

// Wrapper for a Java MediaConstraints object.  Copies all needed data so when
// the constructor returns the Java object is no longer needed.
class ConstraintsWrapper : public webrtc::MediaConstraintsInterface {
public:
    ConstraintsWrapper(JNIEnv* jni, jobject j_constraints) {
        PopulateConstraintsFromJavaPairList(
              jni, j_constraints, "mandatory", &mandatory_);
        PopulateConstraintsFromJavaPairList(
              jni, j_constraints, "optional", &optional_);
    }

    virtual ~ConstraintsWrapper() {}

    // MediaConstraintsInterface.
    const Constraints& GetMandatory() const override { return mandatory_; }

    const Constraints& GetOptional() const override { return optional_; }

private:
    // Helper for translating a List<Pair<String, String>> to a Constraints.
    static void PopulateConstraintsFromJavaPairList(
            JNIEnv* jni, jobject j_constraints,
            const char* field_name, Constraints* field) {
        jfieldID j_id = GetFieldID(jni,
              GetObjectClass(jni, j_constraints), field_name, "Ljava/util/List;");
        jobject j_list = GetObjectField(jni, j_constraints, j_id);
        jmethodID j_iterator_id = GetMethodID(jni,
              GetObjectClass(jni, j_list), "iterator", "()Ljava/util/Iterator;");
        jobject j_iterator = jni->CallObjectMethod(j_list, j_iterator_id);
        CHECK_EXCEPTION(jni) << "error during CallObjectMethod";
        jmethodID j_has_next = GetMethodID(jni,
        GetObjectClass(jni, j_iterator), "hasNext", "()Z");
        jmethodID j_next = GetMethodID(jni,
              GetObjectClass(jni, j_iterator), "next", "()Ljava/lang/Object;");
        while (jni->CallBooleanMethod(j_iterator, j_has_next)) {
            CHECK_EXCEPTION(jni) << "error during CallBooleanMethod";
            jobject entry = jni->CallObjectMethod(j_iterator, j_next);
            CHECK_EXCEPTION(jni) << "error during CallObjectMethod";
            jmethodID get_key = GetMethodID(jni,
                    GetObjectClass(jni, entry), "getKey", "()Ljava/lang/String;");
            jstring j_key = reinterpret_cast<jstring>(
                    jni->CallObjectMethod(entry, get_key));
            CHECK_EXCEPTION(jni) << "error during CallObjectMethod";
            jmethodID get_value = GetMethodID(jni,
                    GetObjectClass(jni, entry), "getValue", "()Ljava/lang/String;");
            jstring j_value = reinterpret_cast<jstring>(
                    jni->CallObjectMethod(entry, get_value));
            CHECK_EXCEPTION(jni) << "error during CallObjectMethod";
            field->push_back(Constraint(JavaToStdString(jni, j_key),
                                        JavaToStdString(jni, j_value)));
        }
        CHECK_EXCEPTION(jni) << "error during CallBooleanMethod";
    }

    Constraints mandatory_;
    Constraints optional_;
};

// Adapter presenting a cricket::VideoRenderer as a
// webrtc::VideoRendererInterface.
class VideoRendererWrapper : public webrtc::VideoRendererInterface {
public:
    static VideoRendererWrapper* Create(cricket::VideoRenderer* renderer) {
        if (renderer)
            return new VideoRendererWrapper(renderer);
        return NULL;
    }

    virtual ~VideoRendererWrapper() {}

    // This wraps VideoRenderer which still has SetSize.
    void RenderFrame(const cricket::VideoFrame* video_frame) override {
        ScopedLocalRefFrame local_ref_frame(AttachCurrentThreadIfNeeded());
        const cricket::VideoFrame* frame =
            video_frame->GetCopyWithRotationApplied();
        if (width_ != frame->GetWidth() || height_ != frame->GetHeight()) {
            width_ = frame->GetWidth();
            height_ = frame->GetHeight();
            renderer_->SetSize(width_, height_, 0);
        }
        renderer_->RenderFrame(frame);
    }

private:
    explicit VideoRendererWrapper(cricket::VideoRenderer* renderer)
        : width_(0), height_(0), renderer_(renderer) {}
    int width_, height_;
    rtc::scoped_ptr<cricket::VideoRenderer> renderer_;
};

// Wrapper dispatching webrtc::VideoRendererInterface to a Java VideoRenderer
// instance.
class JavaVideoRendererWrapper : public webrtc::VideoRendererInterface {
public:
    JavaVideoRendererWrapper(JNIEnv* jni, jobject j_callbacks)
        : j_callbacks_(jni, j_callbacks),
            j_render_frame_id_(GetMethodID(
                jni, GetObjectClass(jni, j_callbacks), "renderFrame",
                "(Lorg/webrtc/VideoRenderer$I420Frame;)V")),
            j_frame_class_(jni,
                           jni->FindClass("org/webrtc/VideoRenderer$I420Frame")),
            j_i420_frame_ctor_id_(GetMethodID(
                    jni, *j_frame_class_, "<init>", "(III[I[Ljava/nio/ByteBuffer;J)V")),
            j_texture_frame_ctor_id_(GetMethodID(
	                jni, *j_frame_class_, "<init>",
	                "(IIII[FJ)V")),
            j_byte_buffer_class_(jni, jni->FindClass("java/nio/ByteBuffer")) {
        CHECK_EXCEPTION(jni);
    }

    virtual ~JavaVideoRendererWrapper() {}

    void RenderFrame(const cricket::VideoFrame* video_frame) override {
        ScopedLocalRefFrame local_ref_frame(jni());
        jobject j_frame = (video_frame->GetNativeHandle() != nullptr)
                              ? CricketToJavaTextureFrame(video_frame)
                              : CricketToJavaI420Frame(video_frame);
        // |j_callbacks_| is responsible for releasing |j_frame| with
        // VideoRenderer.renderFrameDone().
        jni()->CallVoidMethod(*j_callbacks_, j_render_frame_id_, j_frame);
        CHECK_EXCEPTION(jni());
    }

private:
    // Make a shallow copy of |frame| to be used with Java. The callee has
    // ownership of the frame, and the frame should be released with
    // VideoRenderer.releaseNativeFrame().
    static jlong javaShallowCopy(const cricket::VideoFrame* frame) {
        return jlongFromPointer(frame->Copy());
    }

    // Return a VideoRenderer.I420Frame referring to the data in |frame|.
    jobject CricketToJavaI420Frame(const cricket::VideoFrame* frame) {
        jintArray strides = jni()->NewIntArray(3);
        jint* strides_array = jni()->GetIntArrayElements(strides, NULL);
        strides_array[0] = frame->GetYPitch();
        strides_array[1] = frame->GetUPitch();
        strides_array[2] = frame->GetVPitch();
        jni()->ReleaseIntArrayElements(strides, strides_array, 0);
        jobjectArray planes = jni()->NewObjectArray(3, *j_byte_buffer_class_, NULL);
        jobject y_buffer =
            jni()->NewDirectByteBuffer(const_cast<uint8_t*>(frame->GetYPlane()),
                                       frame->GetYPitch() * frame->GetHeight());
        jobject u_buffer = jni()->NewDirectByteBuffer(
            const_cast<uint8_t*>(frame->GetUPlane()), frame->GetChromaSize());
        jobject v_buffer = jni()->NewDirectByteBuffer(
            const_cast<uint8_t*>(frame->GetVPlane()), frame->GetChromaSize());
        jni()->SetObjectArrayElement(planes, 0, y_buffer);
        jni()->SetObjectArrayElement(planes, 1, u_buffer);
        jni()->SetObjectArrayElement(planes, 2, v_buffer);
        return jni()->NewObject(
            *j_frame_class_, j_i420_frame_ctor_id_,
            frame->GetWidth(), frame->GetHeight(),
            static_cast<int>(frame->GetVideoRotation()),
            strides, planes, javaShallowCopy(frame));
    }

    // Return a VideoRenderer.I420Frame referring texture object in |frame|.
    jobject CricketToJavaTextureFrame(const cricket::VideoFrame* frame) {
        NativeHandleImpl* handle =
            reinterpret_cast<NativeHandleImpl*>(frame->GetNativeHandle());
        jfloatArray sampling_matrix = jni()->NewFloatArray(16);
        jni()->SetFloatArrayRegion(sampling_matrix, 0, 16, handle->sampling_matrix);
        return jni()->NewObject(
                *j_frame_class_, j_texture_frame_ctor_id_,
                frame->GetWidth(), frame->GetHeight(),
                static_cast<int>(frame->GetVideoRotation()),
                handle->oes_texture_id, sampling_matrix, javaShallowCopy(frame));
    }

    JNIEnv* jni() {
        return AttachCurrentThreadIfNeeded();
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
    AndroidVideoCapturerJni::SetAndroidObjects(env, context);

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
    g_webRtcEngine->setVideoCapturer(g_capturer, g_captureConstraints);
    g_webRtcEngine->setLocalRenderer(g_localRenderer);
    g_webRtcEngine->setRemoteRenderer(g_remoteRenderer);

    const char *sipUri = env->GetStringUTFChars(j_sipUri, NULL);

    g_sipControllerCore->createSession(sipUri);

    env->ReleaseStringUTFChars(j_sipUri, sipUri);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_answer
        (JNIEnv *, jobject)
{
    g_webRtcEngine->setVideoCapturer(g_capturer, g_captureConstraints);
    g_webRtcEngine->setLocalRenderer(g_localRenderer);
    g_webRtcEngine->setRemoteRenderer(g_remoteRenderer);

    g_sipControllerCore->acceptSession();
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_endCall
        (JNIEnv *env, jobject, jboolean destroyLocalStream)
{
    g_sipControllerCore->terminateSession(destroyLocalStream);
}

JNIEXPORT void JNICALL Java_com_runamedia_rtc_demoapp_SipController_setVideoCapturer
        (JNIEnv *env, jobject, jlong j_capturer_pointer, jobject j_capture_constraints)
{
    rtc::scoped_ptr<ConstraintsWrapper> constraints(
                new ConstraintsWrapper(env, j_capture_constraints));
    g_capturer = reinterpret_cast<cricket::VideoCapturer*>(j_capturer_pointer);
    g_captureConstraints = constraints.release();
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

JNIEXPORT jobject JNICALL Java_org_webrtc_VideoCapturer_nativeCreateVideoCapturer
        (JNIEnv* jni, jclass, jstring j_device_name)
{
  // Since we can't create platform specific java implementations in Java, we
  // defer the creation to C land.
    jclass j_video_capturer_class(
            jni->FindClass("org/webrtc/VideoCapturerAndroid"));
    const int camera_id = jni->CallStaticIntMethod(
            j_video_capturer_class,
            GetStaticMethodID(jni, j_video_capturer_class, "lookupDeviceName",
                              "(Ljava/lang/String;)I"),
            j_device_name);
    CHECK_EXCEPTION(jni) << "error during VideoCapturerAndroid.lookupDeviceName";
    if (camera_id == -1)
        return nullptr;
    jobject j_video_capturer = jni->NewObject(
            j_video_capturer_class,
            GetMethodID(jni, j_video_capturer_class, "<init>", "(I)V"), camera_id);
    CHECK_EXCEPTION(jni) << "error during creation of VideoCapturerAndroid";
    jfieldID helper_fid = GetFieldID(jni, j_video_capturer_class, "surfaceHelper",
                                     "Lorg/webrtc/SurfaceTextureHelper;");

    rtc::scoped_refptr<webrtc::AndroidVideoCapturerDelegate> delegate =
            new rtc::RefCountedObject<AndroidVideoCapturerJni>(
                    jni, j_video_capturer,
                    GetObjectField(jni, j_video_capturer, helper_fid));
    rtc::scoped_ptr<cricket::VideoCapturer> capturer(
            new webrtc::AndroidVideoCapturer(delegate));

    const jmethodID j_videocapturer_set_native_capturer(GetMethodID(
            jni, j_video_capturer_class, "setNativeCapturer", "(J)V"));
    jni->CallVoidMethod(j_video_capturer,
                        j_videocapturer_set_native_capturer,
                        jlongFromPointer(capturer.release()));
    CHECK_EXCEPTION(jni) << "error during setNativeCapturer";

    return j_video_capturer;
}

JNIEXPORT void JNICALL Java_org_webrtc_VideoCapturer_free
        (JNIEnv*, jclass, jlong j_p)
{
    delete reinterpret_cast<cricket::VideoCapturer*>(j_p);
}

JNIEXPORT jlong JNICALL Java_org_webrtc_VideoRenderer_nativeCreateGuiVideoRenderer
        (JNIEnv* jni, jclass, int x, int y)
{
    rtc::scoped_ptr<VideoRendererWrapper> renderer(VideoRendererWrapper::Create(
            cricket::VideoRendererFactory::CreateGuiVideoRenderer(x, y)));
    return (jlong)renderer.release();
}

JNIEXPORT jlong JNICALL Java_org_webrtc_VideoRenderer_nativeWrapVideoRenderer
        (JNIEnv* jni, jclass, jobject j_callbacks)
{
    rtc::scoped_ptr<JavaVideoRendererWrapper> renderer(
            new JavaVideoRendererWrapper(jni, j_callbacks));
    return (jlong)renderer.release();
}

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_nativeCopyPlane
        (JNIEnv *jni, jclass, jobject j_src_buffer, jint width, jint height,
        jint src_stride, jobject j_dst_buffer, jint dst_stride)
{
    size_t src_size = jni->GetDirectBufferCapacity(j_src_buffer);
    size_t dst_size = jni->GetDirectBufferCapacity(j_dst_buffer);
    RTC_CHECK(src_stride >= width) << "Wrong source stride " << src_stride;
    RTC_CHECK(dst_stride >= width) << "Wrong destination stride " << dst_stride;
    RTC_CHECK(src_size >= src_stride * height)
            << "Insufficient source buffer capacity " << src_size;
    RTC_CHECK(dst_size >= dst_stride * height)
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

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_freeGuiVideoRenderer
        (JNIEnv*, jclass, jlong j_p)
{
    delete reinterpret_cast<VideoRendererWrapper*>(j_p);
}

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_freeWrappedVideoRenderer
        (JNIEnv*, jclass, jlong j_p)
{
    delete reinterpret_cast<JavaVideoRendererWrapper*>(j_p);
}

JNIEXPORT void JNICALL Java_org_webrtc_VideoRenderer_releaseNativeFrame
        (JNIEnv* jni, jclass, jlong j_frame_ptr)
{
    delete reinterpret_cast<const cricket::VideoFrame*>(j_frame_ptr);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "rtcsip_jni", "JNI_OnLoad");

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
	
    jint ret = InitGlobalJniVariables(jvm);
    if (ret < 0)
      return -1;

    LoadGlobalClassReferenceHolder();

    return ret;
}

JNIEXPORT void JNICALL JNI_OnUnLoad(JavaVM *jvm, void *reserved)
{
    __android_log_print(ANDROID_LOG_VERBOSE, "rtcsip_jni", "JNI_OnUnLoad");
    FreeGlobalClassReferenceHolder();
}
