//
//  KRSkyBox.cpp
//  KREngine
//
//  Created by Michael Ilich on 2012-08-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRSkyBox.h"
#import "KRShader.h"
#import "KRContext.h"
#import "KRMat4.h"
#import "KRResource.h"

KRSkyBox::KRSkyBox(KRContext &context, std::string name) : KRNode(context, name)
{
    m_frontTexture = "";
    m_backTexture = "";
    m_topTexture = "";
    m_bottomTexture = "";
    m_leftTexture = "";
    m_rightTexture = "";
    
    m_pFrontTexture = NULL;
    m_pBackTexture = NULL;
    m_pTopTexture = NULL;
    m_pBottomTexture = NULL;
    m_pLeftTexture = NULL;
    m_pRightTexture = NULL;

}

KRSkyBox::~KRSkyBox()
{
    
}

std::string KRSkyBox::getElementName() {
    return "sky_box";
}

void KRSkyBox::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);
    
    const char *szFrontTexture = e->Attribute("front_texture");
    if(szFrontTexture) {
        m_frontTexture = szFrontTexture;
    } else {
        m_frontTexture = "";
    }
    m_pFrontTexture = NULL;
    
    const char *szBackTexture = e->Attribute("back_texture");
    if(szBackTexture) {
        m_backTexture = szBackTexture;
    } else {
        m_backTexture = "";
    }
    m_pBackTexture = NULL;
    
    const char *szTopTexture = e->Attribute("top_texture");
    if(szTopTexture) {
        m_topTexture = szTopTexture;
    } else {
        m_topTexture = "";
    }
    m_pTopTexture = NULL;
    
    const char *szBottomTexture = e->Attribute("bottom_texture");
    if(szBottomTexture) {
        m_bottomTexture = szBottomTexture;
    } else {
        m_bottomTexture = "";
    }
    m_pBottomTexture = NULL;
    
    const char *szLeftTexture = e->Attribute("left_texture");
    if(szLeftTexture) {
        m_leftTexture = szLeftTexture;
    } else {
        m_leftTexture = "";
    }
    m_pLeftTexture = NULL;
    
    const char *szRightTexture = e->Attribute("right_texture");
    if(szRightTexture) {
        m_rightTexture = szRightTexture;
    } else {
        m_rightTexture = "";
    }
    m_pRightTexture = NULL;
    
}


#if TARGET_OS_IPHONE

void KRSkyBox::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRNode::RenderPass renderPass) {
    
    if(renderPass == KRNode::RENDER_PASS_SKYBOX) {
        // Skybox is rendered on the final pass of the deferred renderer
        
        KRMat4 projectionMatrix = pCamera->getProjectionMatrix();
        
        KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
        KRMat4 matModelToView = viewMatrix * m_modelMatrix;
        matModelToView.transpose();
        matModelToView.invert();
                
//        KRShader *pShader = pContext->getShaderManager()->getShader("sky_box", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, renderPass);
        
//        pShader->bind(pCamera, matModelToView, mvpmatrix, cameraPosition, NULL, NULL, NULL, 0, renderPass);
        
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
}

#endif

