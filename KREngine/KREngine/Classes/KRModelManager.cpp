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
#include <assert.h>

#import "KRModel.h"

KRModelManager::KRModelManager(KRContext &context) : KRContextObject(context) {
    m_currentVBO.handle = 0;
    m_currentVBO.data = NULL;
    m_vboMemUsed = 0;
    m_bVBOAttribEnabled_Vertex = false;
    m_bVBOAttribEnabled_Normal = false;
    m_bVBOAttribEnabled_Tangent = false;
    m_bVBOAttribEnabled_UVA = false;
    m_bVBOAttribEnabled_UVB = false;
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
    std::map<std::string, KRModel *>::iterator itr_match = m_models.find(szName);
    if(itr_match == m_models.end()) {
        fprintf(stderr, "ERROR: Model not found: %s\n", szName);
        return NULL;
    } else {
        return itr_match->second;
    }
}

KRModel *KRModelManager::getFirstModel() {
    static std::map<std::string, KRModel *>::iterator model_itr = m_models.begin();
    return (*model_itr).second;
}

std::map<std::string, KRModel *> KRModelManager::getModels() {
    return m_models;
}

void KRModelManager::unbindVBO() {
    if(m_currentVBO.data != NULL) {
        GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, 0));
        m_currentVBO.size = 0;
        m_currentVBO.data = NULL;
        m_currentVBO.handle = -1;
    }
}

void KRModelManager::bindVBO(GLvoid *data, GLsizeiptr size, bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb) {
    
    if(m_currentVBO.data != data || m_currentVBO.size != size) {
        
        if(m_vbosActive.find(data) != m_vbosActive.end()) {
            m_currentVBO = m_vbosActive[data];
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.handle));
        } else if(m_vbosPool.find(data) != m_vbosPool.end()) {
            m_currentVBO = m_vbosPool[data];
            m_vbosPool.erase(data);
            m_vbosActive[data] = m_currentVBO;
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.handle));
        } else {
            m_vboMemUsed += size;
            
            while(m_vbosPool.size() + m_vbosActive.size() >= KRENGINE_MAX_VBO_HANDLES || m_vboMemUsed >= KRENGINE_MAX_VBO_MEM) {
                if(m_vbosPool.empty()) {
                    m_pContext->rotateBuffers();
                }
                std::map<GLvoid *, vbo_info_type>::iterator first_itr = m_vbosPool.begin();
                vbo_info_type firstVBO = first_itr->second;
                GLDEBUG(glDeleteBuffers(1, &firstVBO.handle));
                m_vboMemUsed -= firstVBO.size;
                m_vbosPool.erase(first_itr);
                //fprintf(stderr, "VBO Swapping...\n");
            }
            
            m_currentVBO.handle = -1;
            GLDEBUG(glGenBuffers(1, &m_currentVBO.handle));
            
            
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.handle));
            GLDEBUG(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
            
            m_currentVBO.size = size;
            m_currentVBO.data = data;
            
            m_vbosActive[data] = m_currentVBO;
        }
        
        configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb);
        
    }
    
//    fprintf(stderr, "VBO Mem: %i Kbyte    Texture Mem: %i Kbyte\n", (int)m_pContext->getModelManager()->getMemUsed() / 1024, (int)m_pContext->getTextureManager()->getMemUsed() / 1024);
}

void KRModelManager::configureAttribs(bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb)
{
    bool reconfigured = false;
    
    if(m_bVBOAttribEnabled_Vertex != enable_vertex) {
        if(enable_vertex) {
            GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX));
        } else {
            GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX));
        }
        m_bVBOAttribEnabled_Vertex = enable_vertex;
        reconfigured = true;
    }
    if(m_bVBOAttribEnabled_Normal != enable_normal) {
        if(enable_normal) {
            GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL));
        } else {
            GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL));
        }
        m_bVBOAttribEnabled_Normal = enable_normal;
        reconfigured = true;
    }
    if(m_bVBOAttribEnabled_Tangent != enable_tangent) {
        if(enable_tangent) {
            GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT));
        } else {
            GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT));
        }
        m_bVBOAttribEnabled_Tangent = enable_tangent;
        reconfigured = true;
    }
    if(m_bVBOAttribEnabled_UVA != enable_uva) {
        if(enable_uva) {
            GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA));
        } else {
            GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA));
        }
        m_bVBOAttribEnabled_UVA = enable_uva;
        reconfigured = true;
    }
    if(m_bVBOAttribEnabled_UVB != enable_uvb) {
        if(enable_uvb) {
            GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVB));
        } else {
            GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVB));
        }
        m_bVBOAttribEnabled_UVB = enable_uvb;
        reconfigured = true;
    }
    
    if(reconfigured || true) {
        int data_size = 0;
        if(enable_vertex) {
            data_size += sizeof(KRMesh::KRVector3D);
        }
        if(enable_normal) {
            data_size += sizeof(KRMesh::KRVector3D);
        }
        if(enable_tangent) {
            data_size += sizeof(KRMesh::KRVector3D);
        }
        if(enable_uva) {
            data_size += sizeof(KRMesh::TexCoord);
        }
        if(enable_uvb) {
            data_size += sizeof(KRMesh::TexCoord);
        }
        
        int offset = 0;
        if(enable_vertex) {
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
            offset += sizeof(KRMesh::KRVector3D);
        }
        if(enable_normal) {
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
            offset += sizeof(KRMesh::KRVector3D);
        }
        if(enable_tangent) {
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
            offset += sizeof(KRMesh::KRVector3D);
        }
        if(enable_uva) {
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
            offset += sizeof(KRMesh::TexCoord);
        }
        if(enable_uvb) {
            GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVB, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
            offset += sizeof(KRMesh::TexCoord);
        }
    }
}

long KRModelManager::getMemUsed()
{
    return m_vboMemUsed;
}

void KRModelManager::rotateBuffers()
{
    m_vbosPool.insert(m_vbosActive.begin(), m_vbosActive.end());
    m_vbosActive.clear();
    if(m_currentVBO.data != NULL) {
        // Ensure that the currently active VBO does not get flushed to free memory
        m_vbosPool.erase(m_currentVBO.data);
        m_vbosActive[m_currentVBO.data] = m_currentVBO;
    }

}
