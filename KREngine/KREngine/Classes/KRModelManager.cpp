//
//  KRModelManager.cpp
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

#include "KRModelManager.h"

#import "KRModel.h"

KRModelManager::KRModelManager(KRContext &context) : KRContextObject(context) {
    m_currentVBO.handle = 0;
    m_currentVBO.data = NULL;
    m_vboMemUsed = 0;
}

KRModelManager::~KRModelManager() {
    for(map<std::string, KRModel *>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr){
        delete (*itr).second;
    }
    m_models.empty();
}

KRModel *KRModelManager::loadModel(const char *szName, KRDataBlock *pData) {
    KRModel *pModel = new KRModel(*m_pContext, szName, pData);
    m_models[szName] = pModel;
    return pModel;
}

KRModel *KRModelManager::getModel(const char *szName) {
    return m_models[szName];
}

KRModel *KRModelManager::getFirstModel() {
    static std::map<std::string, KRModel *>::iterator model_itr = m_models.begin();
    return (*model_itr).second;
}

std::map<std::string, KRModel *> KRModelManager::getModels() {
    return m_models;
}

void KRModelManager::bindVBO(const GLvoid *data, GLsizeiptr size) {
    
    if(m_currentVBO.data != data || m_currentVBO.size != size) {
        
        if(m_vbos.find(data) != m_vbos.end()) {
            m_currentVBO = m_vbos[data];
            glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.handle);
        } else {
            m_vboMemUsed += size;
            
            while(m_vbos.size() >= KRENGINE_MAX_VBO_HANDLES || m_vboMemUsed >= KRENGINE_MAX_VBO_MEM) {
                // TODO - This should maintain a max size limit for VBO's rather than a limit on the number of VBO's.  As meshes are split to multiple small VBO's, this is not too bad, but still not optimial.
                std::map<const GLvoid *, vbo_info_type>::iterator first_itr = m_vbos.begin();
                vbo_info_type firstVBO = first_itr->second;
                glDeleteBuffers(1, &firstVBO.handle);
                m_vboMemUsed -= firstVBO.size;
                m_vbos.erase(first_itr);
                fprintf(stderr, "VBO Swapping...\n");
            }
            
            m_currentVBO.handle = -1;
            glGenBuffers(1, &m_currentVBO.handle);
            glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.handle);
            glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
            
            m_currentVBO.size = size;
            m_currentVBO.data = data;
            
            m_vbos[data] = m_currentVBO;
        }
    }
}

long KRModelManager::getMemUsed()
{
    return m_vboMemUsed;
}
