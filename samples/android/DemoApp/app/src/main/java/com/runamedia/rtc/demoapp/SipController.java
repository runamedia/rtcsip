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

package com.runamedia.rtc.demoapp;

import android.content.Context;
import android.util.Log;

import com.runamedia.rtc.demoapp.ServerSettings;

import org.webrtc.MediaConstraints;

public class SipController {

    public enum RegistrationEvent {
        REGISTERED,
        NOT_REGISTERED
    }

    public enum CallEvent {
        INCOMING_CALL,
        TERMINATE_CALL,
        CALL_ACCEPTED
    }

    public enum ErrorType {
        WEBRTC_ERROR,
        SIP_CONNECTION_ERROR,
        SIP_SESSION_ERROR
    }

    public static interface OnRegistrationEventListener {
        void onRegistrationEvent(RegistrationEvent event, String user);
    }

    public static interface OnCallEventListener {
        void onCallEvent(CallEvent event, String user);
    }

    public static interface OnLogEventListener {
        void onLogEvent(String log);
    }

    public static interface OnErrorEventListener {
        void onErrorEvent(ErrorType type, String message);
    }

    private static final String TAG = "SipController";

    static {
        try {
            System.loadLibrary("rtcsip_jni");
        } catch (UnsatisfiedLinkError exc) {
            Log.e(TAG, "rtcsip_jni library not found");
        }
    }

    private OnRegistrationEventListener onRegistrationEventListener;
    private OnCallEventListener onCallEventListener;
    private OnLogEventListener onLogEventListener;
    private OnErrorEventListener onErrorEventListener;

    public synchronized void onRegistration(RegistrationEvent event, String user) {
        Log.d(TAG, "onRegistration");
        if (onRegistrationEventListener != null)
            onRegistrationEventListener.onRegistrationEvent(event, user);
    }

    public synchronized void onCall(CallEvent event, String user) {
        Log.d(TAG, "onCall");
        if (onCallEventListener != null)
            onCallEventListener.onCallEvent(event, user);
    }

    public synchronized void onLog(String log) {
        Log.d(TAG, "onLog: " + log);
        if (onLogEventListener != null)
            onLogEventListener.onLogEvent(log);
    }

    public synchronized void onError(ErrorType type, String error) {
        Log.d(TAG, "onError: " + error);
        if (onErrorEventListener != null)
            onErrorEventListener.onErrorEvent(type, error);
    }

    public synchronized void registerOnRegistrationEventListener(OnRegistrationEventListener listener) {
        onRegistrationEventListener = listener;
    }

    public synchronized void registerOnCallEventListener(OnCallEventListener listener) {
        onCallEventListener = listener;
    }

    public synchronized void registerOnLogEventListener(OnLogEventListener listener) {
        onLogEventListener = listener;
    }

    public synchronized void registerOnErrorEventListener(OnErrorEventListener listener) {
        onErrorEventListener = listener;
    }

    public native void init(Context context, ServerSettings serverSettings);
    public native void setHasAudio(boolean hasAudio);
    public native void setHasVideo(boolean hasVideo);
    public native void registerUser(String username, String password);
    public native void unregisterUser();
    public native void makeCall(String sipUri);
    public native void answer();
    public native void endCall(boolean destroyLocalStream);
    public native void setVideoCapturer(long capturer, MediaConstraints captureConstraints);
    public native void setLocalView(long renderer);
    public native void setRemoteView(long renderer);
}
