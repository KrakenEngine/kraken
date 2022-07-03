//
//  KRReverbZone.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#include "KRReverbZone.h"
#include "KRContext.h"

/* static */
void KRReverbZone::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->reverb_zone.gain = 1.0f;
  nodeInfo->reverb_zone.gradient = 0.25f;
  nodeInfo->reverb_zone.sample = -1;
  nodeInfo->reverb_zone.pZoneName = nullptr;
}

KRReverbZone::KRReverbZone(KRScene &scene, std::string name) : KRNode(scene, name)
{
    m_reverb = "";
    m_reverb_gain = 1.0f;
    m_gradient_distance = 0.25f;
    
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
    e->SetAttribute("zone", m_zone.c_str());
    e->SetAttribute("sample", m_reverb.c_str());
    e->SetAttribute("gain", m_reverb_gain);
    e->SetAttribute("gradient", m_gradient_distance);
    return e;
}

void KRReverbZone::loadXML(tinyxml2::XMLElement *e)
{
    KRNode::loadXML(e);
    
    m_zone = e->Attribute("zone");
    
    m_gradient_distance = 0.25f;
    if(e->QueryFloatAttribute("gradient", &m_gradient_distance) != tinyxml2::XML_SUCCESS) {
        m_gradient_distance = 0.25f;
    }
    
    m_reverb = e->Attribute("sample");
    
    m_reverb_gain = 1.0f;
    if(e->QueryFloatAttribute("gain", &m_reverb_gain) != tinyxml2::XML_SUCCESS) {
        m_reverb_gain = 1.0f;
    }
}

std::string KRReverbZone::getReverb()
{
    return m_reverb;
}

void KRReverbZone::setReverb(const std::string &reverb)
{
    m_reverb = reverb;
}

float KRReverbZone::getReverbGain()
{
    return m_reverb_gain;
}

void KRReverbZone::setReverbGain(float reverb_gain)
{
    m_reverb_gain = reverb_gain;
}

std::string KRReverbZone::getZone()
{
    return m_zone;
}

void KRReverbZone::setZone(const std::string &zone)
{
    m_zone = zone;
}

void KRReverbZone::render(RenderInfo& ri)
{
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(ri);
    
    bool bVisualize = ri.camera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_SIREN_REVERB_ZONES;
    
    if(ri.renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && bVisualize) {
        Matrix4 sphereModelMatrix = getModelMatrix();
        PipelineInfo info{};
        std::string shader_name("visualize_overlay");
        info.shader_name = &shader_name;
        info.pCamera = ri.camera;
        info.point_lights = &ri.point_lights;
        info.directional_lights = &ri.directional_lights;
        info.spot_lights = &ri.spot_lights;
        info.renderPass = ri.renderPass;

        KRPipeline *pShader = getContext().getPipelineManager()->getPipeline(*ri.surface, info);
        info.rasterMode = PipelineInfo::RasterMode::kAdditive;
        
        if(getContext().getPipelineManager()->selectPipeline(*ri.surface, *ri.camera, pShader, ri.viewport, sphereModelMatrix, &ri.point_lights, &ri.directional_lights, &ri.spot_lights, 0, ri.renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
            
            // Disable z-buffer write
            GLDEBUG(glDepthMask(GL_FALSE));
            
            // Enable z-buffer test
            GLDEBUG(glEnable(GL_DEPTH_TEST));
            GLDEBUG(glDepthFunc(GL_LEQUAL));
            GLDEBUG(glDepthRangef(0.0, 1.0));
            std::vector<KRMesh *> sphereModels = getContext().getMeshManager()->getModel("__sphere");
            if(sphereModels.size()) {
                for(int i=0; i < sphereModels[0]->getSubmeshCount(); i++) {
                    sphereModels[0]->renderSubmesh(ri.commandBuffer, i, ri.renderPass, getName(), "visualize_overlay", 1.0f);
                }
            }

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

AABB KRReverbZone::getBounds() {
    // Reverb zones always have a -1, -1, -1 to 1, 1, 1 bounding box
    return AABB::Create(-Vector3::One(), Vector3::One(), getModelMatrix());
}

float KRReverbZone::getContainment(const Vector3 &pos)
{
    AABB bounds = getBounds();
    if(bounds.contains(pos)) {
        Vector3 size = bounds.size();
        Vector3 diff = pos - bounds.center();
        diff = diff * 2.0f;
        diff = Vector3::Create(diff.x / size.x, diff.y / size.y, diff.z / size.z);
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