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

KRVector3 KRDirectionalLight::getLightDirection() {
    KRVector3 world_rotation = getWorldRotation();
    KRVector3 light_rotation = KRVector3(0.0, -1.0, 0.0);
    KRMat4 m;
    m.rotate(world_rotation.x, X_AXIS);
    m.rotate(world_rotation.y, Y_AXIS);
    m.rotate(world_rotation.z, Z_AXIS);
    KRVector3 light_direction = m.dot(light_rotation);
    return light_direction;
}

#if TARGET_OS_IPHONE

void KRDirectionalLight::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, bool bRenderShadowMap, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, int gBufferPass) {
    
    if(gBufferPass == 2) {
        // Lights are rendered on the second pass of the deferred renderer
        
        KRMat4 projectionMatrix = pCamera->getProjectionMatrix();

        KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
        KRMat4 matModelToView = viewMatrix * m_modelMatrix;
        matModelToView.transpose();
        matModelToView.invert();
        
        KRVector3 light_direction = getLightDirection();
        light_direction = matModelToView.dot(light_direction);
        light_direction.normalize();
        
        KRShader *pShader = pContext->getShaderManager()->getShader("light_directional", pCamera, false, false, false, 0, false, false, false, false, false, false, false, gBufferPass);
        pShader->bind(pCamera, matModelToView, mvpmatrix, cameraPosition, light_direction, pShadowMatrices, shadowDepthTextures, 0, gBufferPass);
        
        
        // Render a full screen quad
        static const GLfloat squareVertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f,  1.0f,
            1.0f,  1.0f,
        };
        
        // Disable z-buffer test
        glDisable(GL_DEPTH_TEST);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    
    KRNode::render(pCamera, pContext, frustrumVolume, bRenderShadowMap, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, gBufferPass);
}

#endif