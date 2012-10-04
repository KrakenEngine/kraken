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

#if TARGET_OS_IPHONE

void KRDirectionalLight::render(KRCamera *pCamera, KRContext *pContext, KRMat4 &viewMatrix, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
    KRLight::render(pCamera, pContext, viewMatrix, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);

    if(renderPass == KRNode::RENDER_PASS_DEFERRED_LIGHTS) {
        // Lights are rendered on the second pass of the deferred renderer
        
        KRMat4 projectionMatrix = pCamera->getProjectionMatrix();

        KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
        KRMat4 matModelToView = viewMatrix * m_modelMatrix;
        matModelToView.transpose();
        matModelToView.invert();
        
        KRVector3 light_direction_view_space = getWorldLightDirection();
        light_direction_view_space = KRMat4::Dot(matModelToView, light_direction_view_space);
        light_direction_view_space.normalize();
        
        KRShader *pShader = pContext->getShaderManager()->getShader("light_directional", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, false, renderPass);
        if(pShader->bind(pCamera, matModelToView, mvpmatrix, lightDirection, pShadowMatrices, shadowDepthTextures, 0, renderPass)) {
            
            
            GLDEBUG(glUniform3f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE],
                        light_direction_view_space.x,
                        light_direction_view_space.y,
                        light_direction_view_space.z
            ));
            
            GLDEBUG(glUniform3f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR],
                        m_color.x,
                        m_color.y,
                        m_color.z
                        ));
            
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