//
//  KRAmbientSphere.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAmbientSphere.h"
#include "KRContext.h"

KRAmbientSphere::KRAmbientSphere(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_ambient = "";
    m_reverb = "";
    m_ambient_gain = 1.0f;
    m_reverb_gain = 1.0f;
    m_gradient_distance = 1.0f;
    
}

KRAmbientSphere::~KRAmbientSphere()
{
}

std::string KRAmbientSphere::getElementName() {
    return "ambient_sphere";
}

tinyxml2::XMLElement *KRAmbientSphere::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("zone", m_zone.c_str());
    e->SetAttribute("ambient", m_ambient.c_str());
    e->SetAttribute("ambient_gain", m_ambient_gain);
    e->SetAttribute("reverb", m_reverb.c_str());
    e->SetAttribute("reverb_gain", m_reverb_gain);
    e->SetAttribute("gradient", m_gradient_distance);
    return e;
}

void KRAmbientSphere::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
    
    m_zone = e->Attribute("zone");
    
    m_gradient_distance = 1.0f;
    if(e->QueryFloatAttribute("gradient", &m_gradient_distance) != tinyxml2::XML_SUCCESS) {
        m_gradient_distance = 1.0f;
    }
    
    m_reverb = e->Attribute("reverb");
    
    m_reverb_gain = 1.0f;
    if(e->QueryFloatAttribute("reverb_gain", &m_reverb_gain) != tinyxml2::XML_SUCCESS) {
        m_reverb_gain = 1.0f;
    }
    
    m_ambient = e->Attribute("ambient");
    
    m_ambient_gain = 1.0f;
    if(e->QueryFloatAttribute("ambient_gain", &m_ambient_gain) != tinyxml2::XML_SUCCESS) {
        m_ambient_gain = 1.0f;
    }
}

std::string KRAmbientSphere::getReverb()
{
    return m_reverb;
}

void KRAmbientSphere::setReverb(const std::string &reverb)
{
    m_reverb = reverb;
}

float KRAmbientSphere::getReverbGain()
{
    return m_reverb_gain;
}

void KRAmbientSphere::setReverbGain(float reverb_gain)
{
    m_reverb_gain = reverb_gain;
}

std::string KRAmbientSphere::getAmbient()
{
    return m_ambient;
}

void KRAmbientSphere::setAmbient(const std::string &ambient)
{
    m_ambient = ambient;
}

float KRAmbientSphere::getAmbientGain()
{
    return m_ambient_gain;
}

void KRAmbientSphere::setAmbientGain(float ambient_gain)
{
    m_ambient_gain = ambient_gain;
}

std::string KRAmbientSphere::getZone()
{
    return m_zone;
}

void KRAmbientSphere::setZone(const std::string &zone)
{
    m_zone = zone;
}

void KRAmbientSphere::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
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


float KRAmbientSphere::getGradientDistance()
{
    return m_gradient_distance;
}

void KRAmbientSphere::setGradientDistance(float gradient_distance)
{
    m_gradient_distance = gradient_distance;
}

KRAABB KRAmbientSphere::getBounds() {
    // Reverb zones always have a -1, -1, -1 to 1, 1, 1 bounding box
    return KRAABB(-KRVector3::One(), KRVector3::One(), getModelMatrix());
}
