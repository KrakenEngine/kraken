//
//  KRCollider.cpp
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

#include "KREngine-common.h"
#include "KRCollider.h"
#include "KRContext.h"
#include "KRMesh.h"

/* static */
void KRCollider::InitNodeInfo(KrNodeInfo* nodeInfo)
{
  KRNode::InitNodeInfo(nodeInfo);
  nodeInfo->collider.audio_occlusion = 1.0f;
  nodeInfo->collider.layer_mask = 65535;
  nodeInfo->collider.mesh = -1;
}

KRCollider::KRCollider(KRScene &scene, std::string collider_name, std::string model_name, unsigned int layer_mask, float audio_occlusion) : KRNode(scene, collider_name) {
    m_model_name = model_name;
    m_layer_mask = layer_mask;
    m_audio_occlusion = audio_occlusion;
}

KRCollider::~KRCollider() {
    
}

std::string KRCollider::getElementName() {
    return "collider";
}

tinyxml2::XMLElement *KRCollider::saveXML( tinyxml2::XMLNode *parent)
{
    tinyxml2::XMLElement *e = KRNode::saveXML(parent);
    e->SetAttribute("mesh", m_model_name.c_str());
    e->SetAttribute("layer_mask", m_layer_mask);
    e->SetAttribute("audio_occlusion", m_audio_occlusion);
    return e;
}

void KRCollider::loadXML(tinyxml2::XMLElement *e) {
    KRNode::loadXML(e);

    m_model_name = e->Attribute("mesh");
    
    m_layer_mask = 65535;
    if(e->QueryUnsignedAttribute("layer_mask", &m_layer_mask) != tinyxml2::XML_SUCCESS) {
        m_layer_mask = 65535;
    }
    
    m_audio_occlusion = 1.0f;
    if(e->QueryFloatAttribute("audio_occlusion", &m_audio_occlusion) != tinyxml2::XML_SUCCESS) {
        m_audio_occlusion = 1.0f;
    }
}

void KRCollider::loadModel() {
    if(m_models.size() == 0) {
        m_models = m_pContext->getMeshManager()->getModel(m_model_name.c_str()); // The model manager returns the LOD levels in sorted order, with the highest detail first
        if(m_models.size() > 0) {
            getScene().notify_sceneGraphModify(this);
        }
    }
}

AABB KRCollider::getBounds() {
    loadModel();
    if(m_models.size() > 0) {
            return AABB::Create(m_models[0]->getMinPoint(), m_models[0]->getMaxPoint(), getModelMatrix());
    } else {
        return AABB::Infinite();
    }
}

bool KRCollider::lineCast(const Vector3 &v0, const Vector3 &v1, HitInfo &hitinfo, unsigned int layer_mask)
{
    if(layer_mask & m_layer_mask ) { // Only test if layer masks have a common bit set
        loadModel();
        if(m_models.size()) {
            if(getBounds().intersectsLine(v0, v1)) {
                Vector3 v0_model_space = Matrix4::Dot(getInverseModelMatrix(), v0);
                Vector3 v1_model_space = Matrix4::Dot(getInverseModelMatrix(), v1);
                HitInfo hitinfo_model_space;
                if(hitinfo.didHit()) { 
                    Vector3 hit_position_model_space = Matrix4::Dot(getInverseModelMatrix(), hitinfo.getPosition());
                    hitinfo_model_space = HitInfo(hit_position_model_space, Matrix4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal()), (hit_position_model_space - v0_model_space).magnitude(), hitinfo.getNode());
                }

                if(m_models[0]->lineCast(v0_model_space, v1_model_space, hitinfo_model_space)) {
                    Vector3 hit_position_world_space = Matrix4::Dot(getModelMatrix(), hitinfo_model_space.getPosition());
                    hitinfo = HitInfo(hit_position_world_space, Vector3::Normalize(Matrix4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), (hit_position_world_space - v0).magnitude(), this);
                    return true;
                }
            }
        }
    }
    return false;
}

bool KRCollider::rayCast(const Vector3 &v0, const Vector3 &dir, HitInfo &hitinfo, unsigned int layer_mask)
{
    if(layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
        loadModel();
        if(m_models.size()) {
            if(getBounds().intersectsRay(v0, dir)) {
                Vector3 v0_model_space = Matrix4::Dot(getInverseModelMatrix(), v0);
                Vector3 dir_model_space = Vector3::Normalize(Matrix4::DotNoTranslate(getInverseModelMatrix(), dir));
                HitInfo hitinfo_model_space;
                if(hitinfo.didHit()) {
                    Vector3 hit_position_model_space = Matrix4::Dot(getInverseModelMatrix(), hitinfo.getPosition());
                    hitinfo_model_space = HitInfo(hit_position_model_space, Vector3::Normalize(Matrix4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal())), (hit_position_model_space - v0_model_space).magnitude(), hitinfo.getNode());
                }

                if(m_models[0]->rayCast(v0_model_space, dir_model_space, hitinfo_model_space)) {
                    Vector3 hit_position_world_space = Matrix4::Dot(getModelMatrix(), hitinfo_model_space.getPosition());
                    hitinfo = HitInfo(hit_position_world_space, Vector3::Normalize(Matrix4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), (hit_position_world_space - v0).magnitude(), this);
                    return true;
                }
            }
        }
    }
    return false;
}

bool KRCollider::sphereCast(const Vector3 &v0, const Vector3 &v1, float radius, HitInfo &hitinfo, unsigned int layer_mask)
{
    if(layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
        loadModel();
        if(m_models.size()) {
            AABB sphereCastBounds = AABB::Create( // TODO - Need to cache this; perhaps encasulate within a "spherecast" class to be passed through these functions
                Vector3::Create(KRMIN(v0.x, v1.x) - radius, KRMIN(v0.y, v1.y) - radius, KRMIN(v0.z, v1.z) - radius),
                Vector3::Create(KRMAX(v0.x, v1.x) + radius, KRMAX(v0.y, v1.y) + radius, KRMAX(v0.z, v1.z) + radius)
            );
            
            if(getBounds().intersects(sphereCastBounds)) {
                if(m_models[0]->sphereCast(getModelMatrix(), v0, v1, radius, hitinfo)) {
                    hitinfo = HitInfo(hitinfo.getPosition(), hitinfo.getNormal(), hitinfo.getDistance(), this);
                    return true;
                }
            }
        }
    }
    return false;
}

unsigned int KRCollider::getLayerMask()
{
    return m_layer_mask;
}

void KRCollider::setLayerMask(unsigned int layer_mask)
{
    m_layer_mask = layer_mask;
}

float KRCollider::getAudioOcclusion()
{
    return m_audio_occlusion;
}

void KRCollider::setAudioOcclusion(float audio_occlusion)
{
    m_audio_occlusion = audio_occlusion;
}


void KRCollider::render(VkCommandBuffer& commandBuffer, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass)
{
    if(m_lod_visible <= LOD_VISIBILITY_PRESTREAM) return;
    
    KRNode::render(commandBuffer, pCamera, point_lights, directional_lights, spot_lights, viewport, renderPass);
    
    if(renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT && pCamera->settings.debug_display == KRRenderSettings::KRENGINE_DEBUG_DISPLAY_COLLIDERS) {
        loadModel();
        if(m_models.size()) {
            
            GL_PUSH_GROUP_MARKER("Debug Overlays");
            
            KRPipelineManager::PipelineInfo info{};
            std::string shader_name("visualize_overlay");
            info.shader_name = &shader_name;
            info.pCamera = pCamera;
            info.point_lights = &point_lights;
            info.directional_lights = &directional_lights;
            info.spot_lights = &spot_lights;
            info.renderPass = renderPass;

            KRPipeline *pShader = getContext().getPipelineManager()->getPipeline(info);
            
            if(getContext().getPipelineManager()->selectPipeline(*pCamera, pShader, viewport, getModelMatrix(), point_lights, directional_lights, spot_lights, 0, renderPass, Vector3::Zero(), 0.0f, Vector4::Zero())) {
                
                // Enable additive blending
                GLDEBUG(glEnable(GL_BLEND));
                GLDEBUG(glBlendFunc(GL_ONE, GL_ONE));
                
                
                // Disable z-buffer write
                GLDEBUG(glDepthMask(GL_FALSE));
                
                // Enable z-buffer test
                GLDEBUG(glEnable(GL_DEPTH_TEST));
                GLDEBUG(glDepthFunc(GL_LEQUAL));
                GLDEBUG(glDepthRangef(0.0, 1.0));


                for(int i=0; i < m_models[0]->getSubmeshCount(); i++) {
                    m_models[0]->renderSubmesh(commandBuffer, i, renderPass, getName(), "visualize_overlay", 1.0f);
                }
            
                // Enable alpha blending
                GLDEBUG(glEnable(GL_BLEND));
                GLDEBUG(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
            }
            
            GL_POP_GROUP_MARKER;
        }
    }
}
