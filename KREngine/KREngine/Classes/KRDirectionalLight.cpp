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

KRDirectionalLight::KRDirectionalLight(std::string name) : KRLight(name)
{

}

KRDirectionalLight::~KRDirectionalLight()
{
    
}

std::string KRDirectionalLight::getElementName() {
    return "directional_light";
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
        
        
        KRShader *pShader = pContext->getShaderManager()->getShader("light_directional", pCamera, false, false, false, 0, false, false, false, false, false, false, false, gBufferPass);
        pShader->bind(pCamera, matModelToView, mvpmatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, 0, gBufferPass);
        
        
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