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
    setScaleCompensation(true);
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
    setScaleCompensation(true);
}

KRAABB KRBone::getBounds() {
    return KRAABB(-Vector3::One(), Vector3::One(), getModelMatrix()); // Only required for bone debug visualization
}

void KRBone::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    bool bVisualize = pCamera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_BONES;
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
        KRMat4 sphereModelMatrix = getModelMatrix();
        
        // Enable additive blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
        
        
        // Disable z-buffer write
        GLDEBUG(glDepthMask(GL_FALSE));
        
        // Disable z-buffer test
        GLDEBUG(glDisable(GL_DEPTH_TEST));

        KRShader *pShader = getContext().getShaderManager()->getShader("visualize_overlay", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
        
        if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, sphereModelMatrix, point_lights, directional_lights, spot_lights, 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
            std::vector<KRMesh *> sphereModels = getContext().getMeshManager()->getModel("__sphere");
            if(sphereModels.size()) {
                for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                    sphereModels[0]->renderSubmesh(i, renderPass, getName(), "visualize_overlay", 1.0f);
                }
            }

        }
        
        // Enable alpha blending
        GLDEBUG(glEnable(GL_BLEND));
        GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        
        // Enable z-buffer test
        GLDEBUG(glEnable(GL_DEPTH_TEST));
        GLDEBUG(glDepthFunc(GL_LEQUAL));
        GLDEBUG(glDepthRangef(0.0, 1.0));
        

    }
}


void KRBone::setBindPose(const KRMat4 &pose)
{
    m_bind_pose = pose;
}
const KRMat4 &KRBone::getBindPose()
{
    return m_bind_pose;
}
