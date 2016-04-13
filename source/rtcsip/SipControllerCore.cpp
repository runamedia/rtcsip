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

#include "SipControllerCore.h"

#ifdef ANDROID
#include <android/log.h>
#endif

namespace rtcsip
{
    SipControllerCore::SipControllerCore(SipServerSettings serverSettings, WebRtcEngine* webRtcEngine) :
        m_run(false),
        m_receiveClosed(false),
        m_inCall(false),
        m_isCaller(false),
        m_localSdpSet(false),
        m_remoteSdpSet(false),
        m_localCandidatesCollected(false),
        m_receiver(NULL),
        m_webRtcEngine(webRtcEngine),
        m_registrationHandler(NULL),
        m_callHandler(NULL),
        m_logHandler(NULL),
        m_errorHandler(NULL)
    {
        m_domain = serverSettings.domain;
        m_dnsServer = serverSettings.dnsServer;
        m_proxyServer = serverSettings.proxyServer;
        
        if (webRtcEngine != NULL)
            webRtcEngine->registerWebRtcEngineObserver(this);
    }

    SipControllerCore::~SipControllerCore()
    {
        if (m_receiver)
            delete m_receiver;
    }
  
    void SipControllerCore::receive()
    {
        bool run = true;
        
        while (run)
        {
            m_userAgent->process();
            m_receiveMutex.lock();
            run = m_run;
            sleepMs(100);
            m_receiveMutex.unlock();
        }
        
        m_receiveMutex.lock();
        
        m_receiveClosed = true;
        
        m_receiveMutex.unlock();
        
        m_receiveCondition.notify_one();
    }
    
    void SipControllerCore::registerRegistrationHandler(SipRegistrationHandler* handler)
    {
        std::lock_guard<std::mutex> lock(m_controllerMutex);
        m_registrationHandler = handler;
    }
  
    void SipControllerCore::registerCallHandler(SipCallHandler* handler)
    {
        std::lock_guard<std::mutex> lock(m_controllerMutex);
        m_callHandler = handler;
    }
  
    void SipControllerCore::registerLogHandler(SipLogHandler* handler)
    {
        std::lock_guard<std::mutex> lock(m_logMutex);
        m_logHandler = handler;
    }
  
    void SipControllerCore::registerErrorHandler(SipErrorHandler* handler)
    {
        std::lock_guard<std::mutex> lock(m_controllerMutex);
        m_errorHandler = handler;
    }

    void SipControllerCore::registerUser(std::string username, std::string password)
    {
        m_uri = username + "@" + m_domain;
        std::string address = "sip:" + m_uri;
        m_username = username;
        m_password = password;
        
#ifndef ANDROID
        Log::setLevel(Log::Stack);
#else
        Log::initialize(Log::Cout, Log::Stack, "SIP", m_androidLog);
        Log::setLevel(Log::Stack);
#endif
      
        if (m_proxyServer.size() != 0)
        {
          std::string proxyAddress = "sip:" + m_proxyServer;
          m_outboundProxy = Uri(Data(proxyAddress));
        }
        else
        {
          m_outboundProxy = Uri();
        }

        m_dnsServers.clear();
        if (m_dnsServer.size() != 0)
        {
          Data dnsServer(m_dnsServer.c_str());
          m_dnsServers.push_back(Tuple(dnsServer, 0, UNKNOWN_TRANSPORT).toGenericIPAddress());
        }
        else
        {
          Data dnsServer("8.8.8.8");
          m_dnsServers.push_back(Tuple(dnsServer, 0, UNKNOWN_TRANSPORT).toGenericIPAddress());
        }

        m_stack = new SipStack(0, m_dnsServers);
        
        m_userAgent = new DialogUsageManager(*m_stack);
        m_clientAddress = NameAddr(address.c_str());
        createMasterProfile();
        m_stack->addTransport(TCP, 5060);

        m_clientAuth = std::auto_ptr<ClientAuthManager>(new ClientAuthManager);
        m_userAgent->setClientAuthManager(m_clientAuth);
        
        std::auto_ptr<KeepAliveManager> keepAlive(new KeepAliveManager);
        m_userAgent->setKeepAliveManager(keepAlive);
        
        m_userAgent->setClientRegistrationHandler(this);
        
        m_userAgent->setInviteSessionHandler(this);
        
        m_userAgent->setMasterProfile(m_masterProfile);
        
        m_pollGrp = FdPollGrp::create();
        m_interruptor = new EventThreadInterruptor(*m_pollGrp);
        m_stackThread = new EventStackThread(*m_stack, *m_interruptor,*m_pollGrp);
        SharedPtr<SipMessage> regMessage = m_userAgent->makeRegistration(m_clientAddress);
        
        m_stack->run();
        m_stackThread->run();
        
        m_userAgent->send(regMessage);

        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            
            m_run = true;
            m_receiveClosed = false;
            
            m_receiver = new std::thread(&SipControllerCore::receive, this);
        }
    }
  
    void SipControllerCore::unregisterUser()
    {
        m_receiveMutex.lock();
        m_run = false;
        m_receiveMutex.unlock();
        
        {
            std::unique_lock<std::mutex> receiveConditionMutex(m_receiveMutex);
            if (!m_receiveClosed)
                m_receiveCondition.wait(receiveConditionMutex);
        }
    }
  
    void SipControllerCore::createSession(std::string remoteUser)
    {
        WebRtcEngine *webRtcEngine;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            m_isCaller = true;
            m_remoteUri = remoteUser;
        }

        webRtcEngine->createLocalStream();
        webRtcEngine->createPeerConnection();

        std::string localSdp = webRtcEngine->getSdpOffer();
        webRtcEngine->setLocalSdp(localSdp, "offer");
      
        printf("SDP Offer\n%s", localSdp.c_str());
        
        bool localCandidatesCollected;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_localSdp = localSdp;
            m_localSdpSet = true;
            localCandidatesCollected = m_localCandidatesCollected;
        }
        
        if (localCandidatesCollected)
            sendSdpOffer();
    }
  
    void SipControllerCore::acceptSession()
    {
        WebRtcEngine *webRtcEngine;
        std::string remoteSdp;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            remoteSdp = m_remoteSdp;
        }
        
        std::string localSdp;
        webRtcEngine->createLocalStream();
        webRtcEngine->createPeerConnection();

        webRtcEngine->setRemoteSdp(remoteSdp, "offer");
            
        localSdp = webRtcEngine->getSdpAnswer();
        
        webRtcEngine->setLocalSdp(localSdp, "answer");
    
        printf("SDP Offer\n%s", remoteSdp.c_str());
        printf("SDP Answer\n%s", localSdp.c_str());
        
        bool localCandidatesCollected;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_localSdp = localSdp;
            m_localSdpSet = true;
            localCandidatesCollected = m_localCandidatesCollected;
        }
        
        if (localCandidatesCollected)
        {
            sendSdpAnswer();

            {
                std::lock_guard<std::mutex> lock(m_controllerMutex);
                m_inCall = true;
            }
        }
    }
  
    void SipControllerCore::terminateSession(bool destroyLocalStream)
    {
        WebRtcEngine *webRtcEngine;
        bool inCall;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            inCall = m_inCall;
        }

        if (inCall)
        {
            webRtcEngine->destroyPeerConnection();
            webRtcEngine->destroyLocalStream();
        }
        else
        {
            if (destroyLocalStream)
            {
                webRtcEngine->destroyLocalStream();
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_inCall = false;
        }
        
        if (m_clientInviteSession)
            m_clientInviteSession->end(InviteSession::UserHangup);
        else if (m_serverInviteSession)
            m_serverInviteSession->end(InviteSession::UserHangup);

        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_isCaller = false;
            m_localSdpSet = false;
            m_remoteSdpSet = false;
            m_localCandidatesCollected = false;
            m_localSdp.clear();
            m_remoteSdp.clear();
            m_localCandidates.clear();
            m_clientInviteSession.reset();
            m_serverInviteSession.reset();
        }
    }
    
    void SipControllerCore::onSuccess(ClientRegistrationHandle, const SipMessage& response)
    {
        resip::H_From headerType;
        H_From::Type from = response.header(headerType);
        Data uri = from.uri().getAor();
        const char* fromC = from.uri().user().c_str();
        
        SipRegistrationHandler *registrationHandler;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            registrationHandler = m_registrationHandler;
        }
        
        registrationHandler->handleRegistration(Registered, fromC);
    }
    
    void SipControllerCore::onRemoved(ClientRegistrationHandle, const SipMessage& response)
    {
        InfoLog(<< "onRemoved: Unhandled method invoked");
    }
    
    int SipControllerCore::onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response)
    {
        InfoLog(<< "onRequestRetry: Unhandled method invoked");
        return 0;
    }
    
    void SipControllerCore::onFailure(ClientRegistrationHandle, const SipMessage& response)
    {
        InfoLog(<< "onFailure: Unhandled method invoked");
    }
    
    void SipControllerCore::onNewSession(ClientInviteSessionHandle cis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
    {
        InfoLog(<< "Outgoing call");
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_clientInviteSession = SharedPtr<ClientInviteSession>(cis.get());
        }
    }
    
    void SipControllerCore::onNewSession(ServerInviteSessionHandle sis, InviteSession::OfferAnswerType oat, const SipMessage& msg)
    {
        InfoLog(<< "Incoming call");
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_serverInviteSession = SharedPtr<ServerInviteSession>(sis.get());
        }
    }
    
    void SipControllerCore::onFailure(ClientInviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onFailure: Unhandled method invoked");
    }
    
    void SipControllerCore::onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&)
    {
        InfoLog(<< "onEarlyMedia: Unhandled method invoked");
    }
    
    void SipControllerCore::onProvisional(ClientInviteSessionHandle, const SipMessage&)
    {
        InfoLog(<< "onProvisional: Unhandled method invoked");
    }
    
    void SipControllerCore::onConnected(ClientInviteSessionHandle cis, const SipMessage& msg)
    {
        InfoLog(<< "Session connected");
    }
    
    void SipControllerCore::onConnected(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "Session connected");
    }
    
    void SipControllerCore::onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* related)
    {
        InfoLog(<< "Call terminated");
        
        WebRtcEngine *webRtcEngine;
        SipCallHandler *callHandler;
        bool inCall;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            callHandler = m_callHandler;
            inCall = m_inCall;
        }
        
        if (!inCall)
            return;
        
        webRtcEngine->destroyPeerConnection();
        webRtcEngine->destroyLocalStream();
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_inCall = false;
            m_isCaller = false;
            m_localSdpSet = false;
            m_remoteSdpSet = false;
            m_localCandidatesCollected = false;
            m_localSdp.clear();
            m_remoteSdp.clear();
            m_localCandidates.clear();
            m_clientInviteSession.reset();
            m_serverInviteSession.reset();
        }
        
        if (callHandler)
            callHandler->handleCall(TerminateCall, "");
    }
    
    void SipControllerCore::onForkDestroyed(ClientInviteSessionHandle)
    {
        InfoLog(<< "onForkDestroyed: Unhandled method invoked");
    }
    
    void SipControllerCore::onRedirected(ClientInviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onRedirected: Unhandled method invoked");
    }
    
    void SipControllerCore::onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&)
    {
        InfoLog(<< "Answer received");
        
        WebRtcEngine *webRtcEngine;
        SipCallHandler *callHandler;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            callHandler = m_callHandler;
        }
        
        HeaderFieldValue headerFieldValue = msg.getRawBody();
        
        std::string remoteSdp(headerFieldValue.getBuffer());
        
        printf("SDP Answer\n%s", remoteSdp.c_str());
        
        webRtcEngine->setRemoteSdp(remoteSdp, "answer");
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_inCall = true;
            m_remoteSdp = remoteSdp;
            m_remoteSdpSet = true;
        }
        
        if (callHandler)
            callHandler->handleCall(CallAccepted, "");
    }
    
    void SipControllerCore::onOffer(InviteSessionHandle, const SipMessage& msg, const SdpContents&)
    {
        InfoLog(<< "Offer received");
        
        WebRtcEngine *webRtcEngine;
        SipCallHandler *callHandler;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            webRtcEngine = m_webRtcEngine;
            callHandler = m_callHandler;
        }
        
        HeaderFieldValue headerFieldValue = msg.getRawBody();
        
        std::string remoteSdp(headerFieldValue.getBuffer(), headerFieldValue.getLength());
        remoteSdp[headerFieldValue.getLength()] = 0;

        printf("SDP Offer\n%s", remoteSdp.c_str());
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_isCaller = false;
            m_remoteSdp = remoteSdp;
            m_remoteSdpSet = true;
        }
        
        resip::H_From headerType;
        H_From::Type from = msg.header(headerType);
        Data uri = from.uri().getAor();
        const char* fromC = from.uri().user().c_str();

        if (callHandler)
            callHandler->handleCall(IncomingCall, fromC);
    }
    
    void SipControllerCore::onOfferRequired(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onOfferRequired: Unhandled method invoked");
    }
    
    void SipControllerCore::onOfferRejected(InviteSessionHandle, const SipMessage* msg)
    {
        InfoLog(<< "onOfferRejected: Unhandled method invoked");
    }
    
    void SipControllerCore::onInfo(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onInfo: Unhandled method invoked");
    }
    
    void SipControllerCore::onInfoSuccess(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onInfoSuccess: Unhandled method invoked");
    }
    
    void SipControllerCore::onInfoFailure(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onInfoFailure: Unhandled method invoked");
    }
    
    void SipControllerCore::onMessage(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onMessage: Unhandled method invoked");
    }
    
    void SipControllerCore::onMessageSuccess(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onMessageSuccess: Unhandled method invoked");
    }
    
    void SipControllerCore::onMessageFailure(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onMessageFailure: Unhandled method invoked");
    }
    
    void SipControllerCore::onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onRefer: Unhandled method invoked");
    }
    
    void SipControllerCore::onReferNoSub(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onReferNoSub: Unhandled method invoked");
    }
    
    void SipControllerCore::onReferRejected(InviteSessionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onReferRejected: Unhandled method invoked");
    }
    
    void SipControllerCore::onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg)
    {
        InfoLog(<< "onReferAccepted: Unhandled method invoked");
    }
    
    void SipControllerCore::onIceCandidate(std::string &sdp, std::string &mid)
    {
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            IceCandidate iceCandidate;
            iceCandidate.mid = mid;
            iceCandidate.candidate = sdp;
            m_localCandidates.push_back(iceCandidate);
        }
    }
    
    void SipControllerCore::onIceGatheringFinished()
    {
        bool isCaller;
        bool localSdpSet;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            isCaller = m_isCaller;
            localSdpSet = m_localSdpSet;
        }
        
        if (isCaller && localSdpSet)
        {
            sendSdpOffer();
        }
        else if (!isCaller && localSdpSet)
        {
            sendSdpAnswer();
            
            {
                std::lock_guard<std::mutex> lock(m_controllerMutex);
                m_inCall = true;
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            m_localCandidatesCollected = true;
        }
    }

    void SipControllerCore::onError()
    {
        SipErrorHandler *errorHandler;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            errorHandler = m_errorHandler;
        }
        
        if (errorHandler)
            errorHandler->handleError(WebRtcError, "WebRTC error");

    }

    void SipControllerCore::createMasterProfile()
    {
        m_masterProfile = SharedPtr<MasterProfile>(new MasterProfile);
        m_masterProfile->setInstanceId(m_uri.c_str());
        
        m_masterProfile->clearSupportedMethods();
        m_masterProfile->addSupportedMethod(INVITE);
        m_masterProfile->addSupportedMethod(ACK);
        m_masterProfile->addSupportedMethod(CANCEL);
        m_masterProfile->addSupportedMethod(OPTIONS);
        m_masterProfile->addSupportedMethod(BYE);
        m_masterProfile->addSupportedMethod(NOTIFY);
        m_masterProfile->addSupportedMethod(SUBSCRIBE);
        m_masterProfile->addSupportedMethod(INFO);
        m_masterProfile->addSupportedMethod(MESSAGE);
        m_masterProfile->addSupportedMethod(PRACK);
        m_masterProfile->setUacReliableProvisionalMode(MasterProfile::Supported);
        m_masterProfile->setUasReliableProvisionalMode(MasterProfile::SupportedEssential);
        
        // Support Languages
        m_masterProfile->clearSupportedLanguages();
        m_masterProfile->addSupportedLanguage(Token("en"));
        
        // Support Mime Types
        m_masterProfile->clearSupportedMimeTypes();
        m_masterProfile->addSupportedMimeType(INVITE, Mime("application", "sdp"));
        m_masterProfile->addSupportedMimeType(INVITE, Mime("multipart", "mixed"));
        m_masterProfile->addSupportedMimeType(INVITE, Mime("multipart", "signed"));
        m_masterProfile->addSupportedMimeType(INVITE, Mime("multipart", "alternative"));
        m_masterProfile->addSupportedMimeType(OPTIONS,Mime("application", "sdp"));
        m_masterProfile->addSupportedMimeType(OPTIONS,Mime("multipart", "mixed"));
        m_masterProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "signed"));
        m_masterProfile->addSupportedMimeType(OPTIONS, Mime("multipart", "alternative"));
        m_masterProfile->addSupportedMimeType(PRACK, Mime("application", "sdp"));
        m_masterProfile->addSupportedMimeType(PRACK, Mime("multipart", "mixed"));
        m_masterProfile->addSupportedMimeType(PRACK, Mime("multipart", "signed"));
        m_masterProfile->addSupportedMimeType(PRACK, Mime("multipart", "alternative"));
        m_masterProfile->addSupportedMimeType(UPDATE, Mime("application", "sdp"));
        m_masterProfile->addSupportedMimeType(UPDATE, Mime("multipart", "mixed"));
        m_masterProfile->addSupportedMimeType(UPDATE, Mime("multipart", "signed"));
        m_masterProfile->addSupportedMimeType(UPDATE, Mime("multipart", "alternative"));
        
        // Supported Options Tags
        m_masterProfile->clearSupportedOptionTags();
        //mMasterProfile->addSupportedOptionTag(Token(Symbols::Replaces));
        m_masterProfile->addSupportedOptionTag(Token(Symbols::Timer));     // Enable Session Timers
        
        // Supported Schemes
        m_masterProfile->clearSupportedSchemes();
        m_masterProfile->addSupportedScheme("sip");
        
        // Validation Settings
        m_masterProfile->validateContentEnabled() = false;
        m_masterProfile->validateContentLanguageEnabled() = false;
        m_masterProfile->validateAcceptEnabled() = false;
        
        // Have stack add Allow/Supported/Accept headers to INVITE dialog establishment messages
        m_masterProfile->clearAdvertisedCapabilities(); // Remove Profile Defaults, then add our preferences
        m_masterProfile->addAdvertisedCapability(Headers::Allow);
        //_masterProfile->addAdvertisedCapability(Headers::AcceptEncoding);  // This can be misleading - it might specify what is expected in response
        m_masterProfile->addAdvertisedCapability(Headers::AcceptLanguage);
        m_masterProfile->addAdvertisedCapability(Headers::Supported);
        m_masterProfile->setMethodsParamEnabled(true);
        
        m_masterProfile->setDefaultFrom(m_clientAddress);
        m_masterProfile->setDigestCredential(m_clientAddress.uri().host(), m_clientAddress.uri().user(), m_password.c_str());
      
        if (!m_outboundProxy.host().empty())
        {
            m_masterProfile->setOutboundProxy(m_outboundProxy);
            m_masterProfile->addSupportedOptionTag(Token(Symbols::Outbound));
        }
      
        m_masterProfile->setKeepAliveTimeForDatagram(30);
        m_masterProfile->setKeepAliveTimeForStream(120);
    }
    
    void SipControllerCore::sendSdpOffer()
    {
        Data txt(m_localSdp);
        
        HeaderFieldValue hfv(txt.data(), txt.size());
        Mime type("application", "sdp");
        SdpContents offerSdp(hfv, type);
        
        SdpContents::Session::Medium* audioMedium = NULL;
        SdpContents::Session::Medium* videoMedium = NULL;
        
        SdpContents::Session::MediumContainer& mediumContainer = offerSdp.session().media();
        
        SdpContents::Session::MediumContainer::iterator iter = mediumContainer.begin();
        while (iter != mediumContainer.end())
        {
            if (iter->name() == "audio")
                audioMedium = &(*iter);
            else if (iter->name() == "video")
                videoMedium = &(*iter);
            iter++;
        }
        
        std::vector<IceCandidate> localCandidates;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            localCandidates = m_localCandidates;
        }
        
        std::vector<IceCandidate>::iterator candidateIterator = localCandidates.begin();
        while (candidateIterator != localCandidates.end())
        {
            std::string localCandidate = candidateIterator->candidate;
            printf("Local Candidate: %s\n", localCandidate.c_str());
            std::string midString(candidateIterator->mid);
            std::string candidateString(candidateIterator->candidate);
            Data candidateData(candidateString.c_str());
            if (audioMedium != NULL && midString.compare("audio") == 0)
                audioMedium->addAttribute(Data("candidate"), candidateData.substr(10));
            else if (videoMedium != NULL && midString.compare("video") == 0)
                videoMedium->addAttribute(Data("candidate"), candidateData.substr(10));
            candidateIterator++;
        }
        
        std::string address = "sip:" + m_remoteUri;
        SharedPtr<SipMessage> msg = m_userAgent->makeInviteSession(NameAddr(address.c_str()), m_masterProfile, &offerSdp, 0);
        m_userAgent->send(msg);
        
        InfoLog(<< "SDP Offer Sent:\n" << m_localSdp);
    }
    
    void SipControllerCore::sendSdpAnswer()
    {
        Data txt(m_localSdp);
        
        HeaderFieldValue hfv(txt.data(), txt.size());
        Mime type("application", "sdp");
        SdpContents answerSdp(hfv, type);
        
        SdpContents::Session::Medium* audioMedium = NULL;
        SdpContents::Session::Medium* videoMedium = NULL;
        
        SdpContents::Session::MediumContainer& mediumContainer = answerSdp.session().media();
        
        SdpContents::Session::MediumContainer::iterator iter = mediumContainer.begin();
        while (iter != mediumContainer.end())
        {
            if (iter->name() == "audio")
                audioMedium = &(*iter);
            else if (iter->name() == "video")
                videoMedium = &(*iter);
            iter++;
        }
        
        std::vector<IceCandidate> localCandidates;
        {
            std::lock_guard<std::mutex> lock(m_controllerMutex);
            localCandidates = m_localCandidates;
        }

        std::vector<IceCandidate>::iterator candidateIterator = localCandidates.begin();
        while (candidateIterator != localCandidates.end())
        {
            std::string midString(candidateIterator->mid);
            std::string candidateString(candidateIterator->candidate);
            Data candidateData(candidateString.c_str());
            if (audioMedium != NULL && midString.compare("audio") == 0)
                audioMedium->addAttribute(Data("candidate"), candidateData.substr(10));
            else if (videoMedium != NULL && midString.compare("video") == 0)
                videoMedium->addAttribute(Data("candidate"), candidateData.substr(10));
            candidateIterator++;
        }
        
        m_serverInviteSession->provideAnswer(answerSdp);
        m_serverInviteSession->accept();
        
        InfoLog(<< "SDP Answer Sent:\n" << m_localSdp);
    }
}
