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

#include "WebRtcEngine.h"

#include "webrtc/base/ssladapter.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#define DEFAULT_STUN_SERVER_URL "stun:stun.counterpath.net"
#define DEFAULT_TURN_SERVER_URL "turn:developer.runamedia.com"
#define DEFAULT_TURN_SERVER_USERNAME "test"
#define DEFAULT_TURN_SERVER_PASSWORD "test"

namespace rtcsip
{
    WebRtcEngineMediaConstraintsInterface::WebRtcEngineMediaConstraintsInterface(
        webrtc::MediaConstraintsInterface::Constraints mandatory,
        webrtc::MediaConstraintsInterface::Constraints optional) :
        m_mandatory(mandatory), m_optional(optional)
    {
    
    }
  
    const webrtc::MediaConstraintsInterface::Constraints& WebRtcEngineMediaConstraintsInterface::GetMandatory() const
    {
        return m_mandatory;
    }
    
    const webrtc::MediaConstraintsInterface::Constraints& WebRtcEngineMediaConstraintsInterface::GetOptional() const
    {
        return m_optional;
    }

    void WebRtcEngineCreateSessionDescriptionObserver::registerSessionDescriptionObserver(SessionDescriptionObserver *observer)
    {
        m_sessionDescriptionObserver = observer;
    }
    
    void WebRtcEngineCreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc)
    {
        if (m_sessionDescriptionObserver != NULL)
            m_sessionDescriptionObserver->onCreateSessionDescriptionSuccess(desc);
    }
    
    void WebRtcEngineCreateSessionDescriptionObserver::OnFailure(const std::string& error)
    {
        if (m_sessionDescriptionObserver != NULL)
            m_sessionDescriptionObserver->onCreateSessionDescriptionFailure(error);
    }

    void WebRtcEngineSetSessionDescriptionObserver::registerSessionDescriptionObserver(SessionDescriptionObserver *observer)
    {
        m_sessionDescriptionObserver = observer;
    }
  
    void WebRtcEngineSetSessionDescriptionObserver::OnSuccess()
    {
        if (m_sessionDescriptionObserver != NULL)
            m_sessionDescriptionObserver->onSetSessionDescriptionSuccess();
    }
    
    void WebRtcEngineSetSessionDescriptionObserver::OnFailure(const std::string& error)
    {
        if (m_sessionDescriptionObserver != NULL)
            m_sessionDescriptionObserver->onSetSessionDescriptionFailure(error);
    }
  
    WebRtcEngine::WebRtcEngine() :
        m_createSdpObserver(new rtc::RefCountedObject<WebRtcEngineCreateSessionDescriptionObserver>()),
        m_setSdpObserver(new rtc::RefCountedObject<WebRtcEngineSetSessionDescriptionObserver>()),
        m_waitForCallback(false), m_capturer(NULL), m_captureConstraints(NULL),
        m_localRenderer(NULL), m_remoteRenderer(NULL),
        m_hasAudio(true), m_hasVideo(true), m_webRtcEngineObserver(NULL)
    {
        m_signalingThread.reset(new rtc::Thread());
        m_signalingThread->Start();
        m_workerThread.reset(new rtc::Thread());
        m_workerThread->Start();

        m_peerConnectionFactory = webrtc::CreatePeerConnectionFactory(m_signalingThread.get(),
            m_workerThread.get(), NULL, NULL, NULL);
        if (!m_peerConnectionFactory) {
            printf("Failed to initialize PeerConnectionFactory!\n");
            return;
        }
      
        //rtc::LogMessage::LogToDebug(rtc::LS_SENSITIVE);
      
        static_cast<WebRtcEngineCreateSessionDescriptionObserver *>(m_createSdpObserver.get())->registerSessionDescriptionObserver(this);
        static_cast<WebRtcEngineSetSessionDescriptionObserver *>(m_setSdpObserver.get())->registerSessionDescriptionObserver(this);
      
        bool initialized = rtc::InitializeSSL();
        if (!initialized) {
            printf("Failed to initialize SSL library\n");
            return;
        }
    }
  
    WebRtcEngine::~WebRtcEngine()
    {
        bool deinitialized = rtc::CleanupSSL();
        if (!deinitialized) {
            printf("Failed to deinitialize SSL library\n");
        }
    }

    void WebRtcEngine::setVideoCapturer(cricket::VideoCapturer *capturer, webrtc::MediaConstraintsInterface *captureConstraints)
    {
        m_capturer = capturer;
        m_captureConstraints = captureConstraints;
    }

    void WebRtcEngine::setLocalRenderer(webrtc::VideoRendererInterface *renderer)
    {
        m_localRenderer = renderer;
    }
    
    void WebRtcEngine::setRemoteRenderer(webrtc::VideoRendererInterface *renderer)
    {
        m_remoteRenderer = renderer;
    }
  
    void WebRtcEngine::setHasAudio(bool hasAudio)
    {
        m_hasAudio = hasAudio;
    }
  
    void WebRtcEngine::setHasVideo(bool hasVideo)
    {
        m_hasVideo = hasVideo;
    }

    void WebRtcEngine::createLocalStream()
    {
        m_localMediaStream = m_peerConnectionFactory->CreateLocalMediaStream("media_stream");
      
        if (m_hasVideo) {
            if (m_capturer == NULL) {
                rtc::scoped_ptr<cricket::DeviceManagerInterface> deviceManager(
                        cricket::DeviceManagerFactory::Create());
                bool initialized = deviceManager->Init();
                if (!initialized) {
                    printf("DeviceManager::Init() failed\n");
                    return;
                }

                std::vector<cricket::Device> devices;
                bool getDevicesSucceeded = deviceManager->GetVideoCaptureDevices(&devices);
                if (!getDevicesSucceeded) {
                    printf("DeviceManager::GetVideoCaptureDevices() failed\n");
                    return;
                }

                cricket::Device cameraDevice;
                std::vector<cricket::Device>::iterator deviceIter = devices.begin();
                while (deviceIter != devices.end()) {
                    cameraDevice = *deviceIter;
                    deviceIter++;
                }

                rtc::scoped_ptr<cricket::VideoCapturer> capturer(
                        deviceManager->CreateVideoCapturer(cameraDevice));

                webrtc::MediaConstraintsInterface::Constraints mandatory;
                webrtc::MediaConstraintsInterface::Constraints optional;

                mandatory.push_back(
                        webrtc::MediaConstraintsInterface::Constraint(
                                webrtc::MediaConstraintsInterface::kMaxWidth, "640"));
                mandatory.push_back(
                        webrtc::MediaConstraintsInterface::Constraint(
                                webrtc::MediaConstraintsInterface::kMaxHeight, "480"));
                optional.push_back(
                        webrtc::MediaConstraintsInterface::Constraint(
                                webrtc::MediaConstraintsInterface::kMaxFrameRate, "15"));

                rtc::scoped_ptr<WebRtcEngineMediaConstraintsInterface> capturerConstraints(
                        new WebRtcEngineMediaConstraintsInterface(mandatory, optional));

                rtc::scoped_refptr<webrtc::VideoSourceInterface> videoSource(
                        m_peerConnectionFactory->CreateVideoSource(capturer.release(),
                                                                   capturerConstraints.get()));
                if (videoSource == NULL) {
                    printf("PeerConnectionFactoryInterface::CreateVideoSource() failed\n");
                    return;
                }

                rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack =
                        m_peerConnectionFactory->CreateVideoTrack("video_track", videoSource.get());
                if (videoTrack == NULL) {
                    printf("PeerConnectionFactoryInterface::CreateVideoTrack() failed\n");
                    return;
                }

                videoTrack->AddRenderer(m_localRenderer);

                bool addTrackSucceeded = m_localMediaStream->AddTrack(videoTrack.get());
                if (!addTrackSucceeded) {
                    printf("VideoTrackInterface::AddTrack() failed\n");
                    return;
                }
            } else {
                rtc::scoped_refptr<webrtc::VideoSourceInterface> videoSource(
                        m_peerConnectionFactory->CreateVideoSource(m_capturer,
                                                                   m_captureConstraints));
                if (videoSource == NULL) {
                    printf("PeerConnectionFactoryInterface::CreateVideoSource() failed\n");
                    return;
                }

                rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack =
                        m_peerConnectionFactory->CreateVideoTrack("video_track", videoSource.get());
                if (videoTrack == NULL) {
                    printf("PeerConnectionFactoryInterface::CreateVideoTrack() failed\n");
                    return;
                }

                videoTrack->AddRenderer(m_localRenderer);

                bool addTrackSucceeded = m_localMediaStream->AddTrack(videoTrack.get());
                if (!addTrackSucceeded) {
                    printf("VideoTrackInterface::AddTrack() failed\n");
                    return;
                }
            }
        }
      
        if (m_hasAudio) {
            rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack =
                m_peerConnectionFactory->CreateAudioTrack("audio_track", NULL);
            if (audioTrack == NULL) {
                printf("PeerConnectionFactoryInterface::CreateAudioTrack() failed\n");
                return;
            }
          
            bool addTrackSucceeded = m_localMediaStream->AddTrack(audioTrack.get());
            if (!addTrackSucceeded) {
                printf("AudioTrackInterface::AddTrack() failed\n");
                return;
            }
        }
    }
  
    void WebRtcEngine::destroyLocalStream()
    {
        webrtc::MediaStreamInterface *localMediaStream = m_localMediaStream.release();
        localMediaStream->Release();
    }

    void WebRtcEngine::createPeerConnection()
    {
        webrtc::PeerConnectionInterface::IceServers iceServers;
        webrtc::PeerConnectionInterface::IceServer stunServer;
        stunServer.uri = DEFAULT_STUN_SERVER_URL;
        stunServer.username = "";
        stunServer.password = "";
        webrtc::PeerConnectionInterface::IceServer turnServer;
        turnServer.uri = DEFAULT_TURN_SERVER_URL;
        turnServer.username = DEFAULT_TURN_SERVER_USERNAME;
        turnServer.password = DEFAULT_TURN_SERVER_PASSWORD;
        iceServers.push_back(stunServer);
        iceServers.push_back(turnServer);
      
        webrtc::MediaConstraintsInterface::Constraints mandatory;
        webrtc::MediaConstraintsInterface::Constraints optional;
      
        optional.push_back(
            webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, "true"));
      
        rtc::scoped_ptr<WebRtcEngineMediaConstraintsInterface> peerConnectionConstraints(
            new WebRtcEngineMediaConstraintsInterface(mandatory, optional));

        m_peerConnection = m_peerConnectionFactory->CreatePeerConnection(
            iceServers, peerConnectionConstraints.get(), NULL, NULL, this);
        if (m_peerConnection == NULL) {
            printf("PeerConnectionFactoryInterface::CreatePeerConnection() failed\n");
            return;
        }
      
        bool addStreamSucceeded = m_peerConnection->AddStream(m_localMediaStream.get());
        if (!addStreamSucceeded) {
            printf("PeerConnectionInterface::AddStream() failed\n");
            return;
        }
    }
  
    void WebRtcEngine::destroyPeerConnection()
    {
        m_peerConnection->RemoveStream(m_localMediaStream.get());
        m_peerConnection->Close();
      
        webrtc::PeerConnectionInterface *peerConnection = m_peerConnection.release();
        peerConnection->Release();
    }

    std::string WebRtcEngine::getSdpOffer()
    {
        webrtc::MediaConstraintsInterface::Constraints mandatory;
        webrtc::MediaConstraintsInterface::Constraints optional;
      
        if (m_hasAudio && m_hasVideo) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, "true"));
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, "true"));
        } else if (m_hasAudio) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, "true"));
        } else if (m_hasVideo) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, "true"));
        }
      
        rtc::scoped_ptr<WebRtcEngineMediaConstraintsInterface> sdpConstraints(
            new WebRtcEngineMediaConstraintsInterface(mandatory, optional));
      
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            while (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
            m_waitForCallback = true;
        }
      
      
        m_peerConnection->CreateOffer(m_createSdpObserver.get(), sdpConstraints.get());
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            if (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
        }
      
        return m_localSdp;
    }
  
    std::string WebRtcEngine::getSdpAnswer()
    {
        webrtc::MediaConstraintsInterface::Constraints mandatory;
        webrtc::MediaConstraintsInterface::Constraints optional;
      
        if (m_hasAudio && m_hasVideo) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, "true"));
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, "true"));
        } else if (m_hasAudio) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, "true"));
        } else if (m_hasVideo) {
            mandatory.push_back(
                webrtc::MediaConstraintsInterface::Constraint(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, "true"));
        }
        
        rtc::scoped_ptr<WebRtcEngineMediaConstraintsInterface> sdpConstraints(
            new WebRtcEngineMediaConstraintsInterface(mandatory, optional));
        
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            while (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
            m_waitForCallback = true;
        }
        
        m_peerConnection->CreateAnswer(m_createSdpObserver.get(), sdpConstraints.get());
        
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            if (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
        }
        
        return m_localSdp;
    }
  
    void WebRtcEngine::setLocalSdp(std::string &sdp, std::string type)
    {
        m_localSdp = sdp;

        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            while (m_waitForCallback)
              m_callbackCondition.wait(callbackConditionMutex);
            m_waitForCallback = true;
        }

        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface *desc = webrtc::CreateSessionDescription(type, sdp, &error);
        m_peerConnection->SetLocalDescription(m_setSdpObserver.get(), desc);
      
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            if (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
        }
    }
  
    void WebRtcEngine::setRemoteSdp(std::string &sdp, std::string type)
    {
        m_remoteSdp = sdp;
      
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            while (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
            m_waitForCallback = true;
        }

        webrtc::SdpParseError error;
        webrtc::SessionDescriptionInterface *desc = webrtc::CreateSessionDescription(type, sdp, &error);
        m_peerConnection->SetRemoteDescription(m_setSdpObserver.get(), desc);
        
        {
            std::unique_lock<std::mutex> callbackConditionMutex(m_callbackMutex);
            if (m_waitForCallback)
                m_callbackCondition.wait(callbackConditionMutex);
        }
    }
  
    void WebRtcEngine::addICECandidate(std::string &sdp, std::string mid)
    {
        webrtc::SdpParseError error;
        webrtc::IceCandidateInterface *iceCandidate = webrtc::CreateIceCandidate(mid, 0, sdp, &error);
        m_peerConnection->AddIceCandidate(iceCandidate);
    }
  
    void WebRtcEngine::registerWebRtcEngineObserver(WebRtcEngineObserver *observer)
    {
        m_webRtcEngineObserver = observer;
    }
  
    void WebRtcEngine::onCreateSessionDescriptionSuccess(webrtc::SessionDescriptionInterface *desc)
    {
        printf("WebRtcEngine onCreateSessionDescriptionSuccess()\n");
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            desc->ToString(&m_localSdp);
            m_waitForCallback = false;
        }
      
        m_callbackCondition.notify_all();
    }
  
    void WebRtcEngine::onCreateSessionDescriptionFailure(const std::string &error)
    {
        printf("WebRtcEngine onCreateSessionDescriptionFailure()\n");
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_waitForCallback = false;
        }
      
        m_callbackCondition.notify_all();
    }
  
    void WebRtcEngine::onSetSessionDescriptionSuccess()
    {
        printf("WebRtcEngine onSetSessionDescriptionSuccess()\n");
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_waitForCallback = false;
        }
      
        m_callbackCondition.notify_all();
    }
  
    void WebRtcEngine::onSetSessionDescriptionFailure(const std::string &error)
    {
        printf("WebRtcEngine onSetSessionDescriptionFailure()\n");
        {
            std::lock_guard<std::mutex> lock(m_callbackMutex);
            m_waitForCallback = false;
        }
      
        m_callbackCondition.notify_all();
    }
  
    void WebRtcEngine::OnError()
    {
        printf("WebRtcEngine OnError()\n");
        if (m_webRtcEngineObserver != NULL) {
            m_webRtcEngineObserver->onError();
        }
    }

    void  WebRtcEngine::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
    {
        printf("WebRtcEngine OnSignalingChange(): %d\n", new_state);
    }
    
    void  WebRtcEngine::OnStateChange(webrtc::PeerConnectionObserver::StateType state_changed)
    {
        printf("WebRtcEngine OnStateChange(): %d\n", state_changed);
    }
    
    void  WebRtcEngine::OnAddStream(webrtc::MediaStreamInterface* stream)
    {
        printf("WebRtcEngine OnAddStream()\n");
      
        m_remoteMediaStream = stream;
      
        if (m_hasVideo && m_remoteMediaStream->GetVideoTracks().size() != 0) {
            rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack = m_remoteMediaStream->GetVideoTracks()[0];
            videoTrack->AddRenderer(m_remoteRenderer);
        }
    }
    
    void  WebRtcEngine::OnRemoveStream(webrtc::MediaStreamInterface *stream)
    {
        printf("WebRtcEngine OnRemoveStream()\n");
    }
  
    void  WebRtcEngine::OnDataChannel(webrtc::DataChannelInterface *data_channel)
    {
        printf("WebRtcEngine OnDataChannel()\n");
    }
    
    void  WebRtcEngine::OnRenegotiationNeeded()
    {
        printf("WebRtcEngine OnRenegotiationNeeded()\n");
    }
    
    void  WebRtcEngine::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
    {
        printf("WebRtcEngine OnIceConnectionChange(): %d\n", new_state);
    }
    
    void  WebRtcEngine::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
    {
        printf("WebRtcEngine OnIceGatheringChange(): %d\n", new_state);
        if (m_webRtcEngineObserver != NULL && new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete)
            m_webRtcEngineObserver->onIceGatheringFinished();
    }
    
    void  WebRtcEngine::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
    {
        std::string mid = candidate->sdp_mid();
        int index = candidate->sdp_mline_index();
        std::string sdp;
        candidate->ToString(&sdp);
      
        printf("WebRtcEngine onICECandidate().\n Mid[%s] Index[%d] Sdp[%s]\n", mid.c_str(), index, sdp.c_str());
      
        if (m_webRtcEngineObserver != NULL) {
            m_webRtcEngineObserver->onIceCandidate(sdp, mid);
        }
    }
    
    void  WebRtcEngine::OnIceComplete()
    {
        printf("WebRtcEngine OnIceComplete()\n");
    }
}
