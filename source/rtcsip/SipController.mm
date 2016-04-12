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

#import "SipController.h"

#import "RTCEAGLVideoView.h"
#import "RTCVideoRendererAdapter.h"
#include "SipControllerCore.h"

using namespace rtcsip;

class SipControllerCoreWrapper : public SipRegistrationHandler, public SipCallHandler, public SipLogHandler, public SipErrorHandler
{
public:
    SipControllerCoreWrapper(SipControllerCore *controller, WebRtcEngine *webRtcEngine);
    ~SipControllerCoreWrapper();
  
    void registerRegistrationHandler(void (^handler)(RegistrationEvent, NSString*));
    void registerCallHandler(void (^handler)(CallEvent, NSString*));
    void registerLogHandler(void (^handler)(NSString*));
    void registerErrorHandler(void (^handler)(ErrorType, NSString*));
    void registerUser(std::string username, std::string password);
    void unregisterUser();
    void createSession(std::string remoteUser);
    void acceptSession();
    void terminateSession(bool destroyLocalStream);
  
    void setLocalVideoView(RTCEAGLVideoView *view);
    void setRemoteVideoView(RTCEAGLVideoView *view);
    void setDnsServer(std::string dns);
    void setProxyServer(std::string proxy);
    void setHasAudio(bool hasAudio);
    void setHasVideo(bool hasVideo);
    
    virtual void handleRegistration(SipRegistrationEvent sipEvent, std::string user);

    virtual void handleCall(SipCallEvent event, std::string user);
    
    virtual void handleLog(std::string log);
  
    virtual void handleError(SipErrorType type, std::string error);
  
private:
    void (^m_registrationHandler)(RegistrationEvent, NSString*);
    void (^m_callHandler)(CallEvent, NSString*);
    void (^m_logHandler)(NSString*);
    void (^m_errorHandler)(ErrorType, NSString*);
    SipControllerCore *m_sipControllerCore;
    WebRtcEngine *m_webRtcEngine;
    RTCVideoRendererAdapter *m_localVideoRendererAdapter;
    RTCVideoRendererAdapter *m_remoteVideoRendererAdapter;
};

SipControllerCoreWrapper::SipControllerCoreWrapper(SipControllerCore *controller, WebRtcEngine *webRtcEngine) :
    m_sipControllerCore(controller),
    m_webRtcEngine(webRtcEngine)
{
    m_sipControllerCore->registerRegistrationHandler(this);
    m_sipControllerCore->registerCallHandler(this);
    m_sipControllerCore->registerLogHandler(this);
    m_sipControllerCore->registerErrorHandler(this);
}

SipControllerCoreWrapper::~SipControllerCoreWrapper()
{
  
}

void SipControllerCoreWrapper::registerRegistrationHandler(void (^handler)(RegistrationEvent, NSString*))
{
    m_registrationHandler = handler;
}

void SipControllerCoreWrapper::registerCallHandler(void (^handler)(CallEvent, NSString*))
{
    m_callHandler = handler;
}

void SipControllerCoreWrapper::registerLogHandler(void (^handler)(NSString*))
{
    m_logHandler = handler;
}

void SipControllerCoreWrapper::registerErrorHandler(void (^handler)(ErrorType, NSString*))
{
    m_errorHandler = handler;
}

void SipControllerCoreWrapper::registerUser(std::string username, std::string password)
{
    m_sipControllerCore->registerUser(username, password);
}

void SipControllerCoreWrapper::unregisterUser()
{
    m_sipControllerCore->unregisterUser();
}

void SipControllerCoreWrapper::createSession(std::string remoteUser)
{
    m_sipControllerCore->createSession(remoteUser);
}

void SipControllerCoreWrapper::acceptSession()
{
    m_sipControllerCore->acceptSession();
}

void SipControllerCoreWrapper::terminateSession(bool destroyLocalStream)
{
    m_sipControllerCore->terminateSession(destroyLocalStream);
}

void SipControllerCoreWrapper::setLocalVideoView(RTCEAGLVideoView *view)
{
    m_localVideoRendererAdapter =
        [[RTCVideoRendererAdapter alloc] initWithVideoRenderer:view];
    m_webRtcEngine->setLocalRenderer(m_localVideoRendererAdapter.nativeVideoRenderer);
}

void SipControllerCoreWrapper::setRemoteVideoView(RTCEAGLVideoView *view)
{
    m_remoteVideoRendererAdapter =
        [[RTCVideoRendererAdapter alloc] initWithVideoRenderer:view];
    m_webRtcEngine->setRemoteRenderer(m_remoteVideoRendererAdapter.nativeVideoRenderer);
}

void SipControllerCoreWrapper::setHasAudio(bool hasAudio)
{
    m_webRtcEngine->setHasAudio(hasAudio);
}

void SipControllerCoreWrapper::setHasVideo(bool hasVideo)
{
    m_webRtcEngine->setHasVideo(hasVideo);
}

void SipControllerCoreWrapper::handleRegistration(SipRegistrationEvent sipEvent, std::string user)
{
    RegistrationEvent event;
    if (sipEvent == SipRegistrationEvent::Registered)
        event = RegistrationEvent::Registered;
    else if (sipEvent == SipRegistrationEvent::NotRegistered)
        event = RegistrationEvent::NotRegistered;
    else
        return;
    
    if (m_registrationHandler)
        m_registrationHandler(event, [NSString stringWithUTF8String:user.c_str()]);
 
}

void SipControllerCoreWrapper::handleCall(SipCallEvent sipEvent, std::string user)
{
    CallEvent event;
    if (sipEvent == SipCallEvent::IncomingCall)
        event = CallEvent::IncomingCall;
    else if (sipEvent == SipCallEvent::TerminateCall)
        event = CallEvent::TerminateCall;
    else if (sipEvent == SipCallEvent::CallAccepted)
        event = CallEvent::CallAccepted;
    else
        return;
  
    if (m_callHandler)
        m_callHandler(event, [NSString stringWithUTF8String:user.c_str()]);
}

void SipControllerCoreWrapper::handleLog(std::string log)
{
    if (m_logHandler)
        m_logHandler([NSString stringWithUTF8String:log.c_str()]);
}

void SipControllerCoreWrapper::handleError(SipErrorType sipType, std::string error)
{
    ErrorType type;
    if (sipType == SipErrorType::WebRtcError)
        type = ErrorType::WebRtcError;
    else if (sipType == SipErrorType::SipConnectionError)
        type = ErrorType::SipStackError;
    else if (sipType == SipErrorType::SipSessionError)
        type = ErrorType::SipStackError;
    else
        return;
  
    if (m_errorHandler)
        m_errorHandler(type, [NSString stringWithUTF8String:error.c_str()]);
}

@implementation ServerSettings

@synthesize domain;
@synthesize dnsServer;
@synthesize proxyServer;

@end

@interface SipController ()

@property(nonatomic, assign) SipControllerCoreWrapper *sipControllerCoreWrapper;
@property(nonatomic, assign) SipControllerCore *sipControllerCore;
@property(nonatomic, assign) WebRtcEngine *webRtcEngine;
@property(nonatomic, strong) RTCEAGLVideoView *localView;
@property(nonatomic, strong) RTCEAGLVideoView *remoteView;

@end

@implementation SipController

@synthesize sipControllerCore = _sipControllerCore;
@synthesize sipControllerCoreWrapper = _sipControllerCoreWrapper;
@synthesize webRtcEngine = _webRtcEngine;
@synthesize localView = _localView;
@synthesize remoteView = _remoteView;

- (id)init {
    if (self = [super init]) {
        rtcsip::SipServerSettings sipServerSettings;
        sipServerSettings.domain = "developer.runamedia.com";
        
        _webRtcEngine = new WebRtcEngine();
        _sipControllerCore = new rtcsip::SipControllerCore(sipServerSettings, _webRtcEngine);
        _sipControllerCoreWrapper = new SipControllerCoreWrapper(_sipControllerCore, _webRtcEngine);
    }
    return self;
}

- (id)init: (ServerSettings *) serverSettings{
    if (self = [super init]) {
        rtcsip::SipServerSettings sipServerSettings;
        sipServerSettings.domain = [serverSettings.domain UTF8String];
        sipServerSettings.dnsServer = [serverSettings.dnsServer UTF8String];
        sipServerSettings.proxyServer = [serverSettings.proxyServer UTF8String];
      
        _webRtcEngine = new WebRtcEngine();
        _sipControllerCore = new rtcsip::SipControllerCore(sipServerSettings, _webRtcEngine);
        _sipControllerCoreWrapper = new SipControllerCoreWrapper(_sipControllerCore, _webRtcEngine);
    }
    return self;
}

- (void)dealloc {
    delete(_webRtcEngine);
    delete(_sipControllerCore);
    delete(_sipControllerCoreWrapper);
}

- (void)setHasAudio:(BOOL)hasAudio {
    _sipControllerCoreWrapper->setHasAudio(hasAudio);
}

- (void)setHasVideo:(BOOL)hasVideo {
    _sipControllerCoreWrapper->setHasVideo(hasVideo);
}

- (void)registerUserWithUsername:(NSString *)username andPassword:(NSString *)password {
    std::string usernameString([username UTF8String]);
    std::string passwordString([password UTF8String]);
    _sipControllerCoreWrapper->registerUser(usernameString, passwordString);
}

- (void)unregisterUser {
    _sipControllerCoreWrapper->unregisterUser();
}

- (void)makeCallTo:(NSString *)sipUri {
    _sipControllerCoreWrapper->setLocalVideoView([self localView]);
    _sipControllerCoreWrapper->setRemoteVideoView([self remoteView]);
    _sipControllerCoreWrapper->createSession([sipUri UTF8String]);
}

- (void)answer {
    _sipControllerCoreWrapper->setLocalVideoView([self localView]);
    _sipControllerCoreWrapper->setRemoteVideoView([self remoteView]);
    _sipControllerCoreWrapper->acceptSession();
}

- (void)endCallAndDestroyLocalStream:(BOOL) destroyLocalStream {
    _sipControllerCoreWrapper->terminateSession(destroyLocalStream);
}

- (void)registerRegistrationHandler:(void (^)(RegistrationEvent, NSString*))handler {
    _sipControllerCoreWrapper->registerRegistrationHandler(handler);
}

- (void)registerCallHandler:(void (^)(CallEvent, NSString*))handler {
    _sipControllerCoreWrapper->registerCallHandler(handler);
}

- (void)registerLogHandler:(void (^)(NSString*))handler {
    _sipControllerCoreWrapper->registerLogHandler(handler);
}

- (void)registerErrorHandler:(void (^)(ErrorType, NSString*))handler {
    _sipControllerCoreWrapper->registerErrorHandler(handler);
}

- (void)setLocalView:(RTCEAGLVideoView*)view {
    _localView = view;
}

- (void)setRemoteView:(RTCEAGLVideoView*)view {
    _remoteView = view;
}

@end
