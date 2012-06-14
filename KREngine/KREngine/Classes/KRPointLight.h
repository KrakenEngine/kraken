//
//  KRPointLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRPointLight_h
#define KREngine_KRPointLight_h

#import "KRLight.h"
#import "KRMat4.h"

class KRPointLight : public KRLight {
    
public:
    
    KRPointLight(std::string name);
    virtual ~KRPointLight();
    
    virtual std::string getElementName();
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass);
#endif
    
private:
    KRMat4 m_modelMatrix;
    
    void generateMesh();
    
    GLfloat *m_sphereVertices;
    int m_cVertices;
};

#endif
