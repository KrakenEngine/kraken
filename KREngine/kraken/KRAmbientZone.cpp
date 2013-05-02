//
//  KRAmbientZone.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-12-06.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAmbientZone.h"
#include "KRContext.h"

KRAmbientZone::KRAmbientZone(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_ambient = "";
    m_ambient_gain = 1.0f;

    m_gradient_distance = 0.25f;
    
}

KRAmbientZone::~KRAmbientZone()
{
}

std::string KRAmbientZone::getElementName() {
    return "ambient_zone";
}

tinyxml2::XMLElement *KRAmbientZone::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("zone", m_zone.c_str());
    e->SetAttribute("sample", m_ambient.c_str());
    e->SetAttribute("gain", m_ambient_gain);
    e->SetAttribute("gradient", m_gradient_distance);
    return e;
}

void KRAmbientZone::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
    
    m_zone = e->Attribute("zone");
    
    m_gradient_distance = 0.25f;
    if(e->QueryFloatAttribute("gradient", &m_gradient_distance) != tinyxml2::XML_SUCCESS) {
        m_gradient_distance = 0.25f;
    }
    
    m_ambient = e->Attribute("ambient");
    
    m_ambient_gain = 1.0f;
    if(e->QueryFloatAttribute("ambient_gain", &m_ambient_gain) != tinyxml2::XML_SUCCESS) {
        m_ambient_gain = 1.0f;
    }
}

std::string KRAmbientZone::getAmbient()
{
    return m_ambient;
}

void KRAmbientZone::setAmbient(const std::string &ambient)
{
    m_ambient = ambient;
}

float KRAmbientZone::getAmbientGain()
{
    return m_ambient_gain;
}

void KRAmbientZone::setAmbientGain(float ambient_gain)
{
    m_ambient_gain = ambient_gain;
}

std::string KRAmbientZone::getZone()
{
    return m_zone;
}

void KRAmbientZone::setZone(const std::string &zone)
{
    m_zone = zone;
}

void KRAmbientZone::render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    
    KRNode::render(pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    bool bVisualize = false;
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
        KRMat4 sphereModelMatrix = getModelMatrix();
        
        KRShader *pShader = getContext().getShaderManager()->getShader("visualize_overlay", pCamera, point_lights, directional_lights, spot_lights, 0, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, renderPass);
        
        if(getContext().getShaderManager()->selectShader(*pCamera, pShader, viewport, sphereModelMatrix, point_lights, directional_lights, spot_lights, 0, renderPass)) {
            
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
                    sphereModels[0]->renderSubmesh(i, renderPass, getName(), "visualize_overlay");
                }
            }
            
            // Enable alpha blending
            GLDEBUG(glEnable(GL_BLEND));
            GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
        }
    }
}


float KRAmbientZone::getGradientDistance()
{
    return m_gradient_distance;
}

void KRAmbientZone::setGradientDistance(float gradient_distance)
{
    m_gradient_distance = gradient_distance;
}

KRAABB KRAmbientZone::getBounds() {
    // Ambient zones always have a -1, -1, -1 to 1, 1, 1 bounding box
    return KRAABB(-KRVector3::One(), KRVector3::One(), getModelMatrix());
}

float KRAmbientZone::getContainment(const KRVector3 &pos)
{
    KRAABB bounds = getBounds();
    if(bounds.contains(pos)) {
        KRVector3 size = bounds.size();
        KRVector3 diff = pos - bounds.center();
        diff = diff * 2.0f;
        diff = KRVector3(diff.x / size.x, diff.y / size.y, diff.z / size.z);
        float d = diff.magnitude();
    
        if(m_gradient_distance <= 0.0f) {
            // Avoid division by zero
            d = d > 1.0f ? 0.0f : 1.0f;
        } else {
            d = (1.0f - d) / m_gradient_distance;
            d = KRCLAMP(d, 0.0f, 1.0f);
        }
        return d;
        
    } else {
        return 0.0f;
    }
}