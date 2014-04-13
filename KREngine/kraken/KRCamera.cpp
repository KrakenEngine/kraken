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
    m_last_frame_start = 0;
    
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
    m_frame_times_filled = 0;
}

KRCamera::~KRCamera() {
    destroyBuffers();
}

std::string KRCamera::getElementName() {
    return "camera";
}

tinyxml2::XMLElement *KRCamera::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    
    return e;
}


void KRCamera::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}

void KRCamera::flushSkybox()
{
    m_pSkyBoxTexture = NULL; // NOTE: the texture manager manages the loading and unloading of the skybox textures
}

void KRCamera::renderFrame(float deltaTime, GLint renderBufferWidth, GLint renderBufferHeight)
{    
    // ----====---- Record timing information for measuring FPS ----====----
    uint64_t current_time = m_pContext->getAbsoluteTimeMilliseconds();
    if(m_last_frame_start != 0) {
        m_frame_times[m_pContext->getCurrentFrame() % KRAKEN_FPS_AVERAGE_FRAME_COUNT] = (current_time - m_last_frame_start);
        if(m_frame_times_filled < KRAKEN_FPS_AVERAGE_FRAME_COUNT) m_frame_times_filled++;
    }
    m_last_frame_start = current_time;
    
    GLint defaultFBO;
    GLDEBUG(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO));
    
    createBuffers(renderBufferWidth, renderBufferHeight);
    
    KRScene &scene = getScene();
    
    KRMat4 modelMatrix = getModelMatrix();
    KRMat4 viewMatrix = KRMat4::LookAt(KRMat4::Dot(modelMatrix, KRVector3::Zero()), KRMat4::Dot(modelMatrix, KRVector3::Forward()), KRVector3::Normalize(KRMat4::DotNoTranslate(modelMatrix, KRVector3::Up())));
    
    //KRMat4 viewMatrix = KRMat4::Invert(getModelMatrix());
    
    settings.setViewportSize(KRVector2(backingWidth, backingHeight));
    KRMat4 projectionMatrix;
    projectionMatrix.perspective(settings.perspective_fov, settings.m_viewportSize.x / settings.m_viewportSize.y, settings.perspective_nearz, settings.perspective_farz);
    m_viewport = KRViewport(settings.getViewportSize(), viewMatrix, projectionMatrix);
    m_viewport.setLODBias(settings.getLODBias());

    KRVector3 vecCameraDirection = m_viewport.getCameraDirection();
    
    
    scene.updateOctree(m_viewport);
    
    // ----====---- Generate Shadowmaps for Lights ----====----
    if(settings.m_cShadowBuffers > 0) {
        GL_PUSH_GROUP_MARKER("Generate Shadowmaps");
        
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_GENERATE_SHADOWMAPS, settings.bEnableDeferredLighting);
        GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
        GL_POP_GROUP_MARKER;
    }
    
    if(settings.bEnableDeferredLighting) {
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 1 ----====----
        
        GL_PUSH_GROUP_MARKER("Deferred Lighting - Pass 1 (Opaque)");
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        
        
#if GL_EXT_discard_framebuffer
        GLenum attachments[2] = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
        GLDEBUG(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments));
#endif
        
        // Enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_DEFERRED_GBUFFER, true);
        
        
        GL_POP_GROUP_MARKER;
        
        
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 2 ----====----
        
        GL_PUSH_GROUP_MARKER("Deferred Lighting - Pass 2 (Opaque)");
        
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
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(6);
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
        m_pContext->getTextureManager()->selectTexture(7, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(7);
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_DEFERRED_LIGHTS, false);
        
        GL_POP_GROUP_MARKER;
        
        //  ----====---- Opaque Geometry, Deferred rendering Pass 3 ----====----
        
        GL_PUSH_GROUP_MARKER("Deferred Lighting - Pass 3 (Opaque)");
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
        
        // Set source to buffers from pass 2
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(6);
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
        m_pContext->getTextureManager()->selectTexture(6, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(6);
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        m_pContext->getTextureManager()->selectTexture(7, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(7);
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        
        GL_POP_GROUP_MARKER;
    } else {
        // ----====---- Opaque Geometry, Forward Rendering ----====----
        
        GL_PUSH_GROUP_MARKER("Forward Rendering - Opaque");
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        GLDEBUG(glViewport(0, 0, m_viewport.getSize().x, m_viewport.getSize().y));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        
        // Enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        
#if GL_EXT_discard_framebuffer
        GLenum attachments[2] = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
        GLDEBUG(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments));
        GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
#else
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
#endif
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        

        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        
        // Render the geometry
        scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_FORWARD_OPAQUE, true);
        
        GL_POP_GROUP_MARKER;
    }
    
    // ----====---- Sky Box ----====----
    
    
    GL_PUSH_GROUP_MARKER("Sky Box");
    
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
        KRVector3 rim_color;
        getContext().getShaderManager()->selectShader("sky_box", *this, std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, m_viewport, KRMat4(), false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_OPAQUE, rim_color, 0.0f);

        getContext().getTextureManager()->selectTexture(0, m_pSkyBoxTexture, 0.0f, KRTexture::TEXTURE_USAGE_SKY_CUBE);
        
        // Render a full screen quad
        m_pContext->getModelManager()->bindVBO(getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_VERTICES, getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_INDEXES, getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_ATTRIBS, true);
        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
    
    GL_POP_GROUP_MARKER;
    
    
    // ----====---- Transparent Geometry, Forward Rendering ----====----
    
    GL_PUSH_GROUP_MARKER("Forward Rendering - Transparent");

//    Note: These parameters have already been set up by the skybox render above
//
//    // Set render target
//    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
//    GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
//    
//    // Disable backface culling
//    GLDEBUG(glDisable(GL_CULL_FACE));
//
    // Enable backface culling
    GLDEBUG(glCullFace(GL_BACK));
    GLDEBUG(glEnable(GL_CULL_FACE));
    
    // Disable z-buffer write
    GLDEBUG(glDepthMask(GL_FALSE));
//    
//    // Enable z-buffer test
//    GLDEBUG(glEnable(GL_DEPTH_TEST));
//    GLDEBUG(glDepthFunc(GL_LEQUAL));
//    GLDEBUG(glDepthRangef(0.0, 1.0));
    
    // Enable alpha blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
    
    // Render all transparent geometry
    scene.render(this, m_viewport.getVisibleBounds(), m_viewport, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, false);
    
    GL_POP_GROUP_MARKER;
    
    // ----====---- Particle Occlusion Tests ----====----
    
    GL_PUSH_GROUP_MARKER("Particle Occlusion Tests");
    
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
    
    GL_POP_GROUP_MARKER;
    
    // ----====---- Flares ----====----
    
    GL_PUSH_GROUP_MARKER("Additive Particles");
    
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
    
    GL_POP_GROUP_MARKER;
    
    // ----====---- Volumetric Lighting ----====----
    
    if(settings.volumetric_environment_enable) {
        
        GL_PUSH_GROUP_MARKER("Volumetric Lighting");
        
        KRViewport volumetricLightingViewport = KRViewport(KRVector2(volumetricBufferWidth, volumetricBufferHeight), m_viewport.getViewMatrix(), m_viewport.getProjectionMatrix());
        
        if(settings.volumetric_environment_downsample != 0) {
            // Set render target
            GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, volumetricLightAccumulationBuffer));
            GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
            GLDEBUG(glClear(GL_COLOR_BUFFER_BIT));
            
            // Disable z-buffer test
            GLDEBUG(glDisable(GL_DEPTH_TEST));
            
            m_pContext->getTextureManager()->selectTexture(0, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
            m_pContext->getTextureManager()->_setActiveTexture(0);
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
        
        GL_POP_GROUP_MARKER;
    }

    
    
    // ----====---- Debug Overlay ----====----
    
    GL_PUSH_GROUP_MARKER("Debug Overlays");
    
    if(settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_OCTREE) {
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        // Enable additive blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
        
        
        KRShader *pVisShader = getContext().getShaderManager()->getShader("visualize_overlay", this, std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        
        m_pContext->getModelManager()->bindVBO(getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_VERTICES, getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_INDEXES, getContext().getModelManager()->KRENGINE_VBO_3D_CUBE_ATTRIBS, true);
        for(unordered_map<KRAABB, int>::iterator itr=m_viewport.getVisibleBounds().begin(); itr != m_viewport.getVisibleBounds().end(); itr++) {
            KRMat4 matModel = KRMat4();
            matModel.scale((*itr).first.size() * 0.5f);
            matModel.translate((*itr).first.center());
            KRVector3 rim_color;
            if(getContext().getShaderManager()->selectShader(*this, pVisShader, m_viewport, matModel, std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, rim_color, 0.0f)) {
                GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
            }
        }
    }
    
    // Re-enable z-buffer write
    GLDEBUG(glDepthMask(GL_TRUE));
    
    GL_POP_GROUP_MARKER;
    

//    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i/%i Kbyte (active/total)     Shader Handles: %i   Visible Bounds: %i  Max Texture LOD: %i\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getActiveMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024, (int)m_pContext->getShaderManager()->getShaderHandlesUsed(), (int)m_visibleBounds.size(), m_pContext->getTextureManager()->getLODDimCap());
    GL_PUSH_GROUP_MARKER("Post Processing");
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO));
    renderPost();
    m_pContext->getModelManager()->unbindVBO();
    
    GL_POP_GROUP_MARKER;
    
    
#if GL_EXT_discard_framebuffer
    GLenum attachments[2] = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
    GLDEBUG(glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, attachments));
#endif
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
    KRShader *postShader = m_pContext->getShaderManager()->getShader("PostShader", this, std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    KRVector3 rim_color;
    getContext().getShaderManager()->selectShader(*this, postShader, m_viewport, KRMat4(), std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, rim_color, 0.0f);
    
    m_pContext->getTextureManager()->selectTexture(0, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
    m_pContext->getTextureManager()->_setActiveTexture(0);
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
    m_pContext->getTextureManager()->_setActiveTexture(1);
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
    
    if(settings.volumetric_environment_enable) {
        m_pContext->getTextureManager()->selectTexture(2, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
        m_pContext->getTextureManager()->_setActiveTexture(2);
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, volumetricLightAccumulationTexture));
    }
	
	// Update attribute values.
    m_pContext->getModelManager()->bindVBO(getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_VERTICES, getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_INDEXES, getContext().getModelManager()->KRENGINE_VBO_2D_SQUARE_ATTRIBS, true);
	
    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    
    m_pContext->getTextureManager()->selectTexture(0, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
    m_pContext->getTextureManager()->_setActiveTexture(0);
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL, 0.0f, KRTexture::TEXTURE_USAGE_NONE);
    m_pContext->getTextureManager()->_setActiveTexture(1);
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
//            m_pContext->getModelManager()->bindVBO(KRENGINE_VBO_2D_SQUARE_INDICES, KRENGINE_VBO_2D_SQUARE_VERTEXES, KRENGINE_VBO_2D_SQUARE_ATTRIBS, true);
//            m_pContext->getTextureManager()->_setActiveTexture(0);
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
//        m_pContext->getTextureManager()->_setActiveTexture(0);
//        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
//    }
    
    
    
    if(m_debug_text_vertices.getSize()) {
        m_pContext->getModelManager()->releaseVBO(m_debug_text_vertices);
    }
    
    const char *szText = settings.m_debug_text.c_str();
    
    std::string debug_text;
    if(settings.debug_display != KRRenderSettings::KRENGINE_DEBUG_DISPLAY_NONE) {
        debug_text = getDebugText();;
        if(debug_text.length() > 0) {
            szText = debug_text.c_str();
        }
    }
    
    if(*szText) {
        int row_count = 1;
        const int MAX_TABS = 5;
        const int TAB_EXTRA = 2;
        int tab_cols[MAX_TABS] = {0, 0, 0, 0, 0};
        int iCol = 0;
        int iTab = 0;
        const char *pChar = szText;
        while(*pChar) {
            char c = *pChar++;
            if(c == '\n') {
                row_count++;
                iCol = 0;
                iTab = 0;
            } else if(c == '\t') {
                iCol = 0;
                iTab++;
            } else {
                iCol++;
                if(iCol > tab_cols[iTab]) tab_cols[iTab] = iCol;
            }
        }
        
        iCol = 0;
        for(iTab=0; iTab < MAX_TABS; iTab++) {
            iCol += tab_cols[iTab] + TAB_EXTRA;
            tab_cols[iTab] = iCol;
        }
        
        const int DEBUG_TEXT_COLUMNS = 256;
        const int DEBUG_TEXT_ROWS = 128;
        
        if(m_debug_text_vertices.getSize() == 0) {
            m_debug_text_vertices.expand(sizeof(DebugTextVertexData) * DEBUG_TEXT_COLUMNS * DEBUG_TEXT_ROWS * 6);
        }
        int vertex_count = 0;
        
        m_debug_text_vertices.lock();
        DebugTextVertexData *vertex_data = (DebugTextVertexData *)m_debug_text_vertices.getStart();

        pChar = szText;
        float dScaleX = 2.0 / (1024 / 16);
        float dScaleY = 2.0 / (768 / 16);
        float dTexScale = 1.0 / 16.0;
        int iRow = row_count - 1; iCol = 0, iTab = 0;
        while(*pChar) {
            char c = *pChar++;
            if(c == '\n') {
                iCol = 0;
                iTab = 0;
                iRow--;
            } else if(c == '\t') {
                iCol = tab_cols[iTab++];
            } else {
                if(iCol < DEBUG_TEXT_COLUMNS && iRow < DEBUG_TEXT_ROWS) {
                    int iChar = c - '\0';
                    int iTexCol = iChar % 16;
                    int iTexRow = 15 - (iChar - iTexCol) / 16;
                    
                    KRVector2 top_left_pos = KRVector2(-1.0f + dScaleX * iCol, dScaleY * iRow - 1.0);
                    KRVector2 bottom_right_pos = KRVector2(-1.0 + dScaleX * (iCol + 1), dScaleY * iRow + dScaleY - 1.0);
                    top_left_pos += KRVector2(1.0f / 2048.0f * 0.5f, 1.0f / 1536.0f * 0.5f);
                    bottom_right_pos += KRVector2(1.0f / 2048.0f * 0.5f, 1.0f / 1536.0f * 0.5f);
                    KRVector2 top_left_uv = KRVector2(dTexScale * iTexCol, dTexScale * iTexRow);
                    KRVector2 bottom_right_uv = KRVector2(dTexScale * iTexCol + dTexScale, dTexScale * iTexRow + dTexScale);
                    
                    vertex_data[vertex_count].x = top_left_pos.x;
                    vertex_data[vertex_count].y = top_left_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = top_left_uv.x;
                    vertex_data[vertex_count].v = top_left_uv.y;
                    vertex_count++;
                    
                    vertex_data[vertex_count].x = bottom_right_pos.x;
                    vertex_data[vertex_count].y = bottom_right_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = bottom_right_uv.x;
                    vertex_data[vertex_count].v = bottom_right_uv.y;
                    vertex_count++;
                    
                    vertex_data[vertex_count].x = top_left_pos.x;
                    vertex_data[vertex_count].y = bottom_right_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = top_left_uv.x;
                    vertex_data[vertex_count].v = bottom_right_uv.y;
                    vertex_count++;
                    
                    
                    vertex_data[vertex_count].x = top_left_pos.x;
                    vertex_data[vertex_count].y = top_left_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = top_left_uv.x;
                    vertex_data[vertex_count].v = top_left_uv.y;
                    vertex_count++;
                    
                    vertex_data[vertex_count].x = bottom_right_pos.x;
                    vertex_data[vertex_count].y = top_left_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = bottom_right_uv.x;
                    vertex_data[vertex_count].v = top_left_uv.y;
                    vertex_count++;
                    
                    vertex_data[vertex_count].x = bottom_right_pos.x;
                    vertex_data[vertex_count].y = bottom_right_pos.y;
                    vertex_data[vertex_count].z = 0.0f;
                    vertex_data[vertex_count].u = bottom_right_uv.x;
                    vertex_data[vertex_count].v = bottom_right_uv.y;
                    vertex_count++;
                }
                
                iCol++;
            }
        }
        
        
        // Disable backface culling
        GLDEBUG(glDisable(GL_CULL_FACE));
        
        // Disable z-buffer write
        GLDEBUG(glDepthMask(GL_FALSE));
        
        // Disable z-buffer test
        GLDEBUG(glDisable(GL_DEPTH_TEST));
//        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        // Enable alpha blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        
        KRShader *fontShader = m_pContext->getShaderManager()->getShader("debug_font", this, std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
        KRVector3 rim_color;
        getContext().getShaderManager()->selectShader(*this, fontShader, m_viewport, KRMat4(), std::vector<KRPointLight *>(), std::vector<KRDirectionalLight *>(), std::vector<KRSpotLight *>(), 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT, rim_color, 0.0f);
        
        m_pContext->getTextureManager()->selectTexture(0, m_pContext->getTextureManager()->getTexture("font"), 0.0f, KRTexture::TEXTURE_USAGE_UI);
        
        KRDataBlock index_data;
        //m_pContext->getModelManager()->bindVBO((void *)m_debug_text_vertices, vertex_count * sizeof(DebugTextVertexData), NULL, 0, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA), true);
        m_pContext->getModelManager()->bindVBO(m_debug_text_vertices, index_data, (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA), true);
        
        GLDEBUG(glDrawArrays(GL_TRIANGLES, 0, vertex_count));
        
        // Re-enable z-buffer write
        GLDEBUG(glDepthMask(GL_TRUE));
        
        m_debug_text_vertices.unlock();

    } else {
        if(m_debug_text_vertices.getSize() > 0) {
            m_debug_text_vertices = KRDataBlock();
        }
    }
}


std::string KRCamera::getDebugText()
{
    std::stringstream stream;
    stream.precision(std::numeric_limits<long double>::digits10);
    
    
    
    uint64_t fps = 0;
    if(m_frame_times_filled == KRAKEN_FPS_AVERAGE_FRAME_COUNT) {
        for(int i=0; i < KRAKEN_FPS_AVERAGE_FRAME_COUNT; i++) {
            fps += m_frame_times[i];
        }
    }
    
    fps = 1000000 / (fps / KRAKEN_FPS_AVERAGE_FRAME_COUNT); // Order of division chosen to prevent overflow
    
    switch(settings.debug_display) {
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_NONE: // ----====---- No debug display ----====----
        break;
            
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_TIME: // ----====---- Time / FPS ----====----
        {
            if(fps > 0) {
                stream << "FPS\t" << fps;
            }
        }
        break;
            
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_MEMORY: // ----====---- Memory Utilization ----=====----
        {
            
            // ---- CPU Memory ----
            
            struct task_basic_info info;
            mach_msg_type_number_t size = sizeof(info);
            kern_return_t kerr = task_info(mach_task_self(),
                                           TASK_BASIC_INFO,
                                           (task_info_t)&info,
                                           &size);
            if( kerr == KERN_SUCCESS ) {
                stream << "\tResident\tVirtual\tTotal";
                stream << "\nCPU\t" << (info.resident_size / 1024 / 1024) << " MB\t" << (info.virtual_size / 1024 / 1024) << " MB\t" << ((info.resident_size + info.virtual_size) / 1024 / 1024) << " MB";
            } else {
                stream << "\nERROR: Could not get CPU memory utilization.";
            }
            
            
            mach_port_t host_port = mach_host_self();
            mach_msg_type_number_t host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
            vm_size_t pagesize = 0;
            vm_statistics_data_t vm_stat;
            if(host_page_size(host_port, &pagesize) != KERN_SUCCESS) {
                stream << "\n\nERROR: Could not get VM page size.";
            } else if(host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) {
                stream << "\n\nERROR: Could not get VM stats.";
            } else {
                stream << "\n\n\n\tWired\tActive\tInactive\tFree\tTotal";
                stream << "\nVM\t";
                stream << (vm_stat.wire_count * pagesize / 1024 / 1024) << " MB\t";
                stream << (vm_stat.active_count * pagesize / 1024 / 1024) << " MB\t";
                stream << (vm_stat.inactive_count * pagesize / 1024 / 1024) << " MB\t";
                stream << (vm_stat.free_count * pagesize / 1024 / 1024) << " MB\t";
                stream << ((vm_stat.wire_count + vm_stat.active_count + vm_stat.inactive_count) * pagesize / 1024 / 1024) << " MB";
            }
                
            // ---- GPU Memory ----
            int texture_count_active = m_pContext->getTextureManager()->getActiveTextures().size();
            int texture_count = texture_count_active;
            long texture_mem_active = m_pContext->getTextureManager()->getMemActive();
            long texture_mem_used = m_pContext->getTextureManager()->getMemUsed();
            long texture_mem_throughput = m_pContext->getTextureManager()->getMemoryTransferedThisFrame();
            
            int vbo_count_active = m_pContext->getModelManager()->getActiveVBOCount();
            int vbo_count_pooled = m_pContext->getModelManager()->getPoolVBOCount();
            long vbo_mem_active = m_pContext->getModelManager()->getMemActive();
            long vbo_mem_used = m_pContext->getModelManager()->getMemUsed();
            long vbo_mem_throughput = m_pContext->getModelManager()->getMemoryTransferedThisFrame();
            
            long total_mem_active = texture_mem_active + vbo_mem_active;
            long total_mem_used = texture_mem_used + vbo_mem_used;
            long total_mem_throughput = texture_mem_throughput + vbo_mem_throughput;
            
            stream << "\n\n\n\t# Active\t# Used\tActive\tUsed\tThroughput\n";
            
            stream << "Textures\t" << texture_count_active << "\t" << texture_count << "\t" << (texture_mem_active / 1024) << " KB\t" << (texture_mem_used / 1024) << " KB\t" << (texture_mem_throughput / 1024) << " KB / frame\n";
            stream << "VBO's\t" << vbo_count_active << "\t" << vbo_count_active + vbo_count_pooled << "\t" << (vbo_mem_active / 1024) <<" KB\t" << (vbo_mem_used / 1024) << " KB\t" << (vbo_mem_throughput / 1024) << " KB / frame\n";
            stream << "\nGPU Total\t\t\t" << (total_mem_active / 1024) << " KB\t"  << (total_mem_used / 1024) << " KB\t" << (total_mem_throughput / 1024) << " KB / frame";
        }
        break;
            
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_TEXTURES: // ----====---- List Active Textures ----====----
        {
            bool first = true;
            int texture_count = 0;
            std::set<KRTexture *> active_textures = m_pContext->getTextureManager()->getActiveTextures();
            for(std::set<KRTexture *>::iterator itr=active_textures.begin(); itr != active_textures.end(); itr++) {
                KRTexture *texture = *itr;
                if(first) {
                    first = false;
                } else {
                    stream << "\n";
                }
                stream << texture->getName();
                stream << "\t";
                stream << texture->getMemSize() / 1024;
                stream << " KB";
                stream << "\t";
                stream << texture->getMaxMipMap();
                if(texture->hasMipmaps() && texture->getCurrentLodMaxDim() != texture->getMaxMipMap()) {
                    stream << " px => ";
                    stream << texture->getCurrentLodMaxDim();
                }
                stream << " px";
                texture_count++;
            }
            
            stream << "\n\nTOTAL: ";
            stream << texture_count;
            stream << " textures\t";
            stream << (m_pContext->getTextureManager()->getMemActive() / 1024) << " KB";
        }
        break;
    
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_DRAW_CALLS: // ----====---- List Draw Calls ----====----
        {
            std::vector<KRMeshManager::draw_call_info> draw_calls = m_pContext->getModelManager()->getDrawCalls();
            
            long draw_call_count = 0;
            long vertex_count = 0;
            stream << "\tVerts\tPass\tObject\tMaterial";
            for(std::vector<KRMeshManager::draw_call_info>::iterator itr = draw_calls.begin(); itr != draw_calls.end(); itr++) {
                draw_call_count++;
                stream << "\n" << draw_call_count << "\t" << (*itr).vertex_count << "\t";
                switch((*itr).pass) {
                    case KRNode::RENDER_PASS_FORWARD_OPAQUE:
                        stream << "opaq";
                        break;
                    case KRNode::RENDER_PASS_DEFERRED_GBUFFER:
                        stream << "d gb";
                        break;
                    case KRNode::RENDER_PASS_DEFERRED_LIGHTS:
                        stream << "d light";
                        break;
                    case KRNode::RENDER_PASS_DEFERRED_OPAQUE:
                        stream << "d opaq";
                        break;
                    case KRNode::RENDER_PASS_FORWARD_TRANSPARENT:
                        stream << "trans";
                        break;
                    case KRNode::RENDER_PASS_PARTICLE_OCCLUSION:
                        stream << "p occl";
                        break;
                    case KRNode::RENDER_PASS_ADDITIVE_PARTICLES:
                        stream << "a part";
                        break;
                    case KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE:
                        stream << "vol add";
                        break;
                    case KRNode::RENDER_PASS_GENERATE_SHADOWMAPS:
                        stream << "g shadow";
                        break;
                    case KRNode::RENDER_PASS_SHADOWMAP:
                        stream << "shadow";
                        break;
                }
                stream << "\t" << (*itr).object_name << "\t" << (*itr).material_name;
                vertex_count += (*itr).vertex_count;
            }
            stream << "\n\n\t\tTOTAL:\t" << draw_call_count << " draw calls\t" << vertex_count << " vertices";
        }
        break;
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_OCTREE:
        stream << "Octree Visualization";
        break;
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_COLLIDERS:
        stream << "Collider Visualization";
        break;
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_BONES:
        stream << "Bone Visualization";
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_SIREN_REVERB_ZONES:
        stream << "Siren - Reverb Zones";
        break;
    case KRRenderSettings::KRENGINE_DEBUG_DISPLAY_SIREN_AMBIENT_ZONES:
        stream << "Siren - Ambient Zones";
        break;
    }
    return stream.str();
}


const KRViewport &KRCamera::getViewport() const
{
    return m_viewport;
}
