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
    m_currentVBO.vbo_handle = 0;
    m_currentVBO.vao_handle = 0;
    m_currentVBO.data = NULL;
    m_vboMemUsed = 0;
}

KRModelManager::~KRModelManager() {
    for(std::multimap<std::string, KRModel *>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr){
        delete (*itr).second;
    }
    m_models.empty();
}

KRModel *KRModelManager::loadModel(const char *szName, KRDataBlock *pData) {
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    
    KRModel *pModel = new KRModel(*m_pContext, lowerName, pData);
    m_models.insert(std::pair<std::string, KRModel *>(pModel->getLODBaseName(), pModel));

    return pModel;
}

std::vector<KRModel *> KRModelManager::getModel(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    
    std::vector<KRModel *> matching_models;
    
    std::pair<std::multimap<std::string, KRModel *>::iterator, std::multimap<std::string, KRModel *>::iterator> range = m_models.equal_range(lowerName);
    for(std::multimap<std::string, KRModel *>::iterator itr_match = range.first; itr_match != range.second; itr_match++) {
        matching_models.push_back(itr_match->second);
    }
    
    std::sort(matching_models.begin(), matching_models.end(), KRModel::lod_sort_predicate);
    
    if(matching_models.size() == 0) {
        fprintf(stderr, "ERROR: Model not found: %s\n", lowerName.c_str());
    }
    
    return matching_models;
}

std::multimap<std::string, KRModel *> KRModelManager::getModels() {
    return m_models;
}

void KRModelManager::unbindVBO() {
    if(m_currentVBO.data != NULL) {
        GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, 0));
        m_currentVBO.size = 0;
        m_currentVBO.data = NULL;
        m_currentVBO.vbo_handle = -1;
        m_currentVBO.vao_handle = -1;
    }
}

void KRModelManager::bindVBO(GLvoid *data, GLsizeiptr size, bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb) {
    
    if(m_currentVBO.data != data || m_currentVBO.size != size) {
        
        if(m_vbosActive.find(data) != m_vbosActive.end()) {
            m_currentVBO = m_vbosActive[data];
#if GL_OES_vertex_array_object
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#else
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb);
#endif
        } else if(m_vbosPool.find(data) != m_vbosPool.end()) {
            m_currentVBO = m_vbosPool[data];
            m_vbosPool.erase(data);
            m_vbosActive[data] = m_currentVBO;
#if GL_OES_vertex_array_object
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#else
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb);
#endif
        } else {
            m_vboMemUsed += size;
            
            while(m_vbosPool.size() + m_vbosActive.size() >= KRContext::KRENGINE_MAX_VBO_HANDLES || m_vboMemUsed >= KRContext::KRENGINE_MAX_VBO_MEM) {
                if(m_vbosPool.empty()) {
                    fprintf(stderr, "flushBuffers due to VBO exhaustion...\n");
                    m_pContext->rotateBuffers(false);
                }
                std::map<GLvoid *, vbo_info_type>::iterator first_itr = m_vbosPool.begin();
                vbo_info_type firstVBO = first_itr->second;
#if GL_OES_vertex_array_object
                GLDEBUG(glDeleteVertexArraysOES(1, &firstVBO.vao_handle));
#endif
                GLDEBUG(glDeleteBuffers(1, &firstVBO.vbo_handle));
                m_vboMemUsed -= firstVBO.size;
                m_vbosPool.erase(first_itr);
                fprintf(stderr, "VBO Swapping...\n");
            }
            
            m_currentVBO.vao_handle = -1;
            m_currentVBO.vbo_handle = -1;
            GLDEBUG(glGenBuffers(1, &m_currentVBO.vbo_handle));
#if GL_OES_vertex_array_object
            GLDEBUG(glGenVertexArraysOES(1, &m_currentVBO.vao_handle));
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#endif

            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            GLDEBUG(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb);
            
            m_currentVBO.size = size;
            m_currentVBO.data = data;
            
            m_vbosActive[data] = m_currentVBO;
        }
    }
}

void KRModelManager::configureAttribs(bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb)
{
    if(enable_vertex) {
        GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX));
    }

    if(enable_normal) {
        GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL));
    }

    if(enable_tangent) {
        GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT));
    }

    if(enable_uva) {
        GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA));
    }

    if(enable_uvb) {
        GLDEBUG(glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVB));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVB));
    }

    int data_size = 0;
    if(enable_vertex) {
        data_size += sizeof(KRModel::KRVector3D);
    }
    if(enable_normal) {
        data_size += sizeof(KRModel::KRVector3D);
    }
    if(enable_tangent) {
        data_size += sizeof(KRModel::KRVector3D);
    }
    if(enable_uva) {
        data_size += sizeof(KRModel::TexCoord);
    }
    if(enable_uvb) {
        data_size += sizeof(KRModel::TexCoord);
    }
    
    int offset = 0;
    if(enable_vertex) {
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
        offset += sizeof(KRModel::KRVector3D);
    }
    if(enable_normal) {
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
        offset += sizeof(KRModel::KRVector3D);
    }
    if(enable_tangent) {
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
        offset += sizeof(KRModel::KRVector3D);
    }
    if(enable_uva) {
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
        offset += sizeof(KRModel::TexCoord);
    }
    if(enable_uvb) {
        GLDEBUG(glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVB, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(offset)));
        offset += sizeof(KRModel::TexCoord);
    }
}

long KRModelManager::getMemUsed()
{
    return m_vboMemUsed;
}

void KRModelManager::rotateBuffers(bool new_frame)
{
    m_vbosPool.insert(m_vbosActive.begin(), m_vbosActive.end());
    m_vbosActive.clear();
    if(m_currentVBO.data != NULL) {
        // Ensure that the currently active VBO does not get flushed to free memory
        m_vbosPool.erase(m_currentVBO.data);
        m_vbosActive[m_currentVBO.data] = m_currentVBO;
    }

}
