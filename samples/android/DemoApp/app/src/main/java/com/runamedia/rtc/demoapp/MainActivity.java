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
import android.media.AudioManager;
import android.opengl.GLSurfaceView;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.RadioButton;

import org.webrtc.CameraEnumerationAndroid;
import org.webrtc.MediaConstraints;
import org.webrtc.RendererCommon;
import org.webrtc.VideoCapturerAndroid;
import org.webrtc.VideoRenderer;
import org.webrtc.VideoRendererGui;

public class MainActivity extends ActionBarActivity {

    private enum CallState {
        IDLE,
        CONNECTED,
        INCOMING_CALL,
        IN_CALL
    }

    private static final String TAG = "MainActivity";

    private static final String SIP_SERVER_DOMAIN = "developer.runamedia.com";
    private static final String SIP_DNS_SERVER = "8.8.8.8";
    private static final String SIP_PROXY_SERVER = "";

    private SipController sipController = new SipController();

    private MediaConstraints videoCaptureConstraints;

    private GLSurfaceView surfaceView;

    private CallState callState = CallState.IDLE;

    private AudioManager audioManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        surfaceView = new GLSurfaceView(this);
        VideoRendererGui.setView(surfaceView, new Runnable() {
            @Override
            public void run() {

            }
        });

        audioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        videoCaptureConstraints = new MediaConstraints();
        videoCaptureConstraints.mandatory.add(new MediaConstraints.KeyValuePair(
                "maxWidth", Integer.toString(640)));
        videoCaptureConstraints.mandatory.add(new MediaConstraints.KeyValuePair(
                "maxHeight", Integer.toString(480)));
        videoCaptureConstraints.mandatory.add(new MediaConstraints.KeyValuePair(
                "maxFrameRate", Integer.toString(15)));

        final Button affirmativeButton = (Button) findViewById(R.id.affirmativeButton);
        final Button negativeButton = (Button) findViewById(R.id.negativeButton);
        final RadioButton user1001RadioButton = (RadioButton) findViewById(R.id.user1001RadioButton);
        final RadioButton user1002RadioButton = (RadioButton) findViewById(R.id.user1002RadioButton);
        final RadioButton user1003RadioButton = (RadioButton) findViewById(R.id.user1003RadioButton);
        final RadioButton user1004RadioButton = (RadioButton) findViewById(R.id.user1004RadioButton);
        final RadioButton callee1001RadioButton = (RadioButton) findViewById(R.id.callee1001RadioButton);
        final RadioButton callee1002RadioButton = (RadioButton) findViewById(R.id.callee1002RadioButton);
        final RadioButton callee1003RadioButton = (RadioButton) findViewById(R.id.callee1003RadioButton);
        final RadioButton callee1004RadioButton = (RadioButton) findViewById(R.id.callee1004RadioButton);
        final RadioButton audioOnlyRadioButton = (RadioButton) findViewById(R.id.audioOnlyRadioButton);
        final RadioButton audioAndVideoRadioButton = (RadioButton) findViewById(R.id.audioAndVideoRadioButton);
        affirmativeButton.setText("Connect");
        negativeButton.setText("");

        ServerSettings serverSettings = new ServerSettings();
        serverSettings.domain = SIP_SERVER_DOMAIN;
        serverSettings.dnsServer = SIP_DNS_SERVER;
        serverSettings.proxyServer = SIP_PROXY_SERVER;

        sipController.init(this, serverSettings);

        affirmativeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (callState == CallState.IDLE) {
                    String username = "";
                    String password = "123456";
                    if (user1001RadioButton.isChecked())
                        username = "1001";
                    else if (user1002RadioButton.isChecked())
                        username = "1002";
                    else if (user1003RadioButton.isChecked())
                        username = "1003";
                    else if (user1004RadioButton.isChecked())
                        username = "1004";
                    sipController.registerUser(username, password);
                    affirmativeButton.setText("Call");
                    negativeButton.setText("Disconnect");
                    callState = CallState.CONNECTED;
                } else if (callState == CallState.CONNECTED) {
                    String callee = "";
                    if (callee1001RadioButton.isChecked())
                        callee = "1001";
                    else if (callee1002RadioButton.isChecked())
                        callee = "1002";
                    else if (callee1003RadioButton.isChecked())
                        callee = "1003";
                    else if (callee1004RadioButton.isChecked())
                        callee = "1004";
                    String sipUrl = callee + "@" + SIP_SERVER_DOMAIN;
                    if (audioOnlyRadioButton.isChecked()) {
                        sipController.setHasAudio(true);
                        sipController.setHasVideo(false);
                        audioManager.setSpeakerphoneOn(false);
                    } else if (audioAndVideoRadioButton.isChecked()) {
                        sipController.setHasAudio(true);
                        sipController.setHasVideo(true);
                        audioManager.setSpeakerphoneOn(true);
                    }
                    LinearLayout videoViewLinearLayout = (LinearLayout) findViewById(R.id.videoViewLinerLayout);
                    videoViewLinearLayout.addView(surfaceView);
                    String cameraDeviceName = CameraEnumerationAndroid.getDeviceName(1);
                    Log.d(TAG, "Opening camera: " + cameraDeviceName);
                    VideoCapturerAndroid videoCapturer =
                            VideoCapturerAndroid.create(cameraDeviceName, null, null);
                    if (videoCapturer == null) {
                        Log.e(TAG, "Failed to open camera");
                        return;
                    }
                    sipController.setVideoCapturer(videoCapturer.takeNativeVideoCapturer(), videoCaptureConstraints);
                    VideoRenderer.Callbacks localRenderer = VideoRendererGui.create(0, 0, 50, 100,
                            RendererCommon.ScalingType.SCALE_ASPECT_FILL, true);
                    VideoRenderer.Callbacks remoteRenderer = VideoRendererGui.create(50, 0, 50, 100,
                            RendererCommon.ScalingType.SCALE_ASPECT_FILL, false);
                    VideoRenderer localVideoRenderer = new VideoRenderer(localRenderer);
                    VideoRenderer remoteVideoRenderer = new VideoRenderer(remoteRenderer);
                    sipController.setLocalView(localVideoRenderer.nativeVideoRenderer);
                    sipController.setRemoteView(remoteVideoRenderer.nativeVideoRenderer);
                    sipController.makeCall(sipUrl);
                    affirmativeButton.setText("");
                    negativeButton.setText("End Call");
                    callState = CallState.IN_CALL;
                } else if (callState == CallState.INCOMING_CALL) {
                    if (audioOnlyRadioButton.isChecked()) {
                        sipController.setHasAudio(true);
                        sipController.setHasVideo(false);
                        audioManager.setSpeakerphoneOn(false);
                    } else if (audioAndVideoRadioButton.isChecked()) {
                        sipController.setHasAudio(true);
                        sipController.setHasVideo(true);
                        audioManager.setSpeakerphoneOn(true);
                    }
                    LinearLayout videoViewLinearLayout = (LinearLayout) findViewById(R.id.videoViewLinerLayout);
                    videoViewLinearLayout.addView(surfaceView);
                    String cameraDeviceName = CameraEnumerationAndroid.getDeviceName(1);
                    Log.d(TAG, "Opening camera: " + cameraDeviceName);
                    VideoCapturerAndroid videoCapturer =
                            VideoCapturerAndroid.create(cameraDeviceName, null, null);
                    if (videoCapturer == null) {
                        Log.e(TAG, "Failed to open camera");
                        return;
                    }
                    sipController.setVideoCapturer(videoCapturer.takeNativeVideoCapturer(), videoCaptureConstraints);
                    VideoRenderer.Callbacks localRenderer = VideoRendererGui.create(0, 0, 50, 100,
                            RendererCommon.ScalingType.SCALE_ASPECT_FILL, true);
                    VideoRenderer.Callbacks remoteRenderer = VideoRendererGui.create(50, 0, 50, 100,
                            RendererCommon.ScalingType.SCALE_ASPECT_FILL, false);
                    VideoRenderer localVideoRenderer = new VideoRenderer(localRenderer);
                    VideoRenderer remoteVideoRenderer = new VideoRenderer(remoteRenderer);
                    sipController.setLocalView(localVideoRenderer.nativeVideoRenderer);
                    sipController.setRemoteView(remoteVideoRenderer.nativeVideoRenderer);
                    sipController.answer();
                    affirmativeButton.setText("");
                    negativeButton.setText("End Call");
                    callState = CallState.IN_CALL;
                }
            }
        });

        negativeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (callState == CallState.IN_CALL) {
                    LinearLayout videoViewLinearLayout = (LinearLayout) findViewById(R.id.videoViewLinerLayout);
                    videoViewLinearLayout.removeAllViews();
                    sipController.endCall(true);
                    affirmativeButton.setText("Call");
                    negativeButton.setText("Disconnect");
                    callState = CallState.CONNECTED;
                } else if (callState == CallState.INCOMING_CALL) {
                    sipController.endCall(false);
                    affirmativeButton.setText("Call");
                    negativeButton.setText("Disconnect");
                    callState = CallState.CONNECTED;
                } else if (callState == CallState.CONNECTED) {
                    sipController.unregisterUser();
                    affirmativeButton.setText("Connect");
                    negativeButton.setText("");
                    callState = CallState.IDLE;
                }
            }
        });

        sipController.registerOnRegistrationEventListener(new SipController.OnRegistrationEventListener() {
            @Override
            public void onRegistrationEvent(SipController.RegistrationEvent event, String user) {

            }
        });

        sipController.registerOnCallEventListener(new SipController.OnCallEventListener() {
            @Override
            public void onCallEvent(SipController.CallEvent event, String user) {
                final Button affirmativeButton = (Button) findViewById(R.id.affirmativeButton);
                final Button negativeButton = (Button) findViewById(R.id.negativeButton);
                if (event == SipController.CallEvent.INCOMING_CALL) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            affirmativeButton.setText("Answer");
                            negativeButton.setText("Decline");
                            callState = CallState.INCOMING_CALL;
                        }
                    });
                } else if (event == SipController.CallEvent.TERMINATE_CALL) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            LinearLayout videoViewLinearLayout = (LinearLayout) findViewById(R.id.videoViewLinerLayout);
                            videoViewLinearLayout.removeAllViews();
                            affirmativeButton.setText("Call");
                            negativeButton.setText("Disconnect");
                            callState = CallState.CONNECTED;
                        }
                    });
                }

            }
        });

        sipController.registerOnLogEventListener(new SipController.OnLogEventListener() {
            @Override
            public void onLogEvent(String log) {
                Log.d(TAG, log);
            }
        });

        sipController.registerOnErrorEventListener(new SipController.OnErrorEventListener() {
            @Override
            public void onErrorEvent(SipController.ErrorType type, String message) {
                Log.e(TAG, String.format("Error - %s", message));
                final Button affirmativeButton = (Button) findViewById(R.id.affirmativeButton);
                final Button negativeButton = (Button) findViewById(R.id.negativeButton);
                if (callState == CallState.IN_CALL && type == SipController.ErrorType.WEBRTC_ERROR) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            LinearLayout videoViewLinearLayout = (LinearLayout) findViewById(R.id.videoViewLinerLayout);
                            videoViewLinearLayout.removeAllViews();
                            sipController.endCall(true);
                            affirmativeButton.setText("Call");
                            negativeButton.setText("Disconnect");
                            callState = CallState.CONNECTED;
                        }
                    });
                } else if ((callState == CallState.IN_CALL ||
                        callState == CallState.INCOMING_CALL) &&
                        type == SipController.ErrorType.SIP_SESSION_ERROR) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            sipController.endCall(false);
                            affirmativeButton.setText("Call");
                            negativeButton.setText("Disconnect");
                            callState = CallState.CONNECTED;
                        }
                    });
                } else if ((callState == CallState.IN_CALL ||
                        callState == CallState.INCOMING_CALL ||
                        callState == CallState.CONNECTED) &&
                        type == SipController.ErrorType.SIP_CONNECTION_ERROR) {
                    runOnUiThread(new Runnable() {
                        public void run() {
                            if (callState == CallState.CONNECTED) {
                                sipController.unregisterUser();
                                affirmativeButton.setText("Connect");
                                negativeButton.setText("");
                                callState = CallState.IDLE;
                            }
                        }
                    });
                }

            }
        });
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
