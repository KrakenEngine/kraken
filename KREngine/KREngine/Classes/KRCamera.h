//
//  KRSettings.h
//  KREngine
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

#ifndef KRCAMERA_H
#define KRCAMERA_H

#import "KREngine-common.h"

#import "KRMat4.h"
#import "KRVector2.h"
#import "KRAABB.h"
#import "KRShader.h"
#import "KRContextObject.h"
#import "KRTexture.h"
#import "KRContext.h"
#import "KRViewport.h"
#import "KRRenderSettings.h"

class KRInstance;
class KRScene;
class KRViewport;

class KRCamera : public KRNode {
public:
    KRCamera(KRScene &scene, std::string name);
    virtual ~KRCamera();

    void renderFrame(float deltaTime); 
    
    KRRenderSettings settings;
        
private:    
    void createBuffers();
    
    GLint backingWidth, backingHeight;
    GLint volumetricBufferWidth, volumetricBufferHeight;
    
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    
    GLuint volumetricLightAccumulationBuffer, volumetricLightAccumulationTexture;
    
    void renderPost();
        
    void destroyBuffers();
    
    void renderFrame(KRScene &scene, float deltaTime);
    
    KRTexture *m_pSkyBoxTexture;
    KRViewport m_viewport;
    
    float m_particlesAbsoluteTime;
};

#endif
