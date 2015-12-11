#import "ViewController.h"
#import "RTCClient.h"
#import <webRTC/webRTC.h>
#import "VideoCallViewController.h"
#import "SipController.h"

#define  DEFAULT_STUN_SERVER_URL  @"stun:stun.counterpath.net"

@interface ViewController ()

@property (nonatomic, strong) RTCClient* client;
@property (nonatomic, strong) RTCMediaConstraints* peerConstraints;
@property(nonatomic, strong) RTCPeerConnection *peerConnection;

- (void)makeVideoCallWithLocalRendererView:(RTCEAGLVideoView*) localRenderer remoteRenderer:(RTCEAGLVideoView*) remoteRenderer;

- (void) prepare;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([segue.identifier isEqualToString:@"sequeVideoCall"])
    {
        VideoCallViewController* videoCallViewController = segue.destinationViewController;
        [self makeVideoCallWithLocalRendererView:videoCallViewController.localVideoView remoteRenderer:videoCallViewController.remoteVideoView];
    }
}

 -(void)prepare
{
    NSArray *optionalConstraints = @[[[RTCPair alloc] initWithKey:@"DtlsSrtpKeyAgreement" value:@"true"]];
    self.peerConstraints = [[RTCMediaConstraints alloc] initWithMandatoryConstraints:nil optionalConstraints:optionalConstraints];
}

- (void)makeVideoCallWithLocalRendererView:(RTCEAGLVideoView*) localRenderer remoteRenderer:(RTCEAGLVideoView*) remoteRenderer
{
    NSError* error = nil;
    
    self.client = [[RTCClient alloc] init];
    [self.client addICEServer:DEFAULT_STUN_SERVER_URL error:&error];
    self.client.localVideoView = localRenderer;
    self.client.remoteVideoView = remoteRenderer;
    RTCMediaStream *localStream = [self.client createLocalMediaStreamWithAudio:YES video:YES videoSource:AVCaptureDevicePositionBack ];
    if (localStream)
    {
        self.peerConnection = [self.client createPeerConnectionWithMediaStream:localStream constraints:self.peerConstraints isInitiater:YES];
    }
  
    SipController* sipController = [[SipController alloc] init];
    [sipController test];
}


@end
