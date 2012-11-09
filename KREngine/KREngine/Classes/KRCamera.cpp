//
//  KRSettings.cpp
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

#import <string>
#include <iostream>
#include <sstream>
#include <fstream>
#import <assert.h>

#import "KRVector2.h"
#import "KRCamera.h"
#import "KRBoundingVolume.h"
#import "KRStockGeometry.h"
#import "KRDirectionalLight.h"

KRCamera::KRCamera(KRContext &context) : KRContextObject(context) {
    m_particlesAbsoluteTime = 0.0f;
    backingWidth = 0;
    backingHeight = 0;
    
    
    float const PI = 3.141592653589793f;
    float const D2R = PI * 2 / 360;
    
    bShowShadowBuffer = false;
    bShowOctree = false;
    bShowDeferred = false;
    bEnablePerPixel = true;
    bEnableDiffuseMap = true;
    bEnableNormalMap = true;
    bEnableSpecMap = true;
    bEnableReflectionMap = true;
    bEnableReflection = true;
    bDebugPSSM = false;
    bEnableAmbient = true;
    bEnableDiffuse = true;
    bEnableSpecular = true;
    bEnableLightMap = true;
    bDebugSuperShiny = false;
    bEnableDeferredLighting = true;
    
    
    dAmbientR = 0.0f;
    dAmbientG = 0.0f;
    dAmbientB = 0.0f;
    
    dSunR = 1.0f;
    dSunG = 1.0f;
    dSunB = 1.0f;
    
    perspective_fov = 45.0 * D2R;
    perspective_nearz = 5.0f;
    perspective_farz = 100.0f;
    
    dof_quality = 0;
    dof_depth = 0.05f;
    dof_falloff = 0.05f;
    
    bEnableFlash = false;
    flash_intensity = 1.0f;
    flash_depth = 0.7f;
    flash_falloff = 0.5f;
    
    
    bEnableVignette = false;
    vignette_radius = 0.4f;
    vignette_falloff = 1.0f;
    
    
    m_cShadowBuffers = 0;
    compositeDepthTexture = 0;
    compositeColorTexture = 0;
    lightAccumulationTexture = 0;
    compositeFramebuffer = 0;
    lightAccumulationBuffer = 0;
    
    volumetricLightAccumulationBuffer = 0;
    volumetricLightAccumulationTexture = 0;
    
    
    memset(shadowFramebuffer, sizeof(GLuint) * 3, 0);
    memset(shadowDepthTexture, sizeof(GLuint) * 3, 0);
    
    m_iFrame = 0;
    
    m_skyBoxName = "";
    m_pSkyBoxTexture = NULL;
    
    volumetric_light_downsample = 4;
}

KRCamera::~KRCamera() {
    destroyBuffers();
}

KRMat4 KRCamera::getProjectionMatrix() {
    KRMat4 projectionMatrix;
    projectionMatrix.perspective(perspective_fov, m_viewportSize.x / m_viewportSize.y, perspective_nearz, perspective_farz);
    return projectionMatrix;
}

const KRVector2 &KRCamera::getViewportSize() {
    return m_viewportSize;
}

void KRCamera::setViewportSize(const KRVector2 &size) {
    m_viewportSize = size;
}

KRVector3 KRCamera::getPosition() const {
    return m_position;
}

void KRCamera::setPosition(const KRVector3 &position) {
    m_position = position;
}

void KRCamera::renderFrame(KRScene &scene, KRMat4 &viewMatrix, float deltaTime)
{
    GLint defaultFBO;
    GLDEBUG(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO));
    
    createBuffers();
    
    setViewportSize(KRVector2(backingWidth, backingHeight));
    m_viewport = KRViewport(getViewportSize(), viewMatrix, getProjectionMatrix());

    m_pContext->rotateBuffers(true);
    
    KRDirectionalLight *firstDirectionalLight = scene.getFirstDirectionalLight();
    KRVector3 lightDirection = KRVector3::Normalize(KRVector3(0.90, 0.70, 0.25));
    if(firstDirectionalLight) {
        lightDirection = firstDirectionalLight->getWorldLightDirection();
    }

    allocateShadowBuffers(m_cShadowBuffers);
    for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
        GLfloat shadowMinDepths[3][3] = {{0.0, 0.0, 0.0},{0.0, 0.0, 0.0},{0.0, 0.05, 0.3}};
        GLfloat shadowMaxDepths[3][3] = {{0.0, 0.0, 1.0},{0.1, 0.0, 0.0},{0.1, 0.3, 1.0}};
        
        
        
        
        
        KRVector3 shadowLook = -KRVector3::Normalize(lightDirection);
        
        KRVector3 shadowUp(0.0, 1.0, 0.0);
        if(KRVector3::Dot(shadowUp, shadowLook) > 0.99f) shadowUp = KRVector3(0.0, 0.0, 1.0); // Ensure shadow look direction is not parallel with the shadowUp direction
        
        KRMat4 matShadowView = KRMat4::LookAt(m_viewport.getCameraPosition() - shadowLook, m_viewport.getCameraPosition(), shadowUp);
        
        
        KRMat4 matShadowProjection = KRMat4();
        matShadowProjection.scale(0.001, 0.001, 0.001);
        
        KRMat4 matBias;
        matBias.bias();
        matShadowProjection *= matBias;
        

        
        
//        KRAABB frustrumWorldBounds = KRAABB(KRVector3(-1.0, -1.0, 0.0), KRVector3(1.0, 1.0, 1.0), m_viewport.getInverseProjectionMatrix() * m_viewport.getInverseViewMatrix());
//        matShadowProjection.translate(KRVector3(0.0, 0.0, -frustrumWorldBounds.size().z));
//        matShadowProjection.scale(2.0 / frustrumWorldBounds.size().x, 2.0 / frustrumWorldBounds.size().y, 2.0 / frustrumWorldBounds.size().z);
//        matShadowView.translate(-m_viewport.getCameraPosition());
        
        //matShadowProjection *= matBias;
        
        m_shadowViewports[iShadow] = KRViewport(KRVector2(KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT), matShadowView, matShadowProjection);
        
//        KRMat4 newShadowMVP;
//        if(shadowMaxDepths[m_cShadowBuffers - 1][iShadow] == 0.0) {
//            KRBoundingVolume ext = KRBoundingVolume(-KRVector3::One(), KRVector3::One(), KRMat4()); // HACK - Temporary workaround to compile until this logic is updated to use information from the Octree
//            
//            newShadowMVP = ext.calcShadowProj(&scene, m_pContext, scene.sun_yaw, scene.sun_pitch);
//        } else {
//            KRBoundingVolume frustrumSliceVolume = KRBoundingVolume(viewMatrix, perspective_fov, getViewportSize().x / getViewportSize().y, perspective_nearz + (perspective_farz - perspective_nearz) * shadowMinDepths[m_cShadowBuffers - 1][iShadow], perspective_nearz + (perspective_farz - perspective_nearz) * shadowMaxDepths[m_cShadowBuffers - 1][iShadow]);
//            newShadowMVP = frustrumSliceVolume.calcShadowProj(&scene, m_pContext, scene.sun_yaw, scene.sun_pitch);
//        }

//        KRBoundingVolume frustrumSliceVolume = KRBoundingVolume(viewMatrix, perspective_fov, getViewportSize().x / getViewportSize().y, perspective_nearz + (perspective_farz - perspective_nearz) * shadowMinDepths[m_cShadowBuffers - 1][iShadow], perspective_nearz + (perspective_farz - perspective_nearz) * shadowMaxDepths[m_cShadowBuffers - 1][iShadow]);
//        newShadowMVP = frustrumSliceVolume.calcShadowProj(&scene, m_pContext, scene.sun_yaw, scene.sun_pitch);
//        
//        m_shadowViewports[iShadow] = KRViewport(KRVector2(KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT), KRMat4(), newShadowMVP);
//        
        renderShadowBuffer(scene, iShadow);

    }
    
    renderFrame(scene, lightDirection, deltaTime);
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO));
    renderPost();
    
    m_iFrame++;
}




void KRCamera::renderFrame(KRScene &scene, KRVector3 &lightDirection, float deltaTime) {
    
    KRVector3 vecCameraDirection = m_viewport.getCameraDirection();
    
    std::set<KRAABB> newVisibleBounds;
    
    if(bEnableDeferredLighting) {
        //  ----====---- Opaque Geometry, Deferred rendering Pass 1 ----====----
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_DEFERRED_GBUFFER, newVisibleBounds);
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 2 ----====----
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, lightAccumulationBuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
        
        // Enable additive blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
        
        // Disable z-buffer write
        GLDEBUG(glDepthMask(GL_FALSE));
        
        // Set source to buffers from pass 1
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
        m_pContext->getTextureManager()->selectTexture(7, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE7));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, 0, KRNode::RENDER_PASS_DEFERRED_LIGHTS, newVisibleBounds);
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
        
        // Set source to buffers from pass 2
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, lightAccumulationTexture));
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        // Enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        // Render the geometry
        std::set<KRAABB> emptyBoundsSet; // At this point, we only render octree nodes that produced fragments during the 1st pass into the GBuffer
        scene.render(this, emptyBoundsSet, m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_DEFERRED_OPAQUE, newVisibleBounds);
        
        // Deactivate source buffer texture units
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        m_pContext->getTextureManager()->selectTexture(7, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE7));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    } else {
        // ----====---- Opaque Geometry, Forward Rendering ----====----
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_FORWARD_OPAQUE, newVisibleBounds);
    }
    
    // ----====---- Sky Box ----====----
    
    // Set render target
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
    GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
    
    // Disable backface culling
    GLDEBUG(glDisable(GL_CULL_FACE));
    
    // Disable z-buffer write
    GLDEBUG(glDepthMask(GL_FALSE));
    
    // Enable z-buffer test
    GLDEBUG(glEnable(GL_DEPTH_TEST));
    GLDEBUG(glDepthFunc(GL_LEQUAL));
    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    if(!m_pSkyBoxTexture && m_skyBoxName.length()) {
        m_pSkyBoxTexture = getContext().getTextureManager()->getTextureCube(m_skyBoxName.c_str());
    }
    
    if(m_pSkyBoxTexture) {
        KRShader *pShader = getContext().getShaderManager()->getShader("sky_box", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_OPAQUE);
        pShader->bind(m_viewport, m_shadowViewports, KRMat4(), lightDirection, NULL, 0, KRNode::RENDER_PASS_FORWARD_OPAQUE);
        
        getContext().getTextureManager()->selectTexture(0, m_pSkyBoxTexture, 2048);
        
        // Render a full screen quad
        m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
    
    
    // ----====---- Transparent Geometry, Forward Rendering ----====----

//    Note: These parameters have already been set up by the skybox render above
//
//    // Set render target
//    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
//    GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
//    
//    // Disable backface culling
//    GLDEBUG(glDisable(GL_CULL_FACE));
//    
//    // Disable z-buffer write
//    GLDEBUG(glDepthMask(GL_FALSE));
//    
//    // Enable z-buffer test
//    GLDEBUG(glEnable(GL_DEPTH_TEST));
//    GLDEBUG(glDepthFunc(GL_LEQUAL));
//    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    // Enable alpha blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Render all transparent geometry
    scene.render(this, m_viewport.getVisibleBounds(), m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, newVisibleBounds);
    
    
    // ----====---- Flares ----====----
    
    // Set render target
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
    GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
    
    // Disable backface culling
    GLDEBUG(glDisable(GL_CULL_FACE));
    
    // Disable z-buffer write
    GLDEBUG(glDepthMask(GL_FALSE));
    
    // Disable z-buffer test
    GLDEBUG(glDisable(GL_DEPTH_TEST));
    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    // Enable additive blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
    
    // Render all flares
    scene.render(this, m_viewport.getVisibleBounds(), m_pContext, m_viewport, m_shadowViewports, lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES, newVisibleBounds);
    
    // ----====---- Volumetric Lighting ----====----
    
    if(m_cShadowBuffers >= 1) {
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, volumetricLightAccumulationBuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
        
        KRShader *pFogShader = m_pContext->getShaderManager()->getShader("volumetric_fog", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        
        KRViewport volumetricLightingViewport = KRViewport(m_viewport.getSize() / volumetric_light_downsample, m_viewport.getViewMatrix(), m_viewport.getProjectionMatrix());
        
        if(pFogShader->bind(volumetricLightingViewport, m_shadowViewports, KRMat4(), lightDirection, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_ADDITIVE_PARTICLES)) {
            
            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthFunc(GL_LEQUAL));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            
            int slice_count = 50;
            
            float slice_near = -100.0;
            float slice_far = -1000.0;
            float slice_spacing = (slice_far - slice_near) / slice_count;
            
            KRVector2(slice_near, slice_spacing).setUniform(pFogShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SLICE_DEPTH_SCALE]);
            
            m_pContext->getModelManager()->bindVBO((void *)m_pContext->getModelManager()->getVolumetricLightingVertexes(), slice_count * 6 * sizeof(KRModelManager::VolumetricLightingVertexData), true, false, false, false, false);
            GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, slice_count*6));
        }
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
    }

    
    
    // ----====---- Debug Overlay ----====----
    
    if(bShowOctree) {
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable additive blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
        
        
        KRShader *pVisShader = m_pContext->getShaderManager()->getShader("visualize_overlay", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
        KRMat4 projectionMatrix = getProjectionMatrix();
        
        m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_3D_CUBE, KRENGINE_VBO_3D_CUBE_SIZE, true, false, false, false, false);
        for(std::set<KRAABB>::iterator itr=m_viewport.getVisibleBounds().begin(); itr != m_viewport.getVisibleBounds().end(); itr++) {
            KRMat4 matModel = KRMat4();
            matModel.scale((*itr).size() / 2.0f);
            matModel.translate((*itr).center());
            if(pVisShader->bind(m_viewport, m_shadowViewports, matModel, lightDirection, shadowDepthTexture, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
            }
        }
    }
    
    m_viewport.setVisibleBounds(newVisibleBounds);
    
    // Re-enable z-buffer write
    GLDEBUG(glDepthMask(GL_TRUE));
    

//    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i/%i Kbyte (active/total)     Shader Handles: %i   Visible Bounds: %i  Max Texture LOD: %i\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getActiveMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024, (int)m_pContext->getShaderManager()->getShaderHandlesUsed(), (int)m_visibleBounds.size(), m_pContext->getTextureManager()->getLODDimCap());
}


void KRCamera::createBuffers() {
    GLint renderBufferWidth = 0, renderBufferHeight = 0;
    GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &renderBufferWidth));
    GLDEBUG(glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &renderBufferHeight));
    
    if(renderBufferWidth != backingWidth || renderBufferHeight != backingHeight) {
        backingWidth = renderBufferWidth;
        backingHeight = renderBufferHeight;

        destroyBuffers();
        
        // ===== Create offscreen compositing framebuffer object =====
        GLDEBUG(glGenFramebuffers(1, &compositeFramebuffer));
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        
        // ----- Create texture color buffer for compositeFramebuffer -----
        GLDEBUG(glGenTextures(1, &compositeColorTexture));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backingWidth, backingHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, compositeColorTexture, 0));
        
        // ----- Create Depth Texture for compositeFramebuffer -----
        GLDEBUG(glGenTextures(1, &compositeDepthTexture));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
        //GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, backingWidth, backingHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
        //GLDEBUG(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, backingWidth, backingHeight));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // ===== Create offscreen compositing framebuffer object =====
        GLDEBUG(glGenFramebuffers(1, &lightAccumulationBuffer));
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, lightAccumulationBuffer));
        
        // ----- Create texture color buffer for compositeFramebuffer -----
        GLDEBUG(glGenTextures(1, &lightAccumulationTexture));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, lightAccumulationTexture));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backingWidth, backingHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightAccumulationTexture, 0));
        
        
        // ===== Create offscreen compositing framebuffer object for volumetric lighting =====
        GLDEBUG(glGenFramebuffers(1, &volumetricLightAccumulationBuffer));
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, volumetricLightAccumulationBuffer));
        
        // ----- Create texture color buffer for compositeFramebuffer for volumetric lighting  -----
        GLDEBUG(glGenTextures(1, &volumetricLightAccumulationTexture));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, volumetricLightAccumulationTexture));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE)); // This is necessary for non-power-of-two textures
        GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backingWidth / volumetric_light_downsample, backingHeight / volumetric_light_downsample, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, volumetricLightAccumulationTexture, 0));
        
        allocateShadowBuffers(m_cShadowBuffers);
    }
}

void KRCamera::allocateShadowBuffers(int cBuffers) {
    // First deallocate buffers no longer needed
    for(int iShadow = cBuffers; iShadow < KRENGINE_MAX_SHADOW_BUFFERS; iShadow++) {
        if (shadowDepthTexture[iShadow]) {
            GLDEBUG(glDeleteTextures(1, shadowDepthTexture + iShadow));
            shadowDepthTexture[iShadow] = 0;
        }
        
        if (shadowFramebuffer[iShadow]) {
            GLDEBUG(glDeleteFramebuffers(1, shadowFramebuffer + iShadow));
            shadowFramebuffer[iShadow] = 0;
        }
    }
    
    // Allocate newly required buffers
    for(int iShadow = 0; iShadow < cBuffers; iShadow++) {
        if(!shadowDepthTexture[iShadow]) {
            shadowValid[iShadow] = false;
            
            GLDEBUG(glGenFramebuffers(1, shadowFramebuffer + iShadow));
            GLDEBUG(glGenTextures(1, shadowDepthTexture + iShadow));
            // ===== Create offscreen shadow framebuffer object =====
            
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
            
            // ----- Create Depth Texture for shadowFramebuffer -----
           GLDEBUG( glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
#if GL_EXT_shadow_samplers
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
#endif
            GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
            
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0));
        }
    }
}

void KRCamera::destroyBuffers()
{
    allocateShadowBuffers(0);
    
    if (compositeDepthTexture) {
        GLDEBUG(glDeleteTextures(1, &compositeDepthTexture));
        compositeDepthTexture = 0;
    }
    
    if (compositeColorTexture) {
		GLDEBUG(glDeleteTextures(1, &compositeColorTexture));
		compositeColorTexture = 0;
	}
    
    if (lightAccumulationTexture) {
        GLDEBUG(glDeleteTextures(1, &lightAccumulationTexture));
        lightAccumulationTexture = 0;
    }
    
    if (compositeFramebuffer) {
        GLDEBUG(glDeleteFramebuffers(1, &compositeFramebuffer));
        compositeFramebuffer = 0;
    }
    
    if (lightAccumulationBuffer) {
        GLDEBUG(glDeleteFramebuffers(1, &lightAccumulationBuffer));
        lightAccumulationBuffer = 0;
    }
    
    
    
    if (volumetricLightAccumulationTexture) {
        GLDEBUG(glDeleteTextures(1, &volumetricLightAccumulationTexture));
        volumetricLightAccumulationTexture = 0;
    }
    
    if (volumetricLightAccumulationBuffer) {
        GLDEBUG(glDeleteFramebuffers(1, &volumetricLightAccumulationBuffer));
        volumetricLightAccumulationBuffer = 0;
    }
}


void KRCamera::renderShadowBuffer(KRScene &scene, int iShadow)
{
    
    glViewport(0, 0, m_shadowViewports[iShadow].getSize().x, m_shadowViewports[iShadow].getSize().y);
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
    GLDEBUG(glClearDepthf(1.0f));
    GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
    
    glViewport(1, 1, m_shadowViewports[iShadow].getSize().x - 2, m_shadowViewports[iShadow].getSize().y - 2);
    
    GLDEBUG(glDisable(GL_DITHER));
    
    GLDEBUG(glCullFace(GL_BACK)); // Enable frontface culling, which eliminates some self-cast shadow artifacts
    GLDEBUG(glEnable(GL_CULL_FACE));
    
    // Enable z-buffer test
    GLDEBUG(glEnable(GL_DEPTH_TEST));
	GLDEBUG(glDepthFunc(GL_LESS));
    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    // Disable alpha blending as we are using alpha channel for packed depth info
    GLDEBUG(glDisable(GL_BLEND));
    
    // Use shader program
    KRShader *shadowShader = m_pContext->getShaderManager()->getShader("ShadowShader", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    shadowShader->bind(m_shadowViewports[iShadow], m_shadowViewports, KRMat4(), KRVector3(), NULL, 0, KRNode::RENDER_PASS_SHADOWMAP);
    
//    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
//    shadowmvpmatrix[iShadow].setUniform(shadowShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SHADOWMVP1]);
    
//    // Calculate the bounding volume of the light map
//    KRMat4 matInvShadow = shadowmvpmatrix[iShadow];
//    matInvShadow.invert();
//    
//    KRVector3 vertices[8];
//    vertices[0] = KRVector3(-1.0, -1.0, 0.0);
//    vertices[1] = KRVector3(1.0,  -1.0, 0.0);
//    vertices[2] = KRVector3(1.0,   1.0, 0.0);
//    vertices[3] = KRVector3(-1.0,  1.0, 0.0);
//    vertices[4] = KRVector3(-1.0, -1.0, 1.0);
//    vertices[5] = KRVector3(1.0,  -1.0, 1.0);
//    vertices[6] = KRVector3(1.0,   1.0, 1.0);
//    vertices[7] = KRVector3(-1.0,  1.0, 1.0);
//    
//    for(int iVertex=0; iVertex < 8; iVertex++) {
//        vertices[iVertex] = KRMat4::Dot(matInvShadow, vertices[iVertex]);
//    }
//    
//    KRVector3 cameraPosition;
//    KRVector3 lightDirection;
//    KRBoundingVolume shadowVolume = KRBoundingVolume(vertices);
    
    
    KRVector3 temp = KRVector3();
    std::set<KRAABB> newVisibleBounds;
    scene.render(this, m_shadowViewports[iShadow].getVisibleBounds(), m_pContext, m_shadowViewports[iShadow], m_shadowViewports, temp, NULL, m_cShadowBuffers, KRNode::RENDER_PASS_SHADOWMAP, newVisibleBounds);
    
    m_shadowViewports[iShadow].setVisibleBounds(newVisibleBounds);
    GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
}

void KRCamera::renderPost()
{
    // Disable alpha blending
    GLDEBUG(glDisable(GL_BLEND));
    

    
    static const GLfloat squareVerticesShadow[3][8] = {{
        -1.0f, -1.0f,
        -0.60f, -1.0f,
        -1.0f,  -0.60f,
        -0.60f,  -0.60f,
    },{
        -0.50f, -1.0f,
        -0.10f, -1.0f,
        -0.50f,  -0.60f,
        -0.10f,  -0.60f,
    },{
        0.00f, -1.0f,
        0.40f, -1.0f,
        0.00f,  -0.60f,
        0.40f,  -0.60f,
    }};
	

	
    GLDEBUG(glDisable(GL_DEPTH_TEST));
    KRShader *postShader = m_pContext->getShaderManager()->getShader("PostShader", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    postShader->bind(m_viewport, m_shadowViewports, KRMat4(), KRVector3(), NULL, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    m_pContext->getTextureManager()->selectTexture(0, NULL, 0);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL, 0);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
    
    m_pContext->getTextureManager()->selectTexture(2, NULL, 0);
    GLDEBUG(glActiveTexture(GL_TEXTURE2));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, volumetricLightAccumulationTexture));
	
	// Update attribute values.
    m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
	
    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    
    m_pContext->getTextureManager()->selectTexture(0, NULL, 0);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL, 0);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    
    if(bShowShadowBuffer) {
        KRShader *blitShader = m_pContext->getShaderManager()->getShader("simple_blit", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        
        for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
            KRMat4 viewMatrix = KRMat4();
            viewMatrix.scale(0.20, 0.20, 0.20);
            viewMatrix.translate(-0.70, 0.70 - 0.45 * iShadow, 0.0);
            blitShader->bind(KRViewport(getViewportSize(), viewMatrix, KRMat4()), m_shadowViewports, KRMat4(), KRVector3(), NULL, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
            m_pContext->getTextureManager()->selectTexture(1, NULL, 0);
            m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
            GLDEBUG(glActiveTexture(GL_TEXTURE0));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]));
#if GL_EXT_shadow_samplers
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
#endif
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
#if GL_EXT_shadow_samplers
            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
#endif
        }
        
        m_pContext->getTextureManager()->selectTexture(0, NULL, 0);
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    }
    
    
    
    const char *szText = m_debug_text.c_str();
    if(*szText) {
        KRShader *fontShader = m_pContext->getShaderManager()->getShader("debug_font", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);

        m_pContext->getTextureManager()->selectTexture(0, m_pContext->getTextureManager()->getTexture("font"), 2048);
        
        const char *pChar = szText;
        int iPos=0;
        float dScale = 1.0 / 24.0;
        float dTexScale = 1.0 / 16.0;
        while(*pChar) {
            int iChar = *pChar++ - '\0';
            int iCol = iChar % 16;
            int iRow = 15 - (iChar - iCol) / 16;
            
            GLfloat charVertices[] = {
                -1.0f,              dScale * iPos - 1.0,
                -1.0 + dScale,      dScale * iPos - 1.0,
                -1.0f,              dScale * iPos + dScale - 1.0,
                -1.0 + dScale,      dScale * iPos + dScale - 1.0,
            };
            
            GLfloat charTexCoords[] = {
                dTexScale * iCol,                 dTexScale * iRow + dTexScale,
                dTexScale * iCol,                 dTexScale * iRow,
                dTexScale * iCol + dTexScale,     dTexScale * iRow + dTexScale,
                dTexScale * iCol + dTexScale,     dTexScale * iRow
            };
#if GL_OES_vertex_array_object
            GLDEBUG(glBindVertexArrayOES(0));
#endif
            m_pContext->getModelManager()->configureAttribs(true, false, false, true, false);
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, charTexCoords));
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, charVertices));
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            
            iPos++;
        }
        
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        
        m_pContext->getTextureManager()->selectTexture(1, NULL, 0);
    }
    
}

void KRCamera::invalidateShadowBuffers() {
    for(int i=0; i < m_cShadowBuffers; i++) {
        shadowValid[i] = false;
    }
}

void KRCamera::setSkyBox(const std::string &skyBoxName) {
    m_pSkyBoxTexture = NULL;
    m_skyBoxName = skyBoxName;
}

float KRCamera::getPerspectiveNearZ()
{
    return perspective_nearz;
}
float KRCamera::getPerspectiveFarZ()
{
    return perspective_farz;
}
void KRCamera::setPerspectiveNear(float v)
{
    if(perspective_nearz != v) {
        perspective_nearz = v;
        invalidateShadowBuffers();
    }
}
void KRCamera::setPerpsectiveFarZ(float v)
{
    if(perspective_farz != v) {
        perspective_farz = v;
        invalidateShadowBuffers();
    }
}
