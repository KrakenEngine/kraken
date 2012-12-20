//
//  KRScene.h
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

#ifndef KRSCENE_H
#define KRSCENE_H

#import "KREngine-common.h"

#import <vector>
#import "KRInstance.h"
#import "KRMat4.h"
#import "KRModel.h"
#import "KRCamera.h"
#import "KRModelManager.h"
#import "KRNode.h"
#import "KROctree.h"
class KRInstance;
class KRLight;

using std::vector;

class KRScene : public KRResource {
public:
    KRScene(KRContext &context, std::string name);
    virtual ~KRScene();
    
    virtual std::string getExtension();
    virtual bool save(KRDataBlock &data);
    
    static KRScene *Load(KRContext &context, const std::string &name, KRDataBlock *data);
    
    KRNode *getRootNode();
    KRLight *getFirstLight();
    
    bool lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo);
    bool rayCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo);
    
#if TARGET_OS_IPHONE
    
    void render(KRCamera *pCamera, std::map<KRAABB, int> &visibleBounds, const KRViewport &viewport, KRNode::RenderPass renderPass, bool new_frame);
    
    void render(KROctreeNode *pOctreeNode, std::map<KRAABB, int> &visibleBounds, KRCamera *pCamera, std::vector<KRLight *> lights, const KRViewport &viewport, KRNode::RenderPass renderPass, std::vector<KROctreeNode *> &remainingOctrees, std::vector<KROctreeNode *> &remainingOctreesTestResults, std::vector<KROctreeNode *> &remainingOctreesTestResultsOnly, bool bOcclusionResultsPass, bool bOcclusionTestResultsOnly);
    
#endif
    
    
    void notify_sceneGraphCreate(KRNode *pNode);
    void notify_sceneGraphDelete(KRNode *pNode);
    void notify_sceneGraphModify(KRNode *pNode);
    
    void physicsUpdate(float deltaTime);
    void addDefaultLights();
    
    KRAABB getRootOctreeBounds();
    
private:
    KRLight *findFirstLight(KRNode &node);
    
    KRNode *m_pRootNode;
    KRLight *m_pFirstLight;
    
    std::set<KRNode *> m_newNodes;
    std::set<KRNode *> m_modifiedNodes;
    
    
    
    std::set<KRNode *> m_physicsNodes;
    
    KROctree m_nodeTree;
    void updateOctree();
    
    std::string m_skyBoxName;
};



#endif
