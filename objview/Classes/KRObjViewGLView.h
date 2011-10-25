//
//  KRObjViewGLView.h
//  KRObjView
//
//  Created by Mac on 11-05-01.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import <KREngine.h>
#import <KRVector3.h>
#import <KRScene.h>

@interface KRObjViewGLView : UIView {

	/* The pixel dimensions of the backbuffer */
	GLint backingWidth, backingHeight;
	
	EAGLContext *context;
	
	/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
	GLuint viewFramebuffer, viewRenderbuffer;
    
    KREngine *renderEngine;
    
    KRScene m_scene;
	
}

// OpenGL drawing
- (BOOL)createFramebuffers;
- (void)destroyFramebuffer;
- (void)setDisplayFramebuffer;
- (BOOL)presentFramebuffer;
- (KREngine *)getEngine;
- (KRScene *)getScene;
- (BOOL)loadObjects;


@end
