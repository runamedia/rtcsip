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

#ifndef SIPCONTROLLERCORE_H__
#define SIPCONTROLLERCORE_H__

#include "WebRtcEngine.h"

#include "resip/stack/SipStack.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/KeepAliveManager.hxx"
#include "resip/dum/ClientRegistration.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/OutOfDialogHandler.hxx"
#include "resip/dum/ClientInviteSession.hxx"
#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/ServerInviteSession.hxx"
#include "resip/stack/EventStackThread.hxx"
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/Helper.hxx"
#include "resip/stack/SipMessage.hxx"
#include "resip/recon/sdp/SdpCodec.hxx"
#include "resip/recon/sdp/Sdp.hxx"
#include "resip/recon/sdp/SdpMediaLine.hxx"
#include "resip/recon/sdp/SdpHelperResip.hxx"
#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "resip/stack/PlainContents.hxx"
#include "resip/stack/GenericContents.hxx"
#ifdef ANDROID
#include "rutil/AndroidLogger.hxx"
#endif

#include <mutex>
#include <thread>

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::APP

using namespace resip;

namespace rtcsip
{
    enum SipRegistrationEvent
    {
        Registered,
        NotRegistered
    };
    
    enum SipCallEvent
    {
        IncomingCall,
        TerminateCall,
        CallAccepted
    };
  
    enum SipErrorType
    {
        WebRtcError,
        SipConnectionError,
        SipSessionError
    };
  
    struct SipServerSettings
    {
      std::string domain;
      std::string dnsServer;
      std::string proxyServer;
    };
    
    class SipRegistrationHandler
    {
    public:
        virtual void handleRegistration(SipRegistrationEvent sipEvent, std::string user) = 0;
    };

    class SipCallHandler
    {
    public:
        virtual void handleCall(SipCallEvent sipEvent, std::string user) = 0;
    };
  
    class SipLogHandler
    {
    public:
        virtual void handleLog(std::string log) = 0;
    };
  
    class SipErrorHandler
    {
    public:
        virtual void handleError(SipErrorType sipType, std::string error) = 0;
    };

    class SipControllerCore : public ClientRegistrationHandler,
        public InviteSessionHandler,
        public WebRtcEngineObserver
    {
    public:
        struct IceCandidate
        {
            std::string mid;
            std::string candidate;
        };
      
        SipControllerCore(SipServerSettings serverSettings, WebRtcEngine* webRtcEngine);
        ~SipControllerCore();
    
        void receive();
        void registerRegistrationHandler(SipRegistrationHandler* handler);
        void registerCallHandler(SipCallHandler* handler);
        void registerLogHandler(SipLogHandler* handler);
        void registerErrorHandler(SipErrorHandler* handler);
        void registerUser(std::string username, std::string password);
        void unregisterUser();
        void createSession(std::string remoteUser);
        void acceptSession();
        void terminateSession(bool destroyLocalStream);
        
        virtual void onSuccess(ClientRegistrationHandle, const SipMessage& response);
        virtual void onRemoved(ClientRegistrationHandle, const SipMessage& response);
        virtual int onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response);
        virtual void onFailure(ClientRegistrationHandle, const SipMessage& response);

        virtual void onNewSession(ClientInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg);
        virtual void onNewSession(ServerInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg);
        virtual void onFailure(ClientInviteSessionHandle, const SipMessage& msg);
        virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&);
        virtual void onProvisional(ClientInviteSessionHandle, const SipMessage&);
        virtual void onConnected(ClientInviteSessionHandle, const SipMessage& msg);
        virtual void onConnected(InviteSessionHandle, const SipMessage& msg);
        virtual void onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* related=0);
        virtual void onForkDestroyed(ClientInviteSessionHandle);
        virtual void onRedirected(ClientInviteSessionHandle, const SipMessage& msg);
        virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&);
        virtual void onOffer(InviteSessionHandle, const SipMessage& msg, const SdpContents&);
        virtual void onOfferRequired(InviteSessionHandle, const SipMessage& msg);
        virtual void onOfferRejected(InviteSessionHandle, const SipMessage* msg);
        virtual void onInfo(InviteSessionHandle, const SipMessage& msg);
        virtual void onInfoSuccess(InviteSessionHandle, const SipMessage& msg);
        virtual void onInfoFailure(InviteSessionHandle, const SipMessage& msg);
        virtual void onMessage(InviteSessionHandle, const SipMessage& msg);
        virtual void onMessageSuccess(InviteSessionHandle, const SipMessage& msg);
        virtual void onMessageFailure(InviteSessionHandle, const SipMessage& msg);
        virtual void onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg);
        virtual void onReferNoSub(InviteSessionHandle, const SipMessage& msg);
        virtual void onReferRejected(InviteSessionHandle, const SipMessage& msg);
        virtual void onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg);
        
        virtual void onIceCandidate(std::string &sdp, std::string &mid);
        virtual void onIceGatheringFinished();
        virtual void onError();

    private:
        void createMasterProfile();
        void sendSdpOffer();
        void sendSdpAnswer();
        
    private:
        std::string m_domain;
        bool m_run;
        bool m_receiveClosed;
        std::string m_localSdp;
        std::string m_remoteSdp;
        std::vector<IceCandidate> m_localCandidates;
        bool m_inCall;
        bool m_isCaller;
        bool m_localSdpSet;
        bool m_remoteSdpSet;
        bool m_localCandidatesCollected;
        std::thread *m_receiver;
        std::mutex m_receiveMutex;
        std::condition_variable m_receiveCondition;
        std::mutex m_controllerMutex;
        std::mutex m_logMutex;
        WebRtcEngine *m_webRtcEngine;
        SipRegistrationHandler *m_registrationHandler;
        SipCallHandler *m_callHandler;
        SipLogHandler *m_logHandler;
        SipErrorHandler *m_errorHandler;
        char m_logBuffer[65536];
#ifdef ANDROID
        AndroidLogger m_androidLog;
#endif

        SipStack* m_stack;
        DnsStub::NameserverList m_dnsServers;
        EventStackThread* m_stackThread;
        DialogUsageManager* m_userAgent;
        FdPollGrp* m_pollGrp;
        EventThreadInterruptor* m_interruptor;
        NameAddr m_clientAddress;
        Uri m_outboundProxy;
        SharedPtr<MasterProfile> m_masterProfile;
        std::auto_ptr<ClientAuthManager> m_clientAuth;
        SharedPtr<ClientInviteSession> m_clientInviteSession;
        SharedPtr<ServerInviteSession> m_serverInviteSession;
        
        std::string m_username;
        std::string m_password;
        std::string m_uri;
        std::string m_remoteUri;
        std::string m_dnsServer;
        std::string m_proxyServer;
  };

}
#endif
