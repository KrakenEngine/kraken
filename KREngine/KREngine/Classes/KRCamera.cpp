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

KRCamera::KRCamera(KRContext &context, GLint width, GLint height) : KRNotified(context) {
    backingWidth = width;
    backingHeight = height;
    
    
    double const PI = 3.141592653589793f;
    double const D2R = PI * 2 / 360;
    
    bShowShadowBuffer = false;
    bShowOctree = false;
    bShowDeferred = false;
    bEnablePerPixel = true;
    bEnableDiffuseMap = true;
    bEnableNormalMap = true;
    bEnableSpecMap = true;
    bEnableReflectionMap = true;
    bDebugPSSM = false;
    bEnableAmbient = true;
    bEnableDiffuse = true;
    bEnableSpecular = false; // FINDME - Should be "true"
    bEnableLightMap = true;
    bDebugSuperShiny = false;
    bEnableDeferredLighting = false; // FINDME - should be "true"
    
    
    dAmbientR = 0.25f; // FINDME - should be "0.0f"
    dAmbientG = 0.25f; // FINDME - should be "0.0f"
    dAmbientB = 0.25f; // FINDME - should be "0.0f"
    
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
    
    memset(shadowFramebuffer, sizeof(GLuint) * 3, 0);
    memset(shadowDepthTexture, sizeof(GLuint) * 3, 0);
    
    m_iFrame = 0;
    
    createBuffers();
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

void KRCamera::renderFrame(KRScene &scene, KRMat4 &viewMatrix)
{
    m_pContext->rotateBuffers();
    KRMat4 invViewMatrix = viewMatrix;
    invViewMatrix.invert();
    
    KRVector3 cameraPosition = KRMat4::Dot(invViewMatrix, KRVector3(0.0,0.0,0.0));
    
    KRVector3 lightDirection(0.0, 0.0, 1.0);
    
    // ----- Render Model -----
    KRMat4 shadowvp;
    shadowvp.rotate(scene.sun_pitch, X_AXIS);
    shadowvp.rotate(scene.sun_yaw, Y_AXIS);
    lightDirection = KRMat4::Dot(shadowvp, lightDirection);
    shadowvp.invert();
    
    
    lightDirection.normalize();
    
    allocateShadowBuffers();
    int iOffset=m_iFrame % m_cShadowBuffers;
    for(int iShadow2=iOffset; iShadow2 < m_cShadowBuffers + iOffset; iShadow2++) {
        int iShadow = iShadow2 % m_cShadowBuffers;
        
        
        GLfloat shadowMinDepths[3][3] = {{0.0, 0.0, 0.0},{0.0, 0.0, 0.0},{0.0, 0.05, 0.3}};
        GLfloat shadowMaxDepths[3][3] = {{0.0, 0.0, 1.0},{0.1, 0.0, 0.0},{0.1, 0.3, 1.0}};
        
        
        KRMat4 newShadowMVP;
        if(shadowMaxDepths[m_cShadowBuffers - 1][iShadow] == 0.0) {
            KRBoundingVolume ext = KRBoundingVolume(scene.getExtents(m_pContext));
            
            newShadowMVP = ext.calcShadowProj(&scene, m_pContext, scene.sun_yaw, scene.sun_pitch);
        } else {
            KRBoundingVolume frustrumSliceVolume = KRBoundingVolume(viewMatrix, perspective_fov, getViewportSize().x / getViewportSize().y, perspective_nearz + (perspective_farz - perspective_nearz) * shadowMinDepths[m_cShadowBuffers - 1][iShadow], perspective_nearz + (perspective_farz - perspective_nearz) * shadowMaxDepths[m_cShadowBuffers - 1][iShadow]);
            newShadowMVP = frustrumSliceVolume.calcShadowProj(&scene, m_pContext, scene.sun_yaw, scene.sun_pitch);
        }
        
        if(!(shadowmvpmatrix[iShadow] == newShadowMVP)) {
            shadowValid[iShadow] = false;
        }
        
        if(!shadowValid[iShadow]) {
            shadowValid[iShadow] = true;
            
            shadowmvpmatrix[iShadow] = newShadowMVP;
            renderShadowBuffer(scene, iShadow);
            
            break;
        }
    }
    
    renderFrame(scene, viewMatrix, lightDirection, cameraPosition);
    renderPost();
    
    m_iFrame++;
}

void KRCamera::renderFrame(KRScene &scene, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRVector3 &cameraPosition) {
    setViewportSize(KRVector2(backingWidth, backingHeight));
    
    KRBoundingVolume frustrumVolume = KRBoundingVolume(viewMatrix, perspective_fov, getViewportSize().x / getViewportSize().y, perspective_nearz, perspective_farz);
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
        scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_DEFERRED_GBUFFER);
        
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
        scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, 0, KRNode::RENDER_PASS_DEFERRED_LIGHTS);
        
        scene.getOcclusionQueryResults(m_visibleBounds);
        
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
        scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_DEFERRED_OPAQUE);
        
        // Deactivate source buffer texture units
        m_pContext->getTextureManager()->selectTexture(6, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE6));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        m_pContext->getTextureManager()->selectTexture(7, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE7));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    } else {
        // ----====---- Opaque Geometry, Forward Rendering ----====----
        
        // Set render target
        GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, compositeFramebuffer));
        GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, compositeDepthTexture, 0));
        
        // Disable alpha blending
        GLDEBUG(glDisable(GL_BLEND));
        
        GLDEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GLDEBUG(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
        
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
        scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_FORWARD_OPAQUE);
        
        scene.getOcclusionQueryResults(m_visibleBounds);
    }
    
    // ----====---- Transparent Geometry, Forward Rendering ----====----
    
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
    
    // Enable alpha blending
    GLDEBUG(glEnable(GL_BLEND));
    GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Render all transparent geometry
    scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    
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
    scene.render(this, m_visibleBounds, m_pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, m_cShadowBuffers, KRNode::RENDER_PASS_FLARES);
    
    // ----====---- Debug Overlay ----====----
    
    if(bShowOctree) {
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        
        
        // Enable backface culling
        GLDEBUG(glCullFace(GL_BACK));
        GLDEBUG(glEnable(GL_CULL_FACE));
        
        
        KRShader *pVisShader = m_pContext->getShaderManager()->getShader("visualize_overlay", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
        KRMat4 projectionMatrix = getProjectionMatrix();
        
        m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_3D_CUBE, KRENGINE_VBO_3D_CUBE_SIZE, true, false, false, false, false);
        for(std::set<KRAABB>::iterator itr=m_visibleBounds.begin(); itr != m_visibleBounds.end(); itr++) {
            KRMat4 matModel = KRMat4();
            matModel.scale((*itr).size() / 2.0f);
            matModel.translate((*itr).center());
            KRMat4 mvpmatrix = matModel * viewMatrix * projectionMatrix;
            
            if(pVisShader->bind(this, viewMatrix, mvpmatrix, cameraPosition, lightDirection, shadowmvpmatrix, shadowDepthTexture, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 14));
            }
        }
    }
    
//    scene.getOcclusionQueryResults(m_visibleBounds);
    
    // fprintf(stderr, "visible bounds: %i\n", (int)m_visibleBounds.size());
    
    // Re-enable z-buffer write
    GLDEBUG(glDepthMask(GL_TRUE));
    

    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i Kbyte    Shader Handles: %i\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024, (int)m_pContext->getShaderManager()->getShaderHandlesUsed());
}


void KRCamera::createBuffers() {
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
    
    allocateShadowBuffers();
}

void KRCamera::allocateShadowBuffers() {
    // First deallocate buffers no longer needed
    for(int iShadow = m_cShadowBuffers; iShadow < KRENGINE_MAX_SHADOW_BUFFERS; iShadow++) {
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
    for(int iShadow = 0; iShadow < m_cShadowBuffers; iShadow++) {
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
            GLDEBUG(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL));
            
            GLDEBUG(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture[iShadow], 0));
        }
    }
}

void KRCamera::destroyBuffers()
{
    m_cShadowBuffers = 0;
    allocateShadowBuffers();
    
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
}


void KRCamera::renderShadowBuffer(KRScene &scene, int iShadow)
{
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer[iShadow]));
    GLDEBUG(glClearDepthf(1.0f));
    GLDEBUG(glClear(GL_DEPTH_BUFFER_BIT));
    
    //glViewport(1, 1, 2046, 2046);
    
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
    KRShader *shadowShader = m_pContext->getShaderManager()->getShader("ShadowShader", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    KRMat4 matIdentity; // Value not used by postshader
    KRVector3 vec4Temp; // Value not used by postshader
    shadowShader->bind(this, matIdentity, matIdentity, m_position, vec4Temp, NULL, NULL, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);    
    
    // Bind our modelmatrix variable to be a uniform called mvpmatrix in our shaderprogram
    GLDEBUG(glUniformMatrix4fv(shadowShader->m_uniforms[KRShader::KRENGINE_UNIFORM_SHADOWMVP1], 1, GL_FALSE, shadowmvpmatrix[iShadow].getPointer()));
    
    
    // Calculate the bounding volume of the light map
    KRMat4 matInvShadow = shadowmvpmatrix[iShadow];
    matInvShadow.invert();
    
    KRVector3 vertices[8];
    vertices[0] = KRVector3(-1.0, -1.0, 0.0);
    vertices[1] = KRVector3(1.0,  -1.0, 0.0);
    vertices[2] = KRVector3(1.0,   1.0, 0.0);
    vertices[3] = KRVector3(-1.0,  1.0, 0.0);
    vertices[4] = KRVector3(-1.0, -1.0, 1.0);
    vertices[5] = KRVector3(1.0,  -1.0, 1.0);
    vertices[6] = KRVector3(1.0,   1.0, 1.0);
    vertices[7] = KRVector3(-1.0,  1.0, 1.0);
    
    for(int iVertex=0; iVertex < 8; iVertex++) {
        vertices[iVertex] = KRMat4::Dot(matInvShadow, vertices[iVertex]);
    }
    
    KRVector3 cameraPosition;
    KRVector3 lightDirection;
    KRBoundingVolume shadowVolume = KRBoundingVolume(vertices);
    scene.render(this, m_shadowVisibleBounds[iShadow], m_pContext, shadowVolume, shadowmvpmatrix[iShadow], cameraPosition, lightDirection, shadowmvpmatrix, NULL, m_cShadowBuffers, KRNode::RENDER_PASS_SHADOWMAP);
    scene.getOcclusionQueryResults(m_shadowVisibleBounds[iShadow]);
    GLDEBUG(glViewport(0, 0, backingWidth, backingHeight));
}

void KRCamera::renderPost()
{
    
    GLDEBUG(glBindFramebuffer(GL_FRAMEBUFFER, 1)); // renderFramebuffer
    
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
    KRShader *postShader = m_pContext->getShaderManager()->getShader("PostShader", this, false, false, false, 0, false, false, false, false, false, false, false, false, false, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    KRMat4 matIdentity; // Value not used by postshader
    KRVector3 vec4Temp; // Value not used by postshader
    postShader->bind(this, matIdentity, matIdentity, m_position, vec4Temp, NULL, NULL, 0, KRNode::RENDER_PASS_FORWARD_TRANSPARENT);
    
    m_pContext->getTextureManager()->selectTexture(0, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeColorTexture));
	
	// Update attribute values.
    m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
	
    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    
    m_pContext->getTextureManager()->selectTexture(0, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE0));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    m_pContext->getTextureManager()->selectTexture(1, NULL);
    GLDEBUG(glActiveTexture(GL_TEXTURE1));
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    
    
    if(bShowShadowBuffer) {
        GLDEBUG(glDisable(GL_DEPTH_TEST));
        
        m_pContext->getTextureManager()->selectTexture(0, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        
        // Update attribute values.
        
        m_pContext->getModelManager()->configureAttribs(true, false, false, true, false);
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, KRENGINE_VERTICES_2D_SQUARE_UV));
        
        for(int iShadow=0; iShadow < m_cShadowBuffers; iShadow++) {
            m_pContext->getTextureManager()->selectTexture(1, NULL);
            GLDEBUG(glActiveTexture(GL_TEXTURE1));
            GLDEBUG(glBindTexture(GL_TEXTURE_2D, shadowDepthTexture[iShadow]));
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVerticesShadow[iShadow]));
            
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        }
        
        m_pContext->getTextureManager()->selectTexture(0, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        
        m_pContext->getTextureManager()->selectTexture(1, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE1));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    }
    
    
    
    const char *szText = m_debug_text.c_str();
    if(*szText) {
        KRTexture *pFontTexture = m_pContext->getTextureManager()->getTexture("font");
        
        GLDEBUG(glDisable(GL_DEPTH_TEST));
        
        m_pContext->getTextureManager()->selectTexture(0, NULL);
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, compositeDepthTexture));
        
        m_pContext->getTextureManager()->selectTexture(1, pFontTexture);
        
        const char *pChar = szText;
        int iPos=0;
        double dScale = 1.0 / 24.0;
        double dTexScale = 1.0 / 16.0;
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
            
            m_pContext->getModelManager()->configureAttribs(true, false, false, true, false);
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, 0, charTexCoords));
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, charVertices));
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
            
            iPos++;
        }
        
        GLDEBUG(glActiveTexture(GL_TEXTURE0));
        GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
        
        m_pContext->getTextureManager()->selectTexture(1, NULL);
    }
    
}

void KRCamera::invalidateShadowBuffers() {
    for(int i=0; i < m_cShadowBuffers; i++) {
        shadowValid[i] = false;
    }
}

void KRCamera::notify_sceneGraphCreate(KRNode *pNode)
{
    fprintf(stderr, "KRCamera - notify_sceneGraphCreate");
    
    KRInstance *pInstance = dynamic_cast<KRInstance *>(pNode);
    if(pInstance) {
        if(pInstance->hasTransparency()) {
            KRInstanceDistance transparentInstanceDistance = KRInstanceDistance(pInstance, 0.0f);
            m_transparentInstances.push_back(transparentInstanceDistance);
        }
    }
}

void KRCamera::notify_sceneGraphDelete(KRNode *pNode)
{
    fprintf(stderr, "KRCamera - notify_sceneGraphDelete");
    
    KRInstance *pInstance = dynamic_cast<KRInstance *>(pNode);
    if(pInstance) {
        m_transparentInstances.remove_if(KRInstanceDistance::InstanceEqualsPredicate(pInstance));
    }
}

void KRCamera::notify_sceneGraphModify(KRNode *pNode)
{
    fprintf(stderr, "KRCamera - notify_sceneGraphModify");
    
    KRInstance *pInstance = dynamic_cast<KRInstance *>(pNode);
    if(pInstance) {
        m_transparentInstances.remove_if(KRInstanceDistance::InstanceEqualsPredicate(pInstance));
        if(pInstance->hasTransparency()) {
            KRInstanceDistance transparentInstanceDistance = KRInstanceDistance(pInstance, 0.0f);
            m_transparentInstances.push_back(transparentInstanceDistance);
        }
    }
}