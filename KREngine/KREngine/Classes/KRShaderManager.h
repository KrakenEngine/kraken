//
//  KRShaderManager.h
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
#import <map>
#import <string>
#import "KRCamera.h"
#import "KRDataBlock.h"
#import "KRNode.h"

using std::map;
using std::vector;

#include "KRShader.h"

#ifndef KRSHADERMANAGER_H
#define KRSHADERMANAGER_H

class KRShader;
class KRCamera;

class KRShaderManager : public KRContextObject {
public:
    KRShaderManager(KRContext &context);
    virtual ~KRShaderManager();
    
    void loadFragmentShader(const std::string &name, KRDataBlock *data);
    void loadVertexShader(const std::string &name, KRDataBlock *data);
    const std::string &getFragShaderSource(const std::string &name);
    const std::string &getVertShaderSource(const std::string &name);
    
    
    KRShader *getShader(const std::string &shader_name, const KRCamera *pCamera, const std::stack<KRLight *> &lights, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, bool bReflectionMap, bool bReflectionCubeMap, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bReflectionMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bReflectionMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass);
    
    bool selectShader(const KRShader *pShader, const KRViewport &viewport, const KRMat4 &matModel, const std::stack<KRLight *> &lights, const KRNode::RenderPass &renderPass);
    
    bool selectShader(const std::string &shader_name, const KRCamera *pCamera, const std::stack<KRLight *> &lights, const KRViewport &viewport, const KRMat4 &matModel, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, bool bReflectionMap, bool bReflectionCubeMap, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bReflectionMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bReflectionMapOffset, bool bAlphaTest, bool bAlphaBlend, KRNode::RenderPass renderPass);
    
    long getShaderHandlesUsed();
    
private:
    std::map<std::string, KRShader *> m_shaders;
    
    std::map<std::string, std::string> m_fragShaderSource;
    std::map<std::string, std::string> m_vertShaderSource;
    
    KRShader *m_pShader;
    char m_szCurrentShaderKey[256];
};

#endif
