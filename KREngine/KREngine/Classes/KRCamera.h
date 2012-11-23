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

class KRInstance;
class KRScene;
class KRViewport;

class KRCamera : public KRContextObject {
public:
    KRCamera(KRContext &context);
    virtual ~KRCamera();
    
    GLint backingWidth, backingHeight;
    GLint volumetricBufferWidth, volumetricBufferHeight;
    
    void renderFrame(KRScene &scene, KRMat4 &viewMatrix, float deltaTime);

    KRVector3 getPosition() const;
    void setPosition(const KRVector3 &position);
    
    KRMat4 getProjectionMatrix();
    const KRVector2 &getViewportSize();
    void setViewportSize(const KRVector2 &size);
    
    bool bEnablePerPixel;
    bool bEnableDiffuseMap;
    bool bEnableNormalMap;
    bool bEnableSpecMap;
    bool bEnableReflectionMap;
    bool bEnableReflection;
    bool bEnableLightMap;
    bool bDebugPSSM;
    bool bDebugSuperShiny;
    bool bShowShadowBuffer;
    bool bShowOctree;
    bool bShowDeferred;
    bool bEnableAmbient;
    bool bEnableDiffuse;
    bool bEnableSpecular;
    bool bEnableDeferredLighting;
    float dSunR;
    float dSunG;
    float dSunB;
    float dAmbientR;
    float dAmbientG;
    float dAmbientB;
    float perspective_fov;

    
    
    
    int dof_quality;
    float dof_depth;
    float dof_falloff;
    bool bEnableFlash;
    float flash_intensity;
    float flash_depth;
    float flash_falloff;
    
    bool bEnableVignette;
    float vignette_radius;
    float vignette_falloff;
    
    KRVector2 m_viewportSize;
    
    int m_cShadowBuffers;
    
    std::string m_debug_text;
    
    void setSkyBox(const std::string &skyBoxName);
    
    float getPerspectiveNearZ();
    float getPerspectiveFarZ();
    void setPerspectiveNear(float v);
    void setPerpsectiveFarZ(float v);
    
    
    bool volumetric_environment_enable;
    int volumetric_environment_downsample;
    float volumetric_environment_max_distance;
    float volumetric_environment_quality;
    float volumetric_environment_intensity;

    float fog_near;
    float fog_far;
    float fog_density;
    KRVector3 fog_color;
    int fog_type; // 0 = no fog, 1 = linear, 2 = exponential, 3 = exponential squared
    
    float dust_particle_intensity;
    
private:
    KRVector3 m_position;
    
    void createBuffers();
    
    float perspective_nearz;
    float perspective_farz;
    
    int m_iFrame;
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    
    GLuint volumetricLightAccumulationBuffer, volumetricLightAccumulationTexture;
    
    void renderPost();
        
    void destroyBuffers();
    
    void renderFrame(KRScene &scene, float deltaTime);
    
    
    

    
    
    class KRInstanceDistance {
    public:
        KRInstanceDistance(KRInstance *pInstance, float distance) : m_pInstance(pInstance), m_distance(distance) {};
        ~KRInstanceDistance() {};
        
        // a predicate implemented as a class:
        class InstanceEqualsPredicate
        {
        public:
            InstanceEqualsPredicate(KRInstance *pInstance) : m_pInstance(pInstance) {};
            bool operator() (const KRInstanceDistance& value) {return value.m_pInstance == m_pInstance; }
            
        private:
            KRInstance *m_pInstance;
        };
                           
        KRInstance *m_pInstance;
        float m_distance;
    };
    
    std::string m_skyBoxName;
    KRTexture *m_pSkyBoxTexture;
    
    
    KRViewport m_viewport;
    
    float m_particlesAbsoluteTime;
};

#endif
