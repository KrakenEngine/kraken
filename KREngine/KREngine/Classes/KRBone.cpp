//
//  KRBone.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRBone.h"
#include "KRContext.h"

KRBone::KRBone(KRScene &scene, std::string name) : KRNode(scene, name)
{

}

KRBone::~KRBone()
{
}

std::string KRBone::getElementName() {
    return "bone";
}

tinyxml2::XMLElement *KRBone::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);

    return e;
}

void KRBone::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}


#if TARGET_OS_IPHONE

void KRBone::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
    {
        
        KRNode::render(pCamera, lights, viewport, renderPass);
        
        bool bVisualize = true;
        
        if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
            KRMat4 sphereModelMatrix = getModelMatrix();

                KRShader *pShader = getContext().getShaderManager()->getShader("visualize_overlay", pCamera, lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
                
                if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, sphereModelMatrix, lights, 0, renderPass)) {

                    // Enable additive blending
                    GLDEBUG(glEnable(GL_BLEND));
                    GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));

                    
                    // Disable z-buffer write
                    GLDEBUG(glDepthMask(GL_FALSE));
                    
                    // Enable z-buffer test
                    GLDEBUG(glEnable(GL_DEPTH_TEST));
                    GLDEBUG(glDepthFunc(GL_LEQUAL));
                    GLDEBUG(glDepthRangef(0.0, 1.0));
                    
                    std::vector<KRModel *> sphereModels = getContext().getModelManager()->getModel("__sphere");
                    if(sphereModels.size()) {
                        for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                            sphereModels[0]->renderSubmesh(i);
                        }
                    }

                    // Enable alpha blending
                    GLDEBUG(glEnable(GL_BLEND));
                    GLDEBUG(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            }
        }
    }

#endif

