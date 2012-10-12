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


#define KRENGINE_MAX_SHADOW_BUFFERS 3
#define KRENGINE_SHADOW_MAP_WIDTH 2048
#define KRENGINE_SHADOW_MAP_HEIGHT 2048

class KRInstance;
class KRScene;
class KRContext;

class KRCamera : public KRContextObject {
public:
    KRCamera(KRContext &context, GLint width, GLint height);
    virtual ~KRCamera();
    
    GLint backingWidth, backingHeight;
    
    void renderFrame(KRScene &scene, KRMat4 &viewMatrix);
    void renderShadowBuffer(KRScene &scene, int iShadow);
    void invalidateShadowBuffers();
    void allocateShadowBuffers();
    void createBuffers();
    
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
    double dSunR;
    double dSunG;
    double dSunB;
    double dAmbientR;
    double dAmbientG;
    double dAmbientB;
    double perspective_fov;
    double perspective_nearz;
    double perspective_farz;
    
    int dof_quality;
    double dof_depth;
    double dof_falloff;
    bool bEnableFlash;
    double flash_intensity;
    double flash_depth;
    double flash_falloff;
    
    bool bEnableVignette;
    double vignette_radius;
    double vignette_falloff;
    
    KRVector2 m_viewportSize;
    
    int m_cShadowBuffers;
    
    std::string m_debug_text;
    
    void setSkyBox(const std::string &skyBoxName);

private:
    KRVector3 m_position;
    
    int m_iFrame;
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRMat4 shadowmvpmatrix[KRENGINE_MAX_SHADOW_BUFFERS]; /* MVP Matrix for view from light source */
    
    void renderPost();
        
    void destroyBuffers();
    
    void renderFrame(KRScene &scene, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRVector3 &cameraPosition);
    
    
    

    
    
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
    
    std::set<KRAABB> m_visibleBounds; // AABB's that output fragments in the last frame
    std::set<KRAABB> m_shadowVisibleBounds[KRENGINE_MAX_SHADOW_BUFFERS]; // AABB's that output fragments in the last frame for each shadow map
    
    std::string m_skyBoxName;
    KRTexture *m_pSkyBoxTexture;
    
};

#endif
