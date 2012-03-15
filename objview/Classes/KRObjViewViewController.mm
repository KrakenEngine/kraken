//
//  KRObjViewViewController.m
//  KRObjView
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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

#import "KRObjViewViewController.h"
#import <KRMat4.h>
#import <KRModelManager.h>

@implementation KRObjViewViewController

@synthesize overlayLayer;
@synthesize glView;

// Handle Touch Events
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
    for(id touch in touches) {
        CGPoint touchPoint = [touch locationInView:self.view];
        if(!leftStickStartY && !rightStickStartY && touchPoint.y < CGRectGetMinY(self.view.frame) + CGRectGetHeight(self.view.frame) * 0.05) {
            dRightSlider = (touchPoint.x - CGRectGetMinX(self.view.frame)) / CGRectGetWidth(self.view.frame);
            cParamDisplayFrames = 30;
            bUpdateParam = true;
        } else if(!leftStickStartY && !rightStickStartY && touchPoint.y > CGRectGetMinY(self.view.frame) + CGRectGetHeight(self.view.frame) * 0.95) {
            dLeftSlider = (touchPoint.x - CGRectGetMinX(self.view.frame)) / CGRectGetWidth(self.view.frame);
            cParamDisplayFrames = 30;
        } else if(touchPoint.y > CGRectGetMidY(self.view.frame)) {
            leftStickStartX = touchPoint.x;
            leftStickStartY = touchPoint.y;
            leftStickDeltaX = 0.0f;
            leftStickDeltaY = 0.0f;
            // NSLog(@"Left Stick Pressed");
        } else {
            rightStickStartX = touchPoint.x;
            rightStickStartY = touchPoint.y;
            rightStickDeltaX = 0.0f;
            rightStickDeltaY = 0.0f;
            // NSLog(@"Right Stick Pressed");
        }
    }

}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
    for(id touch in touches) {
        CGPoint touchPoint = [touch locationInView:self.view];
        if(!leftStickStartY && !rightStickStartY && touchPoint.y < CGRectGetMinY(self.view.frame) + CGRectGetHeight(self.view.frame) * 0.10) {
            dRightSlider = (touchPoint.x - CGRectGetMinX(self.view.frame)) / CGRectGetWidth(self.view.frame);
            cParamDisplayFrames = 30;
            bUpdateParam = true;
        } else if(!leftStickStartY && !rightStickStartY && touchPoint.y > CGRectGetMinY(self.view.frame) + CGRectGetHeight(self.view.frame) * 0.90) {
            dLeftSlider = (touchPoint.x - CGRectGetMinX(self.view.frame)) / CGRectGetWidth(self.view.frame);
            cParamDisplayFrames = 30;
        } else if(touchPoint.y > CGRectGetMidY(self.view.frame)) {
            if(leftStickStartX > 0.0f) { // Avoid interpreting touches sliding across center of screen
                leftStickDeltaX = (leftStickStartX - touchPoint.x) / (CGRectGetWidth(self.view.frame) * 0.25);
                leftStickDeltaY = (leftStickStartY - touchPoint.y) / (CGRectGetHeight(self.view.frame) * 0.25);
                // clamp values
                if(leftStickDeltaX < -1.0f) {
                    leftStickDeltaX = -1.0f;
                } else if(leftStickDeltaX > 1.0f) {
                    leftStickDeltaX = 1.0f;
                }
                if(leftStickDeltaY < -1.0f) {
                    leftStickDeltaY = -1.0f;
                } else if(leftStickDeltaY > 1.0f) {
                    leftStickDeltaY = 1.0f;
                }
            }
        } else {
            if(rightStickStartX > 0.0f) { // Avoid interpreting touches sliding across center of screen
                rightStickDeltaX = (rightStickStartX - touchPoint.x) / (CGRectGetWidth(self.view.frame) * 0.25);
                rightStickDeltaY = (rightStickStartY - touchPoint.y) / (CGRectGetHeight(self.view.frame) * 0.25);
                // clamp values
                if(rightStickDeltaX < -1.0f) {
                    rightStickDeltaX = -1.0f;
                } else if(rightStickDeltaX > 1.0f) {
                    rightStickDeltaX = 1.0f;
                }
                if(rightStickDeltaY < -1.0f) {
                    rightStickDeltaY = -1.0f;
                } else if(rightStickDeltaY > 1.0f) {
                    rightStickDeltaY = 1.0f;
                }
            }
        }
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
    for(id touch in touches) {
        CGPoint touchPoint = [touch locationInView:self.view];
        if(touchPoint.y > CGRectGetMidY(self.view.frame)) {
            leftStickStartX = 0.0f;
            leftStickStartY = 0.0f;
            leftStickDeltaX = 0.0f;
            leftStickDeltaY = 0.0f;
        } else {
            rightStickStartX = 0.0f;
            rightStickStartY = 0.0f;
            rightStickDeltaX = 0.0f;
            rightStickDeltaY = 0.0f;
        }
    }
}
-(void)loadView {    

	CGRect mainScreenFrame = [[UIScreen mainScreen] applicationFrame];	
	UIView *primaryView = [[UIView alloc] initWithFrame:mainScreenFrame];
	self.view = primaryView;
	[primaryView release];
	
	glView = [[KRObjViewGLView alloc] initWithFrame:CGRectMake(0.0f, 0.0f, mainScreenFrame.size.width, mainScreenFrame.size.height)];
	glView.multipleTouchEnabled = YES;
	
	[self.view addSubview:glView];
	[glView release];
    
    camera_yaw = -4.0;
    camera_pitch = 0.1;
    
    leftStickStartX = 0.0f;
    leftStickStartY = 0.0f;
    rightStickStartX = 0.0f;
    rightStickStartY = 0.0f;
    leftStickDeltaX = 0.0f;
    leftStickDeltaY = 0.0f;
    rightStickDeltaX = 0.0f;
    rightStickDeltaY = 0.0f;
    

    bUpdateParam = false;
    dRightSlider = 0.0f;
    dLeftSlider = 0.0f;
    bLoadedTestInstances = false;
    cParamDisplayFrames = 0;
    
    camera_position = Vector3(-850, -10, -700);
    
    displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
    [displayLink setFrameInterval:1]; // Maximum 60fps
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void)dealloc 
{
    [displayLink invalidate];
    
    [super dealloc];
}


- (void)drawView:(id)sender
{
    const GLfloat PI = 3.14159265;
    const GLfloat d2r = PI * 2 / 360;
    
    
    KREngine *engine = [glView getEngine];
    int iParam = int(dLeftSlider * ([engine getParameterCount] + 1));
    if(iParam > [engine getParameterCount]) {
        iParam = [engine getParameterCount];
    }
    
    if(cParamDisplayFrames && iParam < [engine getParameterCount]) {
        cParamDisplayFrames--;
        char szText[256];
        const char *szName = [[engine getParameterLabelWithIndex: iParam] UTF8String];
        double dValue = [engine getParameterValueWithIndex: iParam];
        switch([engine getParameterTypeWithIndex: iParam]) {
            case KRENGINE_PARAMETER_INT:
                sprintf(szText, "%s: %i", szName, (int)dValue);
                break;
            case KRENGINE_PARAMETER_BOOL:
                sprintf(szText, "%s: %s", szName, dValue == 0.0 ? "false" : "true");
                break;
            case KRENGINE_PARAMETER_FLOAT:
                sprintf(szText, "%s: %f", szName, dValue);
                break;
        }
        NSString *debug_text = [[NSString alloc] initWithUTF8String:szText];
        [engine setDebugText: debug_text];
    } else {
        [engine setDebugText: @""];
    }
    
    
    
    if(bUpdateParam) {
        bUpdateParam = false;
        if(iParam == [engine getParameterCount] && dRightSlider > 0.9) {
            if(!bLoadedTestInstances) {
                bLoadedTestInstances = true;
                
                KRModelManager *pModelManager = [engine getModelManager];

                KRScene *scene =[glView getScene];
                
                 srand ( time(NULL) );
                 for(int iHouse=1; iHouse < 12; iHouse++) {
                     for(int iInstance=0; iInstance < 20; iInstance++) {
                         char szName[100];
                         sprintf(szName, "fachwerkhaus%i", iHouse);
                         KRMat4 pos;
                         
                         pos.rotate((double)rand() / (double)RAND_MAX * 6.282, Y_AXIS);
                         pos.translate((double)rand() / (double)RAND_MAX * 10000 - 5000, 0.0, (double)rand() / (double)RAND_MAX * 10000 - 5000);
                         
                         scene->addInstance(pModelManager->getModel(szName), pos);
                     }
                 }
                
            }

        } else {
            double dValue = dRightSlider * ([engine getParameterMaxWithIndex: iParam] - [engine getParameterMinWithIndex: iParam]) + [engine getParameterMinWithIndex: iParam];
            switch([engine getParameterTypeWithIndex: iParam]) {
                case KRENGINE_PARAMETER_INT:
                    dValue = dRightSlider * ([engine getParameterMaxWithIndex: iParam] + 0.5 - [engine getParameterMinWithIndex: iParam]) + [engine getParameterMinWithIndex: iParam];
                    [engine setParameterValueWithIndex: iParam Value: dValue];
                    break;
                case KRENGINE_PARAMETER_BOOL:
                    [engine setParameterValueWithIndex: iParam Value: 1.0 - dValue];
                    break;
                case KRENGINE_PARAMETER_FLOAT:
                    [engine setParameterValueWithIndex: iParam Value: dValue];
                    break;
            }
            

        }
    }
    
    
    
    [glView setDisplayFramebuffer];
    

    
    //double dScaleFactor = [engine getModelManager]->getFirstModel()->getMaxDimension() / 100.0f;
    double dScaleFactor = 10.0f;
    
    camera_position.z += (-cos(camera_pitch) * cos(camera_yaw) * leftStickDeltaX  + -cos(camera_pitch) * cos(camera_yaw - 90.0f * d2r) * -leftStickDeltaY) * dScaleFactor;
    camera_position.x += (cos(camera_pitch) * sin(camera_yaw) * leftStickDeltaX + cos(camera_pitch) * sin(camera_yaw - 90.0f * d2r) * -leftStickDeltaY) * dScaleFactor;
    camera_position.y += sin(camera_pitch)  * leftStickDeltaX * dScaleFactor;
    camera_yaw += rightStickDeltaY * 4.0 * d2r;
    camera_pitch += rightStickDeltaX * 4.0 * d2r;

    [engine renderScene: [glView getScene] WithPosition:camera_position Yaw: camera_yaw Pitch: camera_pitch Roll:0.0f];
    
    
    [glView presentFramebuffer];
}

@end
