//
//  KRCollider.cpp
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
#include "KRCollider.h"
#import "KRContext.h"
#import "KRMesh.h"
#import "KRQuaternion.h"
#include <assert.h>

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
        m_models = m_pContext->getModelManager()->getModel(m_model_name.c_str()); // The model manager returns the LOD levels in sorted order, with the highest detail first
        if(m_models.size() > 0) {
            getScene().notify_sceneGraphModify(this);
        }
    }
}

KRAABB KRCollider::getBounds() {
    loadModel();
    if(m_models.size() > 0) {
            return KRAABB(m_models[0]->getMinPoint(), m_models[0]->getMaxPoint(), getModelMatrix());
    } else {
        return KRAABB::Infinite();
    }
}

bool KRCollider::lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo, unsigned int layer_mask)
{
    if(layer_mask & m_layer_mask ) { // Only test if layer masks have a common bit set
        loadModel();
        if(m_models.size()) {
            if(getBounds().intersectsLine(v0, v1)) {
                KRHitInfo hitinfo_model_space = KRHitInfo(KRMat4::Dot(getInverseModelMatrix(), hitinfo.getPosition()), KRMat4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal()), hitinfo.getNode());
                KRVector3 v0_model_space = KRMat4::Dot(getInverseModelMatrix(), v0);
                KRVector3 v1_model_space = KRMat4::Dot(getInverseModelMatrix(), v1);
                if(m_models[0]->lineCast(v0_model_space, v1_model_space, hitinfo_model_space)) {
                    hitinfo = KRHitInfo(KRMat4::Dot(getModelMatrix(), hitinfo_model_space.getPosition()), KRVector3::Normalize(KRMat4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), this);
                    return true;
                }
            }
        }
    }
    return false;
}

bool KRCollider::rayCast(const KRVector3 &v0, const KRVector3 &dir, KRHitInfo &hitinfo, unsigned int layer_mask)
{
    if(layer_mask & m_layer_mask) { // Only test if layer masks have a common bit set
        loadModel();
        if(m_models.size()) {
            if(getBounds().intersectsRay(v0, dir)) {
                KRHitInfo hitinfo_model_space = KRHitInfo(KRMat4::Dot(getInverseModelMatrix(), hitinfo.getPosition()), KRVector3::Normalize(KRMat4::DotNoTranslate(getInverseModelMatrix(), hitinfo.getNormal())), hitinfo.getNode());
                KRVector3 v0_model_space = KRMat4::Dot(getInverseModelMatrix(), v0);
                KRVector3 dir_model_space = KRMat4::DotNoTranslate(getInverseModelMatrix(), dir);
                if(m_models[0]->rayCast(v0_model_space, dir, hitinfo_model_space)) {
                    hitinfo = KRHitInfo(KRMat4::Dot(getModelMatrix(), hitinfo_model_space.getPosition()), KRVector3::Normalize(KRMat4::DotNoTranslate(getModelMatrix(), hitinfo_model_space.getNormal())), this);
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
