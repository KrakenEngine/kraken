//
//  KRScene.cpp
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

#include <iostream>

#import "KRVector3.h"
#import "KRMat4.h"
#import "tinyxml2.h"

#import "KRDirectionalLight.h"

#import "KRScene.h"

KRScene::KRScene(KRContext &context, std::string name) : KRResource(context, name) {
    m_pContext = &context;
    m_pFirstDirectionalLight = NULL;
    m_pRootNode = new KRNode(context, "scene_root");
    
    sun_yaw = 4.333; // TODO - Remove temporary testing code
    sun_pitch = 0.55;
}
KRScene::~KRScene() {
    delete m_pRootNode;
    m_pRootNode = NULL;
}

#if TARGET_OS_IPHONE

void KRScene::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
    if(renderPass != KRNode::RENDER_PASS_SHADOWMAP) {
    
        if(cShadowBuffers > 0) {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        
        if(cShadowBuffers > 1) {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[1]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        
        if(cShadowBuffers > 2) {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, shadowDepthTextures[2]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
        
    KRVector3 forward_render_light_direction = lightDirection;
    KRDirectionalLight *directional_light = getFirstDirectionalLight();
    if(directional_light) {
        forward_render_light_direction = directional_light->getWorldLightDirection();
        KRVector3 sun_color = directional_light->getColor() * (directional_light->getIntensity() / 100.0f);
        pCamera->dSunR = sun_color.x;
        pCamera->dSunG = sun_color.y;
        pCamera->dSunB = sun_color.z;
    }
    
    m_pRootNode->render(pCamera, pContext, frustrumVolume, viewMatrix, cameraPosition, forward_render_light_direction, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);
}

#endif

KRBoundingVolume KRScene::getExtents(KRContext *pContext) {
    return m_pRootNode->getExtents(pContext);
}


std::string KRScene::getExtension() {
    return "krscene";
}

KRNode *KRScene::getRootNode() {
    return m_pRootNode;
}

bool KRScene::save(const std::string& path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLNode *scene_node = doc.InsertEndChild( doc.NewElement( "scene" ));
    m_pRootNode->saveXML(scene_node);
    doc.SaveFile(path.c_str());
    return true;
}


KRDirectionalLight *KRScene::findFirstDirectionalLight(KRNode &node) {
    KRDirectionalLight *pLight = dynamic_cast<KRDirectionalLight *>(&node);
    if(pLight) {
        return pLight;
    } else {
        const std::vector<KRNode *> children = node.getChildren();
        for(std::vector<KRNode *>::const_iterator itr=children.begin(); itr < children.end(); ++itr) {
            pLight = findFirstDirectionalLight(*(*itr));
            if(pLight) {
                return pLight;
            }
        }
    }
    return NULL;
}

KRScene *KRScene::LoadXML(KRContext &context, const std::string& path)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile(path.c_str());
    KRScene *new_scene = new KRScene(context, KRResource::GetFileBase(path));
    
    KRNode *n = KRNode::LoadXML(context, doc.RootElement()->FirstChildElement());
    if(n) {
        new_scene->getRootNode()->addChild(n);
    }
    return new_scene;
}

KRDirectionalLight *KRScene::getFirstDirectionalLight()
{
    if(m_pFirstDirectionalLight == NULL) {
        m_pFirstDirectionalLight = findFirstDirectionalLight(*m_pRootNode);
    }
    return m_pFirstDirectionalLight;
}
