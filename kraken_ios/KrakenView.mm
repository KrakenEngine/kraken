//
//  KrakenView.mm
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "KrakenView.h"
#import "KREngine-common.h"
#import "KREngine.h"

@interface KrakenView() {
  GLint framebufferWidth;
  GLint framebufferHeight;
  GLuint defaultFramebuffer;
  GLuint colorRenderbuffer;
  GLuint depthRenderbuffer;

  CFTimeInterval lastTimestamp;

  struct {
    unsigned int preRender:1;
    unsigned int postRender:1;
  } delegateRespondsTo;
}

@property (nonatomic, unsafe_unretained) CADisplayLink *__unsafe_unretained displayLink;
@property (nonatomic, strong) EAGLContext *context;

@end

@implementation KrakenView


+ (Class)layerClass
{
  return [CAEAGLLayer class];
}

- (id)initWithCoder:(NSCoder*)coder
{
  self = [super initWithCoder:coder];
  if (self) {
    lastTimestamp = 0.0;
    delegateRespondsTo.preRender = 0;
    delegateRespondsTo.postRender = 0;
    [self startAnimation];
  }

  return self;
}

- (void)dealloc
{
  [self stopAnimation];
}

- (void)setDelegate:(id <KrakenViewDelegate>)aDelegate {
  if (_delegate != aDelegate) {
    _delegate = aDelegate;

    delegateRespondsTo.preRender = [_delegate respondsToSelector:@selector(preRender:withDeltaTime:)];
    delegateRespondsTo.postRender = [_delegate respondsToSelector:@selector(postRender:withDeltaTime:)];
  }
}

- (void)startAnimation
{
  CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

  eaglLayer.contentsScale = [[UIScreen mainScreen] scale];
  eaglLayer.opaque = TRUE;
  eaglLayer.drawableProperties = @{
                                   kEAGLDrawablePropertyRetainedBacking: [NSNumber numberWithBool:FALSE],
                                   kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
                                   };
  KRContext::activateRenderContext();

  //EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  EAGLContext *aContext = [EAGLContext currentContext];
  if (!aContext) {
    NSLog(@"Failed to create ES context");
  } else if (![EAGLContext setCurrentContext:aContext]) {
    NSLog(@"Failed to set ES context current");
  }

  self.context = aContext;



  [self createFrameBuffer];

  CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
  [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  self.displayLink = aDisplayLink;
}

- (void)stopAnimation
{
  [self.displayLink invalidate];
  self.displayLink = nil;

  if (self.context) {
    [EAGLContext setCurrentContext:self.context];

    if (defaultFramebuffer) {
      GLDEBUG(glDeleteFramebuffers(1, &defaultFramebuffer));
      defaultFramebuffer = 0;
    }

    if (colorRenderbuffer) {
      GLDEBUG(glDeleteRenderbuffers(1, &colorRenderbuffer));
      colorRenderbuffer = 0;
    }

    if (depthRenderbuffer) {
      GLDEBUG(glDeleteRenderbuffers(1, &depthRenderbuffer));
      depthRenderbuffer = 0;
    }

    [EAGLContext setCurrentContext:nil];

    self.context = nil;
  }
}

- (void)drawFrame
{
  CFTimeInterval timeStamp = self.displayLink.timestamp;
  CFTimeInterval deltaTime;
  if (lastTimestamp == 0.0) {
    deltaTime = 0.0;
  } else {
    deltaTime = timeStamp - lastTimestamp;
  }
  lastTimestamp = timeStamp;

  KRContext *context = KREngine.sharedInstance.context;

  if (self.delegate && delegateRespondsTo.preRender) {
    [self.delegate preRender:context withDeltaTime:deltaTime];
  }

  // ---- Render the Buffer ----
  [EAGLContext setCurrentContext:self.context];
  if (self.scene) {
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer));
    [[KREngine sharedInstance] renderScene: self.scene WithDeltaTime: deltaTime AndWidth: framebufferWidth AndHeight: framebufferHeight AndDefaultFBO: defaultFramebuffer];
  } else {
    GLDEBUG(glClearColor(0.0f, 0.0f, 1.0f, 1.0f));
    GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
  }

  // ---- Present the Buffer ----

#if GL_EXT_discard_framebuffer
  GLenum attachments[2] = {GL_DEPTH_ATTACHMENT};
  GLDEBUG(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 1, attachments));
#endif

  GLDEBUG(glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer));
  [self.context presentRenderbuffer:GL_RENDERBUFFER];

  if (self.delegate && delegateRespondsTo.postRender) {
    [self.delegate postRender:context withDeltaTime:deltaTime];
  }
}

- (void)createFrameBuffer
{
  // Create default framebuffer object.
  GLDEBUG(glGenFramebuffers(1, &defaultFramebuffer));
  GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer));

  // Create color render buffer and allocate backing store.
  GLDEBUG(glGenRenderbuffers(1, &colorRenderbuffer));
  GLDEBUG(glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer));
  [self.context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
  GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth));
  GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight));

  // Attach color render buffer
  GLDEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer));

  // Create depth render buffer and allocate backing store.
  GLDEBUG(glGenRenderbuffers(1, &depthRenderbuffer));
  GLDEBUG(glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer));
  GLDEBUG(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight));

  // Attach depth render buffer
  GLDEBUG(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer));
}

@end


