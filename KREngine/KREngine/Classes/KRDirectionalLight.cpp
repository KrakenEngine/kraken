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

KRDirectionalLight::KRDirectionalLight(std::string name) : KRLight(name)
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

void KRDirectionalLight::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
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
        
        KRShader *pShader = pContext->getShaderManager()->getShader("light_directional", pCamera, false, false, false, 0, false, false, false, false, false, false, false, renderPass);
        pShader->bind(pCamera, matModelToView, mvpmatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, 0, renderPass);
        
        
        glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE],
                    light_direction_view_space.x,
                    light_direction_view_space.y,
                    light_direction_view_space.z
        );
        
        glUniform3f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR],
                    m_color.x,
                    m_color.y,
                    m_color.z
                    );
        
        glUniform1f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_INTENSITY],
                    m_intensity / 100.0f
                    );
        
        // Disable z-buffer write
        glDepthMask(GL_FALSE);
        
        // Disable z-buffer test
        glDisable(GL_DEPTH_TEST);
        
        // Render a full screen quad
        static const GLfloat squareVertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f,  1.0f,
            1.0f,  1.0f,
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    KRNode::render(pCamera, pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);
}

#endif