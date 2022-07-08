//
//  KRLight.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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
#pragma once

#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

static const float KRLIGHT_MIN_INFLUENCE = 0.15f; // 0.05f

// KRENGINE_MAX_SHADOW_BUFFERS must be at least 6 to allow omni-directional lights to render cube maps

#define KRENGINE_MAX_SHADOW_BUFFERS 6
#define KRENGINE_SHADOW_MAP_WIDTH 1024
#define KRENGINE_SHADOW_MAP_HEIGHT 1024

class KRLight : public KRNode {
public:
    static void InitNodeInfo(KrNodeInfo* nodeInfo);
    
    virtual ~KRLight();
    virtual std::string getElementName() = 0;
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void setIntensity(float intensity);
    float getIntensity();
    void setDecayStart(float decayStart);
    float getDecayStart();
    const Vector3 &getColor();
    void setColor(const Vector3 &color);
    
    void setFlareTexture(std::string flare_texture);
    void setFlareSize(float flare_size);
    void setFlareOcclusionSize(float occlusion_size);
    void deleteBuffers();

    virtual void render(RenderInfo& ri);
    
    int getShadowBufferCount();
    GLuint *getShadowTextures();
    KRViewport *getShadowViewports();
    
    
protected:
    KRLight(KRScene &scene, std::string name);
    
    float m_intensity;
    float m_decayStart;
    Vector3 m_color;
    
    std::string m_flareTexture;
    KRTexture *m_pFlareTexture;
    float m_flareSize;
    float m_flareOcclusionSize;
    
    bool m_casts_shadow;
    bool m_light_shafts;
    float m_dust_particle_density;
    float m_dust_particle_size;
    float m_dust_particle_intensity;
    
    GLuint m_occlusionQuery; // Occlusion query for attenuating occluded flares
    
    
    // Shadow Maps
    int m_cShadowBuffers;
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRViewport m_shadowViewports[KRENGINE_MAX_SHADOW_BUFFERS];
    
    void allocateShadowBuffers(int cBuffers);
    void invalidateShadowBuffers();
    
    virtual int configureShadowBufferViewports(const KRViewport &viewport);
    void renderShadowBuffers(RenderInfo& ri);
};
