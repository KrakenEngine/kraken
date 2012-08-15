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

using std::map;
using std::vector;

#include "KRShader.h"

#ifndef KRSHADERMANAGER_H
#define KRSHADERMANAGER_H

#define KRENGINE_MAX_SHADER_HANDLES 100

class KRShaderManager {
public:
    KRShaderManager();
    ~KRShaderManager();
    
    void loadFragmentShader(const std::string &name, const std::string &path);
    void loadVertexShader(const std::string &name, const std::string &path);
    const std::string &getFragShaderSource(const std::string &name);
    const std::string &getVertShaderSource(const std::string &name);
    
    
    KRShader *getShader(std::string shader_name, KRCamera *pCamera, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, int iShadowQuality, bool bLightMap, bool bDiffuseMapScale,bool bSpecMapScale, bool bNormalMapScale, bool bDiffuseMapOffset, bool bSpecMapOffset, bool bNormalMapOffset, bool bAlphaTest, KRNode::RenderPass renderPass);
    
private:
    std::map<std::string, KRShader *> m_shaders;
    
    std::map<std::string, std::string> m_fragShaderSource;
    std::map<std::string, std::string> m_vertShaderSource;
    
    KRShader *m_pShader;
};

#endif
