//
//  KRMeshManager.cpp
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

#include "KRMeshManager.h"

#include "KRMesh.h"
#include "KRMeshCube.h"
#include "KRMeshSphere.h"

KRMeshManager::KRMeshManager(KRContext &context) : KRContextObject(context) {
    m_currentVBO.vbo_handle = -1;
    m_currentVBO.vbo_handle_indexes = -1;
    m_currentVBO.vao_handle = -1;
    m_currentVBO.data = NULL;
    m_vboMemUsed = 0;
    m_randomParticleVertexData = NULL;
    m_volumetricLightingVertexData = NULL;
    m_memoryTransferredThisFrame = 0;
    
//    addModel(new KRMeshCube(context)); // FINDME - HACK!  This needs to be fixed, as it currently segfaults
    
    addModel(new KRMeshSphere(context));
}

KRMeshManager::~KRMeshManager() {
    for(std::multimap<std::string, KRMesh *>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr){
        delete (*itr).second;
    }
    m_models.empty();
    if(m_randomParticleVertexData != NULL) delete m_randomParticleVertexData;
    if(m_volumetricLightingVertexData != NULL) delete m_volumetricLightingVertexData;
}

KRMesh *KRMeshManager::loadModel(const char *szName, KRDataBlock *pData) {
    
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    
    KRMesh *pModel = new KRMesh(*m_pContext, lowerName, pData);
    addModel(pModel);
    return pModel;
}

void KRMeshManager::addModel(KRMesh *model) {
    m_models.insert(std::pair<std::string, KRMesh *>(model->getLODBaseName(), model));
}

std::vector<KRMesh *> KRMeshManager::getModel(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    
    std::vector<KRMesh *> matching_models;
    
    std::pair<std::multimap<std::string, KRMesh *>::iterator, std::multimap<std::string, KRMesh *>::iterator> range = m_models.equal_range(lowerName);
    for(std::multimap<std::string, KRMesh *>::iterator itr_match = range.first; itr_match != range.second; itr_match++) {
        matching_models.push_back(itr_match->second);
    }
    
    std::sort(matching_models.begin(), matching_models.end(), KRMesh::lod_sort_predicate);
    
    if(matching_models.size() == 0) {        
        fprintf(stderr, "ERROR: Model not found: %s\n", lowerName.c_str());
    }
    
    return matching_models;
}

std::multimap<std::string, KRMesh *> KRMeshManager::getModels() {
    return m_models;
}

void KRMeshManager::unbindVBO() {
    if(m_currentVBO.data != NULL) {
        GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        m_currentVBO.size = 0;
        m_currentVBO.data = NULL;
        m_currentVBO.vbo_handle = -1;
        m_currentVBO.vbo_handle_indexes = -1;
        m_currentVBO.vao_handle = -1;
    }
}

void KRMeshManager::releaseVBO(GLvoid *data)
{
    if(m_currentVBO.data == data) {
        unbindVBO();
    }

    vbo_info_type vbo_to_release;
    if(m_vbosActive.find(data) != m_vbosActive.end()) {
        fprintf(stderr, "glFinish called due to releasing a VBO that is active in the current frame.\n");
        GLDEBUG(glFinish());
        
        // The VBO is active
        vbo_to_release = m_vbosActive[data];
        m_vbosActive.erase(data);
    } else {
        // The VBO is inactive
        vbo_to_release = m_vbosPool[data];
        m_vbosPool.erase(data);
    }
    
    m_vboMemUsed -= vbo_to_release.size;
    
#if GL_OES_vertex_array_object
    GLDEBUG(glDeleteVertexArraysOES(1, &vbo_to_release.vao_handle));
#endif
    GLDEBUG(glDeleteBuffers(1, &vbo_to_release.vbo_handle));
    if(vbo_to_release.vbo_handle_indexes != -1) {
        GLDEBUG(glDeleteBuffers(1, &vbo_to_release.vbo_handle_indexes));
    }
}

void KRMeshManager::bindVBO(GLvoid *data, GLsizeiptr size, GLvoid *index_data, GLsizeiptr index_data_size, bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb, bool enable_bone_indexes, bool enable_bone_weights, bool static_vbo) {
    
    if(m_currentVBO.data != data || m_currentVBO.size != size) {
        
        if(m_vbosActive.find(data) != m_vbosActive.end()) {
            m_currentVBO = m_vbosActive[data];
#if GL_OES_vertex_array_object
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#else
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb, enable_bone_indexes, enable_bone_weights);
#endif
        } else if(m_vbosPool.find(data) != m_vbosPool.end()) {
            m_currentVBO = m_vbosPool[data];
            m_vbosPool.erase(data);
            m_vbosActive[data] = m_currentVBO;
#if GL_OES_vertex_array_object
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#else
            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb, enable_bone_indexes, enable_bone_weights);
#endif
        } else {
            
            
            while(m_vbosPool.size() + m_vbosActive.size() + 1 >= KRContext::KRENGINE_MAX_VBO_HANDLES || m_vboMemUsed + size >= KRContext::KRENGINE_MAX_VBO_MEM) {
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
                if(firstVBO.vbo_handle_indexes != -1) {
                    GLDEBUG(glDeleteBuffers(1, &firstVBO.vbo_handle_indexes));
                }
                m_vboMemUsed -= firstVBO.size;
                m_vbosPool.erase(first_itr);
                fprintf(stderr, "VBO Swapping...\n");
            }
            
            m_currentVBO.vao_handle = -1;
            m_currentVBO.vbo_handle = -1;
            m_currentVBO.vbo_handle_indexes = -1;
            GLDEBUG(glGenBuffers(1, &m_currentVBO.vbo_handle));
#if GL_OES_vertex_array_object
            GLDEBUG(glGenVertexArraysOES(1, &m_currentVBO.vao_handle));
            GLDEBUG(glBindVertexArrayOES(m_currentVBO.vao_handle));
#endif

            GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_currentVBO.vbo_handle));
            GLDEBUG(glBufferData(GL_ARRAY_BUFFER, size, data, static_vbo ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
            m_memoryTransferredThisFrame += size;
            m_vboMemUsed += size;
            configureAttribs(enable_vertex, enable_normal, enable_tangent, enable_uva, enable_uvb, enable_bone_indexes, enable_bone_weights);
            
            m_currentVBO.size = size;
            m_currentVBO.data = data;
            
            m_vbosActive[data] = m_currentVBO;
        }
    }
}

void KRMeshManager::configureAttribs(bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb, bool enable_bone_indexes, bool enable_bone_weights)
{
    __int32_t attributes = 0;
    
    if(enable_vertex) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_VERTEX);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_VERTEX));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_VERTEX));
    }

    if(enable_normal) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_NORMAL);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_NORMAL));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_NORMAL));
    }

    if(enable_tangent) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_TANGENT);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TANGENT));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TANGENT));
    }

    if(enable_uva) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVA));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVA));
    }

    if(enable_uvb) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_TEXUVB);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVB));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVB));
    }
    
    if(enable_bone_indexes) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_BONEINDEXES);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEINDEXES));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEINDEXES));
    }
    
    if(enable_bone_weights) {
        attributes |= (1 << KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS);
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS));
    }
    
    GLsizei data_size = (GLsizei)KRMesh::VertexSizeForAttributes(attributes);
    
    if(enable_vertex) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_VERTEX, attributes))));
    }
    if(enable_normal) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_NORMAL, attributes))));
    }
    if(enable_tangent) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TANGENT, attributes))));
    }
    if(enable_uva) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVA, attributes))));
    }
    if(enable_uvb) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVB, 2, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVB, attributes))));
    }
    if(enable_bone_indexes ) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_BONEINDEXES, 4, GL_UNSIGNED_BYTE, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_BONEINDEXES, attributes))));
    }
    if(enable_bone_weights) {
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS, 4, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS, attributes))));
    }
}

long KRMeshManager::getMemUsed()
{
    return m_vboMemUsed;
}

long KRMeshManager::getMemActive()
{
    long mem_active = 0;
    for(std::map<GLvoid *, vbo_info_type>::iterator itr = m_vbosActive.begin(); itr != m_vbosActive.end(); itr++) {
        mem_active += (*itr).second.size;
    }
    return mem_active;
}

void KRMeshManager::rotateBuffers(bool new_frame)
{
    m_vbosPool.insert(m_vbosActive.begin(), m_vbosActive.end());
    m_vbosActive.clear();
    if(m_currentVBO.data != NULL) {
        // Ensure that the currently active VBO does not get flushed to free memory
        m_vbosPool.erase(m_currentVBO.data);
        m_vbosActive[m_currentVBO.data] = m_currentVBO;
    }

}

KRMeshManager::VolumetricLightingVertexData *KRMeshManager::getVolumetricLightingVertexes()
{
    if(m_volumetricLightingVertexData == NULL) {
        m_volumetricLightingVertexData = (VolumetricLightingVertexData *)malloc(sizeof(VolumetricLightingVertexData) * KRENGINE_MAX_VOLUMETRIC_PLANES * 6);
        int iVertex=0;
        for(int iPlane=0; iPlane < KRENGINE_MAX_VOLUMETRIC_PLANES; iPlane++) {
            m_volumetricLightingVertexData[iVertex].vertex.x = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            
            m_volumetricLightingVertexData[iVertex].vertex.x = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            
            m_volumetricLightingVertexData[iVertex].vertex.x = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            
            m_volumetricLightingVertexData[iVertex].vertex.x = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            
            m_volumetricLightingVertexData[iVertex].vertex.x = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = -1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            

            m_volumetricLightingVertexData[iVertex].vertex.x = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.y = 1.0f;
            m_volumetricLightingVertexData[iVertex].vertex.z = iPlane;
            iVertex++;
            
//            -1.0f, -1.0f,
//            1.0f, -1.0f,
//            -1.0f,  1.0f,
//            1.0f,  1.0f,
        }
    }
    return m_volumetricLightingVertexData;
}

KRMeshManager::RandomParticleVertexData *KRMeshManager::getRandomParticles()
{
    if(m_randomParticleVertexData == NULL) {
        m_randomParticleVertexData = (RandomParticleVertexData *)malloc(sizeof(RandomParticleVertexData) * KRENGINE_MAX_RANDOM_PARTICLES * 3);
        
        // Generate vertices for randomly placed equilateral triangles with a side length of 1 and an origin point centered so that an inscribed circle can be efficiently rendered without wasting fill
        
        float equilateral_triangle_height = sqrt(3.0f) / 2.0f;
        float inscribed_circle_radius = 1.0f / (2.0f * sqrt(3.0f));
        
        int iVertex=0;
        for(int iParticle=0; iParticle < KRENGINE_MAX_RANDOM_PARTICLES; iParticle++) {
            m_randomParticleVertexData[iVertex].vertex.x = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            m_randomParticleVertexData[iVertex].vertex.y = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            m_randomParticleVertexData[iVertex].vertex.z = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            m_randomParticleVertexData[iVertex].uva.u = -0.5f;
            m_randomParticleVertexData[iVertex].uva.v = -inscribed_circle_radius;
            iVertex++;
            
            m_randomParticleVertexData[iVertex].vertex.x = m_randomParticleVertexData[iVertex-1].vertex.x;
            m_randomParticleVertexData[iVertex].vertex.y = m_randomParticleVertexData[iVertex-1].vertex.y;
            m_randomParticleVertexData[iVertex].vertex.z = m_randomParticleVertexData[iVertex-1].vertex.z;
            m_randomParticleVertexData[iVertex].uva.u = 0.5f;
            m_randomParticleVertexData[iVertex].uva.v = -inscribed_circle_radius;
            iVertex++;
            
            m_randomParticleVertexData[iVertex].vertex.x = m_randomParticleVertexData[iVertex-1].vertex.x;
            m_randomParticleVertexData[iVertex].vertex.y = m_randomParticleVertexData[iVertex-1].vertex.y;
            m_randomParticleVertexData[iVertex].vertex.z = m_randomParticleVertexData[iVertex-1].vertex.z;
            m_randomParticleVertexData[iVertex].uva.u = 0.0f;
            m_randomParticleVertexData[iVertex].uva.v = -inscribed_circle_radius + equilateral_triangle_height;
            iVertex++;
        }
    }
    return m_randomParticleVertexData;
}

void KRMeshManager::startFrame(float deltaTime)
{
    m_memoryTransferredThisFrame = 0;
}

long KRMeshManager::getMemoryTransferedThisFrame()
{
    return m_memoryTransferredThisFrame;
}


int KRMeshManager::getActiveVBOCount()
{
    return m_vbosActive.size();
}

int KRMeshManager::getPoolVBOCount()
{
    return m_vbosPool.size();
}
