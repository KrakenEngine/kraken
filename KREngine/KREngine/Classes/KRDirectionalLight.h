//
//  KRDirectionalLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRDirectionalLight_h
#define KREngine_KRDirectionalLight_h

#import "KRLight.h"
#import "KRMat4.h"

class KRDirectionalLight : public KRLight {
    
public:
    
    KRDirectionalLight(KRScene &scene, std::string name);
    virtual ~KRDirectionalLight();
    
    virtual std::string getElementName();
    KRVector3 getLocalLightDirection();
    KRVector3 getWorldLightDirection();
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass);
#endif
    
private:
    KRMat4 m_modelMatrix;
};


#endif
