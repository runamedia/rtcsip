# Copyright (c) 2015, Runa Media LLC
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies,
# either expressed or implied, of the FreeBSD Project.

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := resipares
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libresipares.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rutil
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := resip
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libresip.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := dum
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libdum.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_coding_module
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_coding_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_conference_mixer
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_conference_mixer.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_decoder_interface
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_decoder_interface.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_device
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_device.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_encoder_interface
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_encoder_interface.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_processing
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_processing.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audio_processing_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudio_processing_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := audioproc_debug_proto
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libaudioproc_debug_proto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := bitrate_controller
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libbitrate_controller.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := boringssl
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libboringssl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cng
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := common_audio
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcommon_audio.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := common_audio_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcommon_audio_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := common_video
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcommon_video.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cpu_features
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcpu_features.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := cpu_features_android
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libcpu_features_android.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := field_trial_default
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libfield_trial_default.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := g711
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libg711.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := g722
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libg722.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ilbc
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libilbc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := isac
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libisac.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := isac_common
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libisac_common.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := isac_fix
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libisac_fix.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := isac_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libisac_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jingle_media
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libjingle_media.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jingle_p2p
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libjingle_p2p.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jingle_peerconnection
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libjingle_peerconnection.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jpeg_turbo
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libjpeg_turbo.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := jsoncpp
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libjsoncpp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := media_file
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libmedia_file.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := metrics_default
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libmetrics_default.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := neteq
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libneteq.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := openmax_dl
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libopenmax_dl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := openmax_dl_armv7
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libopenmax_dl_armv7.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := openmax_dl_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libopenmax_dl_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opus
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := paced_sender
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libpaced_sender.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := pcm16b
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libpcm16b.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := protobuf_lite
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libprotobuf_lite.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := red
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libred.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := remote_bitrate_estimator
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libremote_bitrate_estimator.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rent_a_codec
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librent_a_codec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtc_base
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtc_base.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtc_base_approved
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtc_base_approved.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtc_event_log
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtc_event_log.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtc_event_log_proto
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtc_event_log_proto.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtc_p2p
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtc_p2p.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rtp_rtcp
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/librtp_rtcp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := srtp
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libsrtp.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := system_wrappers
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libsystem_wrappers.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := usrsctplib
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libusrsctplib.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_capture_module
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_capture_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_coding_utility
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_coding_utility.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_processing
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_processing.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_processing_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_processing_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_render_module
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_render_module.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := video_render_module_internal_impl
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvideo_render_module_internal_impl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := voice_engine
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvoice_engine.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vpx
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvpx_new.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := vpx_intrinsics_neon
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libvpx_intrinsics_neon.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_common
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_common.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_h264
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_h264.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_i420
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_i420.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_jni
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_jni.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_opus
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_opus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_utility
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_utility.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_video_coding
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_video_coding.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_vp8
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_vp8.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := webrtc_vp9
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libwebrtc_vp9.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := yuv
LOCAL_SRC_FILES := ../jniLibs/$(TARGET_ARCH_ABI)/libyuv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := rtcsip_jni

LOCAL_WHOLE_STATIC_LIBRARIES := resipares \
    rutil \
    resip \
    dum \
    audio_coding_module \
    audio_conference_mixer \
    audio_decoder_interface \
    audio_device \
    audio_encoder_interface \
    audio_processing \
    audio_processing_neon \
    audioproc_debug_proto \
    bitrate_controller \
    boringssl \
    cng \
    common_audio \
    common_audio_neon \
    common_video \
    cpu_features \
    cpu_features_android \
    field_trial_default \
    g711 \
    g722 \
    ilbc \
    isac \
    isac_common \
    isac_fix \
    isac_neon \
    jingle_media \
    jingle_p2p \
    jingle_peerconnection \
    jpeg_turbo \
    jsoncpp \
    media_file \
    metrics_default \
    neteq \
    openmax_dl \
    openmax_dl_armv7 \
    openmax_dl_neon \
    opus \
    paced_sender \
    pcm16b \
    protobuf_lite \
    red \
    remote_bitrate_estimator \
    rent_a_codec \
    rtc_base \
    rtc_base_approved \
    rtc_event_log \
    rtc_event_log_proto \
    rtc_p2p \
    rtp_rtcp \
    srtp \
    system_wrappers \
    usrsctplib \
    video_capture_module \
    video_coding_utility \
    video_processing \
    video_processing_neon \
    video_render_module \
    video_render_module_internal_impl \
    voice_engine \
    vpx \
    vpx_intrinsics_neon \
    webrtc \
    webrtc_common \
    webrtc_h264 \
    webrtc_i420 \
    webrtc_jni \
    webrtc_opus \
    webrtc_utility \
    webrtc_video_coding \
    webrtc_vp8 \
    webrtc_vp9 \
    yuv

LOCAL_CPP_FEATURES := rtti exceptions

LOCAL_CFLAGS := -DWEBRTC_POSIX -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -UNDEBUG

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../../../source/thirdParty/resiprocate \
    $(LOCAL_PATH)/../../../../../../../source/thirdParty/webrtc_android/src

LOCAL_LDLIBS := -llog -landroid -lGLESv2 -lOpenSLES

LOCAL_SRC_FILES := ./rtcsip_jni.cpp \
    ./SipControllerCore.cpp \
    ./WebRtcEngine.cpp

include $(BUILD_SHARED_LIBRARY)
