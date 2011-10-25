//
//  KRObjViewViewController.h
//  KRObjView
//
//  Created by Mac on 11-04-29.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "KRObjViewGLView.h"
#import <KREngine.h>
#import <KRVector3.h>
#import <KRScene.h>

@interface KRObjViewViewController : UIViewController {

	CALayer *overlayLayer;
	
	KRObjViewGLView *glView;
    
    float heading;
    
    Vector3 camera_position;
    double camera_pitch;
    double camera_yaw;
    
    double leftStickStartX;
    double leftStickStartY;
    double rightStickStartX;
    double rightStickStartY;
    
    double leftStickDeltaX;
    double leftStickDeltaY;
    double rightStickDeltaX;
    double rightStickDeltaY;
    double dLeftSlider;
    double dRightSlider;
    
    bool bUpdateParam;
    bool bLoadedTestInstances;
    int cParamDisplayFrames;
    
    
    id displayLink;

}

@property (nonatomic, retain) IBOutlet CALayer *overlayLayer;
@property (readonly) KRObjViewGLView *glView;

// OpenGL ES 2.0 setup methods
- (void)drawView:(id)sender;

@end
