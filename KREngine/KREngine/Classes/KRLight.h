//
//  KRLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRLight_h
#define KREngine_KRLight_h

#import "KRResource.h"
#import "KRNode.h"
#import "KRTexture.h"

static const float KRLIGHT_MIN_INFLUENCE = 0.15f; // 0.05f

class KRLight : public KRNode {
public:
    
    
    virtual ~KRLight();
    virtual std::string getElementName() = 0;
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void setIntensity(float intensity);
    float getIntensity();
    void setDecayStart(float decayStart);
    float getDecayStart();
    const KRVector3 &getColor();
    void setColor(const KRVector3 &color);
    
    void setFlareTexture(std::string flare_texture);
    void setFlareSize(float flare_size);
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass);
    
#endif
    
protected:
    KRLight(KRScene &scene, std::string name);
    
    float m_intensity;
    float m_decayStart;
    KRVector3 m_color;
    
    std::string m_flareTexture;
    KRTexture *m_pFlareTexture;
    float m_flareSize;
};

#endif
