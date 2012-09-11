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
#import <stdint.h>
#import <vector>
#import <set>
#import <string>
#import "KRMesh.h"
#import "KRVector2.h"
#import "KRContext.h"

#import "KREngine-common.h"

using std::vector;
using std::set;

#ifndef KRMODEL_I
#define KRMODEL_I

#import "KRMaterialManager.h"
#import "KRCamera.h"

class KRMaterial;

class KRModel : public KRContextObject {
    
public:
    KRModel(KRContext &context, std::string name, KRDataBlock *data);
    virtual ~KRModel();
    
    bool hasTransparency();
    
#if TARGET_OS_IPHONE
    
    void render(KRCamera *pCamera, KRContext *pContext, KRMat4 &matModelToView, KRMat4 &mvpMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRTexture *pLightMap, KRNode::RenderPass renderPass);
    
#endif
    
    KRMesh *getMesh();
    std::string getName();

private:    
    vector<KRMaterial *> m_materials;
    set<KRMaterial *> m_uniqueMaterials;
    KRMesh *m_pMesh;
    std::string m_name;
    
    bool m_hasTransparency;

};


#endif // KRMODEL_I