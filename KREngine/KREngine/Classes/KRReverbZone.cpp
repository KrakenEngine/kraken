//
//  KRReverbZone.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRReverbZone.h"
#include "KRContext.h"

KRReverbZone::KRReverbZone(KRScene &scene, std::string name) : KRNode(scene, name)
{
    
}

KRReverbZone::~KRReverbZone()
{
}

std::string KRReverbZone::getElementName() {
    return "reverb_zone";
}

tinyxml2::XMLElement *KRReverbZone::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    return e;
}

void KRReverbZone::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
}

std::string KRReverbZone::getReverbPreset()
{
    return m_reverb_preset_name;
}

void KRReverbZone::setReverbPreset(const std::string &reverb_preset_name)
{
    m_reverb_preset_name = reverb_preset_name;
}


unsigned int KRReverbZone::getReverbSettingId()
{
    if(m_reverb_preset_name.compare("small_room") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_SmallRoom;
    } else if(m_reverb_preset_name.compare("medium_room") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_MediumRoom;
    } else if(m_reverb_preset_name.compare("large_room") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_LargeRoom;
    } else if(m_reverb_preset_name.compare("medium_hall") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_MediumHall;
    } else if(m_reverb_preset_name.compare("large_hall") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_LargeHall;
    } else if(m_reverb_preset_name.compare("plate") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_Plate;
    } else if(m_reverb_preset_name.compare("medium_chamber") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_MediumChamber;
    } else if(m_reverb_preset_name.compare("large_chamber") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_LargeChamber;
    } else if(m_reverb_preset_name.compare("cathedral") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_Cathedral;
    } else if(m_reverb_preset_name.compare("large_room2") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_LargeRoom2;
    } else if(m_reverb_preset_name.compare("medium_hall2") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_MediumHall2;
    } else if(m_reverb_preset_name.compare("medium_hall3") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_MediumHall3;
    } else if(m_reverb_preset_name.compare("large_hall2") == 0) {
        return ALC_ASA_REVERB_ROOM_TYPE_LargeHall2;
    } else {
        return ALC_ASA_REVERB_ROOM_TYPE_SmallRoom;
    }
}

void KRReverbZone::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    
    KRNode::render(pCamera, lights, viewport, renderPass);
    
    bool bVisualize = false;
    
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
            std::vector<KRMesh *> sphereModels = getContext().getModelManager()->getModel("__sphere");
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


float KRReverbZone::getGradientDistance()
{
    return m_gradient_distance;
}

void KRReverbZone::setGradientDistance(float gradient_distance)
{
    m_gradient_distance = gradient_distance;
}

KRAABB KRReverbZone::getBounds() {
    // Reverb zones always have a -1, -1, -1 to 1, 1, 1 bounding box
    return KRAABB(-KRVector3::One(), KRVector3::One(), getModelMatrix());
}
