//
//  KRModel.h
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



#include "KREngine-common.h"

#ifndef KRMODEL_H
#define KRMODEL_H

#include "KRMesh.h"
#include "KRMat4.h"
#include "KRVector3.h"
#include "KRModel.h"
#include "KRCamera.h"
#include "KRMeshManager.h"
#include "KRNode.h"
#include "KRContext.h"
#include "KRMesh.h"
#include "KRTexture.h"
#include "KRBone.h"

class KRModel : public KRNode {
    
public:
    KRModel(KRScene &scene, std::string instance_name, std::string model_name, std::string light_map, float lod_min_coverage, bool receives_shadow, bool faces_camera, KRVector3 rim_color = KRVector3::Zero(), float rim_power = 0.0f);
    virtual ~KRModel();
    
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);

    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    virtual KRAABB getBounds();
    
    void setRimColor(const const KRVector3 &rim_color);
    void setRimPower(float rim_power);
    KRVector3 getRimColor();
    float getRimPower();
    
    void setLightMap(const std::string &name);
    std::string getLightMap();
    
private:
    std::vector<KRMesh *> m_models;
    unordered_map<KRMesh *, std::vector<KRBone *> > m_bones; // Outer std::map connects model to set of bones
    KRTexture *m_pLightMap;
    std::string m_lightMap;
    std::string m_model_name;
    
    
    float m_min_lod_coverage;
    void loadModel();
    
    bool m_receivesShadow;
    bool m_faces_camera;
    
    
    KRMat4 m_boundsCachedMat;
    KRAABB m_boundsCached;
    
    
    KRVector3 m_rim_color;
    float m_rim_power;
};


#endif
