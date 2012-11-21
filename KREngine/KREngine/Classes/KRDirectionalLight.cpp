//
//  KRDirectionalLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRDirectionalLight.h"
#import "KRShader.h"
#import "KRContext.h"
#import "KRMat4.h"
#import "assert.h"
#import "KRStockGeometry.h"

KRDirectionalLight::KRDirectionalLight(KRScene &scene, std::string name) : KRLight(scene, name)
{

}

KRDirectionalLight::~KRDirectionalLight()
{
    
}

std::string KRDirectionalLight::getElementName() {
    return "directional_light";
}

KRVector3 KRDirectionalLight::getWorldLightDirection() {
    const GLfloat PI = 3.14159265;
    const GLfloat d2r = PI * 2 / 360;
    
    KRVector3 world_rotation = getLocalRotation();
    KRVector3 light_rotation = KRVector3(0.0, 0.0, -1.0);
    KRMat4 m;
    m.rotate(world_rotation.x, X_AXIS);
    m.rotate(world_rotation.y, Y_AXIS);
    m.rotate(world_rotation.z, X_AXIS);
    m.rotate(-90.0 * d2r, Y_AXIS);
    KRVector3 light_direction = KRMat4::Dot(m, light_rotation);
    return light_direction;
}

KRVector3 KRDirectionalLight::getLocalLightDirection() {
   return KRVector3(0.0, 0.0, 1.0);
}


int KRDirectionalLight::configureShadowBufferViewports(const KRViewport &viewport) {
    int cShadows = 1;
    for(int iShadow=0; iShadow < cShadows; iShadow++) {
        GLfloat shadowMinDepths[3][3] = {{0.0, 0.0, 0.0},{0.0, 0.0, 0.0},{0.0, 0.05, 0.3}};
        GLfloat shadowMaxDepths[3][3] = {{0.0, 0.0, 1.0},{0.1, 0.0, 0.0},{0.1, 0.3, 1.0}};
        
        KRVector3 shadowLook = -KRVector3::Normalize(getWorldLightDirection());
        
        KRVector3 shadowUp(0.0, 1.0, 0.0);
        if(KRVector3::Dot(shadowUp, shadowLook) > 0.99f) shadowUp = KRVector3(0.0, 0.0, 1.0); // Ensure shadow look direction is not parallel with the shadowUp direction
        
        KRMat4 matShadowView = KRMat4::LookAt(viewport.getCameraPosition() - shadowLook, viewport.getCameraPosition(), shadowUp);
        
        
        KRMat4 matShadowProjection = KRMat4();
        matShadowProjection.scale(0.001, 0.001, 0.001);
        
        KRMat4 matBias;
        matBias.bias();
        matShadowProjection *= matBias;
        
        m_shadowViewports[iShadow] = KRViewport(KRVector2(KRENGINE_SHADOW_MAP_WIDTH, KRENGINE_SHADOW_MAP_HEIGHT), matShadowView, matShadowProjection);        
    }

    return 1;
}

#if TARGET_OS_IPHONE

void KRDirectionalLight::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass) {
    
    KRLight::render(pCamera, lights, viewport, renderPass);

    if(renderPass == KRNode::RENDER_PASS_DEFERRED_LIGHTS) {
        // Lights are rendered on the second pass of the deferred renderer
        
        std::vector<KRLight *> this_light;
        this_light.push_back(this);

        KRMat4 matModelViewInverseTranspose = viewport.getViewMatrix() * getModelMatrix();
        matModelViewInverseTranspose.transpose();
        matModelViewInverseTranspose.invert();
        
        KRVector3 light_direction_view_space = getWorldLightDirection();
        light_direction_view_space = KRMat4::Dot(matModelViewInverseTranspose, light_direction_view_space);
        light_direction_view_space.normalize();
        
        KRShader *pShader = getContext().getShaderManager()->getShader("light_directional", pCamera, this_light, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
        if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, getModelMatrix(), this_light, renderPass)) {
            
            light_direction_view_space.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE]);
            m_color.setUniform(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR]);
            
            GLDEBUG(glUniform1f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_INTENSITY],
                        m_intensity / 100.0f
                        ));
            
            // Disable z-buffer write
            GLDEBUG(glDepthMask(GL_FALSE));
            
            // Disable z-buffer test
            GLDEBUG(glDisable(GL_DEPTH_TEST));
            
            // Render a full screen quad
            m_pContext->getModelManager()->bindVBO((void *)KRENGINE_VBO_2D_SQUARE, KRENGINE_VBO_2D_SQUARE_SIZE, true, false, false, true, false);
            GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        }
    }
}

#endif