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

#ifndef WEBRTCENGINE_H__
#define WEBRTCENGINE_H__

#include "talk/app/webrtc/peerconnectionfactory.h"
#include "talk/app/webrtc/peerconnection.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/videosourceinterface.h"

namespace rtcsip
{
    class WebRtcEngineObserver
    {
    public:
        virtual void onIceCandidate(std::string &sdp, std::string &mid) = 0;
        virtual void onIceGatheringFinished() = 0;
        virtual void onError() = 0;
    };
  
    class SessionDescriptionObserver
    {
    public:
        virtual void onCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc) = 0;
        virtual void onCreateSessionDescriptionFailure(const std::string &error) = 0;
        virtual void onSetSessionDescriptionSuccess() = 0;
        virtual void onSetSessionDescriptionFailure(const std::string &error) = 0;
    };
  
    class WebRtcEngineMediaConstraintsInterface : public webrtc::MediaConstraintsInterface
    {
    public:
        WebRtcEngineMediaConstraintsInterface(Constraints mandatory, Constraints optional);
        virtual ~WebRtcEngineMediaConstraintsInterface() {}

        virtual const Constraints& GetMandatory() const;
        virtual const Constraints& GetOptional() const;

    private:
        Constraints m_mandatory;
        Constraints m_optional;
    };
  
    class WebRtcEngineCreateSessionDescriptionObserver : public webrtc::CreateSessionDescriptionObserver
    {
    public:
        void registerSessionDescriptionObserver(SessionDescriptionObserver *observer);
      
        virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc);
        virtual void OnFailure(const std::string &error);
      
    private:
        SessionDescriptionObserver *m_sessionDescriptionObserver;
    };
  
    class WebRtcEngineSetSessionDescriptionObserver : public webrtc::SetSessionDescriptionObserver
    {
    public:
        void registerSessionDescriptionObserver(SessionDescriptionObserver *observer);
      
        virtual void OnSuccess();
        virtual void OnFailure(const std::string &error);
      
    private:
        SessionDescriptionObserver *m_sessionDescriptionObserver;
    };
  
    class WebRtcEngine : public webrtc::PeerConnectionObserver, public SessionDescriptionObserver
    {
    public:
        WebRtcEngine();
        ~WebRtcEngine();

        void setVideoCapturer(cricket::VideoCapturer *capturer, webrtc::MediaConstraintsInterface *captureConstraints);
        void setLocalRenderer(webrtc::VideoRendererInterface *renderer);
        void setRemoteRenderer(webrtc::VideoRendererInterface *renderer);
        void setHasAudio(bool hasAudio);
        void setHasVideo(bool hasVideo);

        void createLocalStream();
        void destroyLocalStream();
      
        void createPeerConnection();
        void destroyPeerConnection();
        std::string getSdpOffer();
        std::string getSdpAnswer();
        void setLocalSdp(std::string &sdp, std::string type);
        void setRemoteSdp(std::string &sdp, std::string type);
        void addICECandidate(std::string &sdp, std::string mid);
        void registerWebRtcEngineObserver(WebRtcEngineObserver *observer);
      
        virtual void onCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface* desc);
        virtual void onCreateSessionDescriptionFailure(const std::string &error);
        virtual void onSetSessionDescriptionSuccess();
        virtual void onSetSessionDescriptionFailure(const std::string &error);

        virtual void OnError();
        virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);
        virtual void OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed);
        virtual void OnAddStream(webrtc::MediaStreamInterface *stream);
        virtual void OnRemoveStream(webrtc::MediaStreamInterface *stream);
        virtual void OnDataChannel(webrtc::DataChannelInterface *data_channel);
        virtual void OnRenegotiationNeeded();
        virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);
        virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state);
        virtual void OnIceCandidate(const webrtc::IceCandidateInterface *candidate);
        virtual void OnIceComplete();

    protected:
        rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> m_peerConnectionFactory;
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> m_peerConnection;
        rtc::scoped_refptr<webrtc::MediaStreamInterface> m_localMediaStream;
        rtc::scoped_refptr<webrtc::MediaStreamInterface> m_remoteMediaStream;
        rtc::scoped_refptr<webrtc::CreateSessionDescriptionObserver> m_createSdpObserver;
        rtc::scoped_refptr<webrtc::SetSessionDescriptionObserver> m_setSdpObserver;
        rtc::scoped_ptr<rtc::Thread> m_signalingThread;
        rtc::scoped_ptr<rtc::Thread> m_workerThread;
        std::mutex m_callbackMutex;
        std::condition_variable m_callbackCondition;
        bool m_waitForCallback;
        cricket::VideoCapturer *m_capturer;
        webrtc::MediaConstraintsInterface *m_captureConstraints;
        webrtc::VideoRendererInterface *m_localRenderer;
        webrtc::VideoRendererInterface *m_remoteRenderer;
        std::string m_localSdp;
        std::string m_remoteSdp;
        bool m_hasAudio;
        bool m_hasVideo;
        WebRtcEngineObserver *m_webRtcEngineObserver;
    };
}

#endif
