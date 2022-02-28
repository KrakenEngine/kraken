//
//  KRDirectionalLight.cpp
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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

#include "KRDirectionalLight.h"
#include "KRPipeline.h"
#include "KRContext.h"
#include "assert.h"
#include "KRStockGeometry.h"

/* static */
void KRDirectionalLight::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRLight::InitNodeInfo(nodeInfo);
  // No additional members
}

KRDirectionalLight::KRDirectionalLight(KRScene &scene, std::string name) : KRLight(scene, name)
{

}

KRDirectionalLight::~KRDirectionalLight()
{
    
}

std::string KRDirectionalLight::getElementName() {
    return "directional_light";
}

Vector3 KRDirectionalLight::getWorldLightDirection() {
    return Matrix4::Dot(getWorldRotation().rotationMatrix(), getLocalLightDirection());
}

Vector3 KRDirectionalLight::getLocalLightDirection() {
    return Vector3::Up();           //&KRF HACK changed from Vector3::Forward(); - to compensate for the way Maya handles post rotation.
}


int KRDirectionalLight::configureShadowBufferViewports(const KRViewport &viewport) {
    
    const float KRENGINE_SHADOW_BOUNDS_EXTRA_SCALE = 1.25f; // Scale to apply to view frustrum bounds so that we don't need to refresh shadows on every frame
    int cShadows = 1;
    for(int iShadow=0; iShadow < cShadows; iShadow++) {
        /*
           TODO - Determine if we still need this...
         
        GLfloat shadowMinDepths[3][3] = {{0.0f, 0.0f, 0.0f},{0.0f, 0.0f, 0.0f},{0.0f, 0.05f, 0.3f}};
        GLfloat shadowMaxDepths[3][3] = {{0.0f, 0.0f, 1.0f},{0.1f, 0.0f, 0.0f},{0.1f, 0.3f, 1.0f}};
        
        float min_depth = 0.0f;
        float max_depth = 1.0f;
        */
        
        AABB worldSpacefrustrumSliceBounds = AABB::Create(Vector3::Create(-1.0f, -1.0f, -1.0f), Vector3::Create(1.0f, 1.0f, 1.0f), Matrix4::Invert(viewport.getViewProjectionMatrix()));
        worldSpacefrustrumSliceBounds.scale(KRENGINE_SHADOW_BOUNDS_EXTRA_SCALE);
        
        Vector3 shadowLook = -Vector3::Normalize(getWorldLightDirection());
        
        Vector3 shadowUp = Vector3::Create(0.0, 1.0, 0.0);
        if(Vector3::Dot(shadowUp, shadowLook) > 0.99f) shadowUp = Vector3::Create(0.0, 0.0, 1.0); // Ensure shadow look direction is not parallel with the shadowUp direction
        
//        Matrix4 matShadowView = Matrix4::LookAt(viewport.getCameraPosition() - shadowLook, viewport.getCameraPosition(), shadowUp);
//        Matrix4 matShadowProjection = Matrix4();
//        matShadowProjection.scale(0.001, 0.001, 0.001);
        
        Matrix4 matShadowView = Matrix4::LookAt(worldSpacefrustrumSliceBounds.center() - shadowLook, worldSpacefrustrumSliceBounds.center(), shadowUp);
        Matrix4 matShadowProjection = Matrix4();
        AABB shadowSpaceFrustrumSliceBounds = AABB::Create(worldSpacefrustrumSliceBounds.min, worldSpacefrustrumSliceBounds.max, Matrix4::Invert(matShadowProjection));
        AABB shadowSpaceSceneBounds = AABB::Create(getScene().getRootOctreeBounds().min, getScene().getRootOctreeBounds().max, Matrix4::Invert(matShadowProjection));
        if(shadowSpaceSceneBounds.min.z < shadowSpaceFrustrumSliceBounds.min.z) shadowSpaceFrustrumSliceBounds.min.z = shadowSpaceSceneBounds.min.z; // Include any potential shadow casters that are outside the view frustrum
        matShadowProjection.scale(1.0f / shadowSpaceFrustrumSliceBounds.size().x, 1.0f / shadowSpaceFrustrumSliceBounds.size().y, 1.0f / shadowSpaceFrustrumSliceBounds.size().z);
        
        Matrix4 matBias;
        matBias.bias();
        matShadowProjection *= matBias;
        
        KRViewport newShadowViewport = KRViewport(Vector2::Create(KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT), matShadowView, matShadowProjection);
        AABB prevShadowBounds = AABB::Create(-Vector3::One(), Vector3::One(), Matrix4::Invert(m_shadowViewports[iShadow].getViewProjectionMatrix()));
        AABB minimumShadowBounds = AABB::Create(-Vector3::One(), Vector3::One(), Matrix4::Invert(newShadowViewport.getViewProjectionMatrix()));
        minimumShadowBounds.scale(1.0f / KRENGINE_SHADOW_BOUNDS_EXTRA_SCALE);
        if(!prevShadowBounds.contains(minimumShadowBounds) || !shadowValid[iShadow] || true) { // FINDME, HACK - Re-generating the shadow map every frame.  This should only be needed if the shadow contains non-static geometry
            m_shadowViewports[iShadow] = newShadowViewport;
            shadowValid[iShadow] = false;
            fprintf(stderr, "Kraken - Generate shadow maps...\n");
        }
    }

    return 1;
}

void KRDirectionalLight::render(VkCommandBuffer& commandBuffer, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRLight::render(commandBuffer, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);

    if(renderPass == KRNode::RENDER_PASS_DEFERRED_LIGHTS) {
        // Lights are rendered on the second pass of the deferred renderer
        
        std::vector<KRDirectionalLight *> this_light;
        this_light.push_back(this);

        Matrix4 matModelViewInverseTranspose = viewport.getViewMatrix() * getModelMatrix();
        matModelViewInverseTranspose.transpose();
        matModelViewInverseTranspose.invert();
        
        Vector3 light_direction_view_space = getWorldLightDirection();
        light_direction_view_space = Matrix4::Dot(matModelViewInverseTranspose, light_direction_view_space);
        light_direction_view_space.normalize();
        
        KRPipeline *pShader = getContext().getPipelineManager()->getPipeline("light_directional", pCamera, std::vector<KRPointLight *>(), this_light, std::vector<KRSpotLight *>(), 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
        if(getContext().getPipelineManager()->selectPipeline(*pCamera, pShader, viewport, getModelMatrix(), std::vector<KRPointLight *>(), this_light, std::vector<KRSpotLight *>(), 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
            
            pShader->setUniform(KRPipeline::KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE, light_direction_view_space);
            pShader->setUniform(KRPipeline::KRENGINE_UNIFORM_LIGHT_COLOR, m_color);
            pShader->setUniform(KRPipeline::KRENGINE_UNIFORM_LIGHT_INTENSITY, m_intensity * 0.01f);
            
            // Disable z-buffer write
            GLDEBUG(glDepthMask(GL_FALSE));
            
            // Disable z-buffer test
            GLDEBUG(glDisable(GL_DEPTH_TEST));
            
            // Render a full screen quad
            m_pContext->getMeshManager()->bindVBO(&getContext().getMeshManager()->KRENGINE_VBO_DATA_2D_SQUARE_VERTICES, 1.0f);
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        }
    }
}

AABB KRDirectionalLight::getBounds()
{
    return AABB::Infinite();
}
