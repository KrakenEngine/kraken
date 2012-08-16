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


#define KRENGINE_MAX_SHADOW_BUFFERS 3
#define KRENGINE_SHADOW_MAP_WIDTH 2048
#define KRENGINE_SHADOW_MAP_HEIGHT 2048

class KRInstance;
class KRScene;
class KRContext;

class KRCamera {
public:
    KRCamera(KRContext &context, GLint width, GLint height);
    ~KRCamera();
    
    GLint backingWidth, backingHeight;
    
    void renderFrame(KRContext &context, KRScene &scene, KRMat4 &viewMatrix);
    void renderShadowBuffer(KRContext &context, KRScene &scene, int iShadow);
    void invalidatePostShader();
    void invalidateShadowBuffers();
    void allocateShadowBuffers();
    void createBuffers(KRContext &context);
    
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
    bool bEnableLightMap;
    bool bDebugPSSM;
    bool bDebugSuperShiny;
    bool bShowShadowBuffer;
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
    
    std::vector<KRInstance *> m_transparentInstances;
    int m_cShadowBuffers;
    
    std::string m_debug_text;

private:
    KRVector3 m_position;
    
    int m_iFrame;
    GLuint compositeFramebuffer, compositeDepthTexture, compositeColorTexture;
    GLuint lightAccumulationBuffer, lightAccumulationTexture;
    
    
    GLuint shadowFramebuffer[KRENGINE_MAX_SHADOW_BUFFERS], shadowDepthTexture[KRENGINE_MAX_SHADOW_BUFFERS];
    bool shadowValid[KRENGINE_MAX_SHADOW_BUFFERS];
    KRMat4 shadowmvpmatrix[KRENGINE_MAX_SHADOW_BUFFERS]; /* MVP Matrix for view from light source */
    
    
    // uniform index
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_LIGHT_POSITION,
        KRENGINE_UNIFORM_LIGHT_POSITION_VIEW_SPACE,
        KRENGINE_UNIFORM_LIGHT_DIRECTION,
        KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE,
        KRENGINE_UNIFORM_LIGHT_COLOR,
        KRENGINE_UNIFORM_LIGHT_DECAY_START,
        KRENGINE_UNIFORM_LIGHT_CUTOFF,
        KRENGINE_UNIFORM_LIGHT_INTENSITY,
        KRENGINE_UNIFORM_FLARE_SIZE,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_INVP,
        KRENGINE_UNIFORM_MN2V,
        KRENGINE_UNIFORM_M2V,
        KRENGINE_UNIFORM_V2M,
        KRENGINE_UNIFORM_SHADOWMVP1,
        KRENGINE_UNIFORM_SHADOWMVP2,
        KRENGINE_UNIFORM_SHADOWMVP3,
        
        KRENGINE_UNIFORM_CAMERAPOS,
        KRENGINE_UNIFORM_VIEWPORT,
        KRENGINE_NUM_UNIFORMS
    };
    GLint m_shadowUniforms[KRENGINE_NUM_UNIFORMS];
    
    GLuint m_postShaderProgram;
    GLuint m_shadowShaderProgram;
    
    void renderPost(KRContext &context);
    void bindPostShader(KRContext &context);
    
    void destroyBuffers();
    
    void renderFrame(KRContext &context, KRScene &scene, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRVector3 &cameraPosition);
    
    void loadShaders(KRContext &context);
    
    // Code using these shader functions will later be refactored to integrate with KRShaderManager
    static bool ValidateProgram(GLuint prog);
    static bool LoadShader(KRContext &context, const std::string &name, GLuint *programPointer, const std::string &options);
    static bool CompileShader(GLuint *shader, GLenum type, const std::string &shader_source, const std::string &options);
    static bool LinkProgram(GLuint prog);
    
};

#endif
