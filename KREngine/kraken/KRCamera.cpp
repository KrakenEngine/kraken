//
//  KRCamera.cpp
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

#include "KREngine-common.h"
#include "KRVector2.h"
#include "KRCamera.h"
#include "KRStockGeometry.h"
#include "KRDirectionalLight.h"

KRCamera::KRCamera(KRScene &scene, std::string name) : KRNode(scene, name) {
    m_particlesAbsoluteTime = 0.0f;
    backingWidth = 0;
    backingHeight = 0;
    volumetricBufferWidth = 0;
    volumetricBufferHeight = 0;
    m_pSkyBoxTexture = NULL;
    
    compositeDepthTexture = 0;
    compositeColorTexture = 0;
    lightAccumulationTexture = 0;
    compositeFramebuffer = 0;
    lightAccumulationBuffer = 0;
    
    volumetricLightAccumulationBuffer = 0;
    volumetricLightAccumulationTexture = 0;
}

KRCamera::~KRCamera() {
    destroyBuffers();
}

void KRCamera::renderFrame(float deltaTime, GLint renderBufferWidth, GLint renderBufferHeight)
{
    GLint defaultFBO;
    GLDEBUG(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO));
    
    createBuffers(renderBufferWidth, renderBufferHeight);
    
    KRScene &scene = getScene();
    


    KRMat4 viewMatrix = KRMat4::Invert(getModelMatrix());
    getContext().getAudioManager()->setViewMatrix(viewMatrix); // FINDME, TODO - Should we support de-coupling the audio listener location from the camera?
    

    
    settings.setViewportSize(KRVector2(backingWidth, backingHeight));
    KRMat4 projectionMatrix;
    projectionMatrix.perspective(settings.perspective_fov, settings.m_viewportSize.x / settings.m_viewportSize.y, settings.perspective_nearz, settings.perspective_farz);
    m_viewport = KRViewport(settings.getViewportSize(), viewMatrix, projectionMatrix);

    KRVector3 vecCameraDirection = m_viewport.getCameraDirection();
    
    if(settings.bEnableDeferredLighting) {
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
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_DEFERRED_GBUFFER, true);
        
        // ----====---- Generate Shadowmaps for Lights ----====----
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_GENERATE_SHADOWMAPS, false);
        GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
        
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
        m_pContext->getTextureManager()->selectTexture(6, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
        m_pContext->getTextureManager()->selectTexture(7, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE7));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_DEFERRED_LIGHTS, false);
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
        
        // Set source to buffers from pass 2
        m_pContext->getTextureManager()->selectTexture(6, NULL);
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
        // TODO: At this point, we only want to render octree nodes that produced fragments during the 1st pass into the GBuffer
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_DEFERRED_OPAQUE, false);
        
        // Deactivate source buffer texture units
        m_pContext->getTextureManager()->selectTexture(6, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        m_pContext->getTextureManager()->selectTexture(7, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE7));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    } else {
        // ----====---- Generate Shadowmaps for Lights ----====----
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_GENERATE_SHADOWMAPS, true);
        // ----====---- Opaque Geometry, Forward Rendering ----====----
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
        
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
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_FORWARD_OPAQUE, false);
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
    
    if(!m_pSkyBoxTexture && settings.m_skyBoxName.length()) {
        m_pSkyBoxTexture = getContext().getTextureManager()->getTextureCube(settings.m_skyBoxName.c_str());
    }
    
    if(m_pSkyBoxTexture) {
        getContext().getShaderManager()->selectShader("sky_box", *this, std::vector<KRLight *>(), 0, m_viewport, KRMat4(), false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_OPAQUE);

        getContext().getTextureManager()->selectTexture(0, m_pSkyBoxTexture);
        
        // Render a full screen quad
        m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, NULL, 0, true, false, false, true, false, false, false);
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
    // Disable z-buffer write
    GLDEBUG(glDepthMask(GL_FALSE));
//    
//    // Enable z-buffer test
//    GLDEBUG(glEnable(GL_DEPTH_TEST));
//    GLDEBUG(glDepthFunc(GL_LEQUAL));
//    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    // Enable alpha blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Render all transparent geometry
    scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, false);
    
    // ----====---- Particle Occlusion Tests ----====----
    
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
    
    // Enable additive blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
    
    // ----====---- Perform Occlusion Tests ----====----
    scene.render(this, m_viewport.getVisibleBounds(), m_viewport, RENDER_PASS_PARTICLE_OCCLUSION, false);
    
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
    scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_ADDITIVE_PARTICLES, false);
    
    // ----====---- Volumetric Lighting ----====----
    
    if(settings.volumetric_environment_enable) {
        KRViewport volumetricLightingViewport = KRViewport(KRVector2(volumetricBufferWidth, volumetricBufferHeight), m_viewport.getViewMatrix(), m_viewport.getProjectionMatrix());
        
        if(settings.volumetric_environment_downsample != 0) {
            // Set render target
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, volumetricLightAccumulationBuffer));
            GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
            GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
            
            // Disable z-buffer test
            GLDEBUG(glDisable(GL_DEPTH_TEST));
            
            m_pContext->getTextureManager()->selectTexture(0, NULL);
            GLDEBUG(glActiveTexture(GL_TEXTURE0));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
            
            GLDEBUG(glViewport(0, 0, volumetricLightingViewport.getSize().x, volumetricLightingViewport.getSize().y));
        } else {
            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthFunc(GL_LEQUAL));
            GLDEBUG(glDepthRangef(0.0, 1.0));
        }
        
        scene.render(this, m_viewport.getVisibleBounds(), volumetricLightingViewport, KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE, false);
        
        if(settings.volumetric_environment_downsample != 0) {
            // Set render target
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
            
            GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
        }
    }

    
    
    // ----====---- Debug Overlay ----====----
    
    if(settings.bShowOctree) {
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable additive blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
        
        
        KRShader *pVisShader = getContext().getShaderManager()->getShader("visualize_overlay", this, std::vector<KRLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        
        m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_3D_CUBE, KRENGINE_VBO_3D_CUBE_SIZE, NULL, 0, true, false, false, false, false, false, false);
        for(std::map<KRAABB, int>::iterator itr=m_viewport.getVisibleBounds().begin(); itr != m_viewport.getVisibleBounds().end(); itr++) {
            KRMat4 matModel = KRMat4();
            matModel.scale((*itr).first.size() / 2.0f);
            matModel.translate((*itr).first.center());
            
            if(getContext().getShaderManager()->selectShader(*this, pVisShader, m_viewport, matModel, std::vector<KRLight *>(), 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
            }
        }
    }
    
    // Re-enable z-buffer write
    GLDEBUG(glDepthMask(GL_TRUE));
    

//    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i/%i Kbyte (active/total)     Shader Handles: %i   Visible Bounds: %i  Max Texture LOD: %i\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getActiveMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024, (int)m_pContext->getShaderManager()->getShaderHandlesUsed(), (int)m_visibleBounds.size(), m_pContext->getTextureManager()->getLODDimCap());
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO));
    renderPost();
}


void KRCamera::createBuffers(GLint renderBufferWidth, GLint renderBufferHeight) {
    
    if(renderBufferWidth != backingWidth || renderBufferHeight != backingHeight) {
        backingWidth = renderBufferWidth;
        backingHeight = renderBufferHeight;
        
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
    }
    
    int targetVolumetricBufferWidth = 0;
    int targetVolumetricBufferHeight = 0;
    if(settings.volumetric_environment_enable && settings.volumetric_environment_downsample != 0) {
        targetVolumetricBufferWidth = renderBufferWidth >> settings.volumetric_environment_downsample;
        targetVolumetricBufferHeight = renderBufferHeight >> settings.volumetric_environment_downsample;
    }
    
    
    if(targetVolumetricBufferWidth != volumetricBufferWidth || targetVolumetricBufferHeight != volumetricBufferHeight) {
        volumetricBufferWidth = targetVolumetricBufferWidth;
        volumetricBufferHeight = targetVolumetricBufferHeight;
        
        if (volumetricLightAccumulationTexture) {
            GLDEBUG(glDeleteTextures(1, &volumetricLightAccumulationTexture));
            volumetricLightAccumulationTexture = 0;
        }
        
        if (volumetricLightAccumulationBuffer) {
            GLDEBUG(glDeleteFramebuffers(1, &volumetricLightAccumulationBuffer));
            volumetricLightAccumulationBuffer = 0;
        }
        
        
        if(targetVolumetricBufferWidth != 0 && targetVolumetricBufferHeight != 0) {
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
            GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, volumetricBufferWidth, volumetricBufferHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, volumetricLightAccumulationTexture, 0));
        }
    }
}

void KRCamera::destroyBuffers()
{
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
    KRShader *postShader = m_pContext->getShaderManager()->getShader("PostShader", this, std::vector<KRLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    getContext().getShaderManager()->selectShader(*this, postShader, m_viewport, KRMat4(), std::vector<KRLight *>(), 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    m_pContext->getTextureManager()->selectTexture(0, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
    
    if(settings.volumetric_environment_enable) {
        m_pContext->getTextureManager()->selectTexture(2, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE2));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, volumetricLightAccumulationTexture));
    }
	
	// Update attribute values.
    m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, NULL, 0, true, false, false, true, false, false, false);
	
    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    
    m_pContext->getTextureManager()->selectTexture(0, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    
//    if(bShowShadowBuffer) {
//        KRShader *blitShader = m_pContext->getShaderManager()->getShader("simple_blit", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
//        
//        for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
//            KRMat4 viewMatrix = KRMat4();
//            viewMatrix.scale(0.20, 0.20, 0.20);
//            viewMatrix.translate(-0.70, 0.70 - 0.45 * iShadow, 0.0);
//            getContext().getShaderManager()->selectShader(blitShader, KRViewport(getViewportSize(), viewMatrix, KRMat4()), shadowViewports, KRMat4(), KRVector3(), NULL, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
//            m_pContext->getTextureManager()->selectTexture(1, NULL);
//            m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, NULL, 0, true, false, false, true, false);
//            GLDEBUG(glActiveTexture(GL_TEXTURE0));
//            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]));
//#if GL_EXT_shadow_samplers
//            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
//#endif
//            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
//#if GL_EXT_shadow_samplers
//            GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT)); // TODO - Detect GL_EXT_shadow_samplers and only activate if available
//#endif
//        }
//        
//        m_pContext->getTextureManager()->selectTexture(0, NULL);
//        GLDEBUG(glActiveTexture(GL_TEXTURE0));
//        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
//    }
    
    
    
    const char *szText = settings.m_debug_text.c_str();
    if(*szText) {
        KRShader *fontShader = m_pContext->getShaderManager()->getShader("debug_font", this, std::vector<KRLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);

        m_pContext->getTextureManager()->selectTexture(0, m_pContext->getTextureManager()->getTexture("font"));
        
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
#elif GL_vertex_array_object
#endif
            m_pContext->getModelManager()->configureAttribs(true, false, false, true, false, false, false);
            GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, charTexCoords));
            GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, charVertices));
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            
            iPos++;
        }
        
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        
        m_pContext->getTextureManager()->selectTexture(1, NULL);
    }
}

