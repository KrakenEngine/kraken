//
//  KRInstance.h
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



#import "KREngine-common.h"

#ifndef KRINSTANCE_H
#define KRINSTANCE_H

#import "KRModel.h"
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRInstance.h"
#import "KRCamera.h"
#import "KRModelManager.h"
#import "KRNode.h"
#import "KRContext.h"
#import "KRModel.h"
#import "KRTexture.h"

class KRInstance : public KRNode {
    
public:
    KRInstance(KRScene &scene, std::string instance_name, std::string model_name, std::string light_map, float lod_min_coverage, bool receives_shadow);
    virtual ~KRInstance();
    
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, std::stack<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
#endif
    
    bool hasTransparency();
    
    
    virtual KRAABB getBounds();
    
private:
    std::vector<KRModel *> m_models;
    KRTexture *m_pLightMap;
    std::string m_lightMap;
    std::string m_model_name;
    
    
    float m_min_lod_coverage;
    void loadModel();
    
    bool m_receivesShadow;
};


#endif
