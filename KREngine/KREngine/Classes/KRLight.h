//
//  KRLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRLIGHT_H
#define KRLIGHT_H

#import "KRResource.h"
#import "KRNode.h"
#import "KRTexture.h"

static const float KRLIGHT_MIN_INFLUENCE = 0.15f; // 0.05f

// KRENGINE_MAX_SHADOW_BUFFERS must be at least 6 to allow omni-directional lights to render cube maps

#define KRENGINE_MAX_SHADOW_BUFFERS 6
#define KRENGINE_SHADOW_MAP_WIDTH 1024
#define KRENGINE_SHADOW_MAP_HEIGHT 1024

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
    void deleteBuffers();

    virtual void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    int getShadowBufferCount();
    GLuint *getShadowTextures();
    KRViewport *getShadowViewports();
    
    
protected:
    KRLight(KRScene &scene, std::string name);
    
    float m_intensity;
    float m_decayStart;
    KRVector3 m_color;
    
    std::string m_flareTexture;
    KRTexture *m_pFlareTexture;
    float m_flareSize;
    
    bool m_casts_shadow;
    bool m_light_shafts;
    float m_dust_particle_density;
    float m_dust_particle_size;
    float m_dust_particle_intensity;
    
    
    // Shadow Maps
    int m_cShadowBuffers;
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRViewport m_shadowViewports[KRENGINE_MAX_SHADOW_BUFFERS];
    
    void allocateShadowBuffers(int cBuffers);
    void invalidateShadowBuffers();
    
    virtual int configureShadowBufferViewports(const KRViewport &viewport);
    void renderShadowBuffers(KRCamera *pCamera);
};

#endif
