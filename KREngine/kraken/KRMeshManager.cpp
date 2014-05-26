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
#include "KRMeshQuad.h"
#include "KRMeshSphere.h"

KRMeshManager::KRMeshManager(KRContext &context) : KRContextObject(context) {
    m_currentVBO = NULL;
    m_vboMemUsed = 0;
    m_memoryTransferredThisFrame = 0;
    
    addModel(new KRMeshCube(context)); // FINDME - HACK!  This needs to be fixed, as it currently segfaults
    addModel(new KRMeshQuad(context)); // FINDME - HACK!  This needs to be fixed, as it currently segfaults
    addModel(new KRMeshSphere(context));
    m_draw_call_logging_enabled = false;
    m_draw_call_log_used = false;
    
    
    
    // ----  Initialize stock models ----
    
    static const GLfloat _KRENGINE_VBO_3D_CUBE_VERTEX_DATA[] = {
        1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0,
        1.0,-1.0, 1.0,
        -1.0,-1.0, 1.0,
        -1.0,-1.0,-1.0,
        -1.0, 1.0, 1.0,
        -1.0, 1.0,-1.0,
        1.0, 1.0, 1.0,
        1.0, 1.0,-1.0,
        1.0,-1.0, 1.0,
        1.0,-1.0,-1.0,
        -1.0,-1.0,-1.0,
        1.0, 1.0,-1.0,
        -1.0, 1.0,-1.0
    };
    
    KRENGINE_VBO_3D_CUBE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX);
    KRENGINE_VBO_3D_CUBE_VERTICES.expand(sizeof(GLfloat) * 3 * 14);
    KRENGINE_VBO_3D_CUBE_VERTICES.lock();
    memcpy(KRENGINE_VBO_3D_CUBE_VERTICES.getStart(), _KRENGINE_VBO_3D_CUBE_VERTEX_DATA, sizeof(GLfloat) * 3 * 14);
    KRENGINE_VBO_3D_CUBE_VERTICES.unlock();
    
    KRENGINE_VBO_DATA_3D_CUBE_VERTICES.init(this, KRENGINE_VBO_3D_CUBE_VERTICES, KRENGINE_VBO_3D_CUBE_INDEXES, KRENGINE_VBO_3D_CUBE_ATTRIBS, false, false);
    
    
    
    static const GLfloat _KRENGINE_VBO_2D_SQUARE_VERTEX_DATA[] = {
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    KRENGINE_VBO_2D_SQUARE_ATTRIBS = (1 << KRMesh::KRENGINE_ATTRIB_VERTEX) | (1 << KRMesh::KRENGINE_ATTRIB_TEXUVA);
    KRENGINE_VBO_2D_SQUARE_VERTICES.expand(sizeof(GLfloat) * 5 * 4);
    KRENGINE_VBO_2D_SQUARE_VERTICES.lock();
    memcpy(KRENGINE_VBO_2D_SQUARE_VERTICES.getStart(), _KRENGINE_VBO_2D_SQUARE_VERTEX_DATA, sizeof(GLfloat) * 5 * 4);
    KRENGINE_VBO_2D_SQUARE_VERTICES.unlock();
    
    KRENGINE_VBO_DATA_2D_SQUARE_VERTICES.init(this, KRENGINE_VBO_2D_SQUARE_VERTICES, KRENGINE_VBO_2D_SQUARE_INDEXES, KRENGINE_VBO_2D_SQUARE_ATTRIBS, false, false);
    
}

KRMeshManager::~KRMeshManager() {
    for(unordered_multimap<std::string, KRMesh *>::iterator itr = m_models.begin(); itr != m_models.end(); ++itr){
        delete (*itr).second;
    }
    m_models.empty();
}

KRMesh *KRMeshManager::loadModel(const char *szName, KRDataBlock *pData) {
    KRMesh *pModel = new KRMesh(*m_pContext, szName, pData);
    addModel(pModel);
    return pModel;
}

void KRMeshManager::addModel(KRMesh *model) {
    std::string lowerName = model->getLODBaseName();
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    m_models.insert(std::pair<std::string, KRMesh *>(lowerName, model));
}

std::vector<KRMesh *> KRMeshManager::getModel(const char *szName) {
    std::string lowerName = szName;
    std::transform(lowerName.begin(), lowerName.end(),
                   lowerName.begin(), ::tolower);
    
    
    std::vector<KRMesh *> matching_models;
    
    std::pair<unordered_multimap<std::string, KRMesh *>::iterator, unordered_multimap<std::string, KRMesh *>::iterator> range = m_models.equal_range(lowerName);
    for(unordered_multimap<std::string, KRMesh *>::iterator itr_match = range.first; itr_match != range.second; itr_match++) {
        matching_models.push_back(itr_match->second);
    }
    
    std::sort(matching_models.begin(), matching_models.end(), KRMesh::lod_sort_predicate);
    
    if(matching_models.size() == 0) {        
        KRContext::Log(KRContext::LOG_LEVEL_INFORMATION, "Model not found: %s", lowerName.c_str());
    }
    
    return matching_models;
}

unordered_multimap<std::string, KRMesh *> &KRMeshManager::getModels() {
    return m_models;
}

void KRMeshManager::unbindVBO() {
    if(m_currentVBO != NULL) {
        GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
        m_currentVBO = NULL;
    }
}

void KRMeshManager::releaseVBO(KRDataBlock &data)
{
    if(m_currentVBO) {
        if(m_currentVBO->m_data == &data) {
            unbindVBO();
        }
    }

    KRVBOData *vbo_to_release = NULL;
    if(m_vbosActive.find(&data) != m_vbosActive.end()) {
        KRContext::Log(KRContext::LOG_LEVEL_WARNING, "glFinish called due to releasing a VBO that is active in the current frame.");
        GLDEBUG(glFinish());
        
        // The VBO is active
        vbo_to_release = m_vbosActive[&data];
        m_vbosActive.erase(&data);
    } else {
        // The VBO is inactive
        vbo_to_release = m_vbosPool[&data];
        m_vbosPool.erase(&data);
    }
    
    if(vbo_to_release) {
        m_vboMemUsed -= vbo_to_release->getSize();
        
        vbo_to_release->unload();
        if(vbo_to_release->isTemporary()) {
            delete vbo_to_release;
        }
    }
}

void KRMeshManager::bindVBO(KRVBOData *vbo_data)
{
    bool vbo_changed = false;
    if(m_currentVBO == NULL) {
        vbo_changed = true;
    } else if(m_currentVBO->m_data != vbo_data->m_data) {
        vbo_changed = true;
    }
    
    bool used_vbo_data = false;
    
    if(vbo_changed) {
        
        if(m_vbosActive.find(vbo_data->m_data) != m_vbosActive.end()) {
            m_currentVBO = m_vbosActive[vbo_data->m_data];
            
            m_currentVBO->bind();
            
        } else if(m_vbosPool.find(vbo_data->m_data) != m_vbosPool.end()) {
            m_currentVBO = m_vbosPool[vbo_data->m_data];
            m_vbosPool.erase(vbo_data->m_data);
            m_vbosActive[vbo_data->m_data] = m_currentVBO;
            
            m_currentVBO->bind();
            
            
        } else {
            
            
            while(m_vbosPool.size() + m_vbosActive.size() + 1 >= KRContext::KRENGINE_MAX_VBO_HANDLES || m_vboMemUsed + vbo_data->getSize() >= KRContext::KRENGINE_MAX_VBO_MEM) {
                if(m_vbosPool.empty()) {
                    KRContext::Log(KRContext::LOG_LEVEL_WARNING, "flushBuffers due to VBO exhaustion...");
                    m_pContext->rotateBuffers(false);
                }
                unordered_map<KRDataBlock *, KRVBOData *>::iterator first_itr = m_vbosPool.begin();
                KRVBOData *firstVBO = first_itr->second;
                m_vbosPool.erase(first_itr);
                
                m_vboMemUsed -= firstVBO->getSize();
                firstVBO->unload();
                if(firstVBO->isTemporary()) {
                    delete firstVBO;
                }
                // fprintf(stderr, "VBO Swapping...\n");
            }
            
            used_vbo_data = true;
            m_currentVBO = vbo_data;
            
            m_currentVBO->load();
            m_memoryTransferredThisFrame += m_currentVBO->getSize();
            m_vboMemUsed += m_currentVBO->getSize();
            
            m_vbosActive[vbo_data->m_data] = m_currentVBO;
        }
    }
    
    if(!used_vbo_data && vbo_data->isTemporary()) {
        delete vbo_data;
    }
}

void KRMeshManager::bindVBO(KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo)
{
    KRVBOData *vbo_data = new KRVBOData(this, data, index_data, vertex_attrib_flags, static_vbo, true);
    bindVBO(vbo_data);
}

void KRMeshManager::configureAttribs(__int32_t attributes)
{
    GLsizei data_size = (GLsizei)KRMesh::VertexSizeForAttributes(attributes);
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_VERTEX_SHORT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_VERTEX));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 3, GL_SHORT, GL_TRUE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_VERTEX_SHORT, attributes))));
    } else if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_VERTEX)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_VERTEX));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_VERTEX, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_VERTEX));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_NORMAL_SHORT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_NORMAL));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_NORMAL, 3, GL_SHORT, GL_TRUE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_NORMAL_SHORT, attributes))));
    } else if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_NORMAL)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_NORMAL));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_NORMAL, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_NORMAL));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TANGENT_SHORT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TANGENT));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TANGENT, 3, GL_SHORT, GL_TRUE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TANGENT_SHORT, attributes))));
    } else if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TANGENT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TANGENT));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TANGENT, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TANGENT));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TEXUVA_SHORT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVA));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVA, 2, GL_SHORT, GL_TRUE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVA_SHORT, attributes))));
    } else if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TEXUVA)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVA));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVA, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVA));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TEXUVB_SHORT)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVB));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVB, 2, GL_SHORT, GL_TRUE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVB_SHORT, attributes))));
    } else if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_TEXUVB)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVB));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_TEXUVB, 2, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_TEXUVB, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_TEXUVB));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_BONEINDEXES)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEINDEXES));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_BONEINDEXES, 4, GL_UNSIGNED_BYTE, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_BONEINDEXES, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEINDEXES));
    }
    
    if(KRMesh::has_vertex_attribute(attributes, KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS)) {
        GLDEBUG(glEnableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS));
        GLDEBUG(glVertexAttribPointer(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS, 4, GL_FLOAT, GL_FALSE, data_size, BUFFER_OFFSET(KRMesh::AttributeOffset(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS, attributes))));
    } else {
        GLDEBUG(glDisableVertexAttribArray(KRMesh::KRENGINE_ATTRIB_BONEWEIGHTS));
    }
}

long KRMeshManager::getMemUsed()
{
    return m_vboMemUsed;
}

long KRMeshManager::getMemActive()
{
    long mem_active = 0;
    for(unordered_map<KRDataBlock *, KRVBOData *>::iterator itr = m_vbosActive.begin(); itr != m_vbosActive.end(); itr++) {
        mem_active += (*itr).second->getSize();
    }
    return mem_active;
}

void KRMeshManager::rotateBuffers(bool new_frame)
{
    m_vbosPool.insert(m_vbosActive.begin(), m_vbosActive.end());
    m_vbosActive.clear();
    if(m_currentVBO != NULL) {
        // Ensure that the currently active VBO does not get flushed to free memory
        m_vbosPool.erase(m_currentVBO->m_data);
        m_vbosActive[m_currentVBO->m_data] = m_currentVBO;
    }
}

KRDataBlock &KRMeshManager::getVolumetricLightingVertexes()
{
    if(m_volumetricLightingVertexData.getSize() == 0) {
        m_volumetricLightingVertexData.expand(sizeof(VolumetricLightingVertexData) * KRENGINE_MAX_VOLUMETRIC_PLANES * 6);
        m_volumetricLightingVertexData.lock();
        VolumetricLightingVertexData * vertex_data = (VolumetricLightingVertexData *)m_volumetricLightingVertexData.getStart();
        int iVertex=0;
        for(int iPlane=0; iPlane < KRENGINE_MAX_VOLUMETRIC_PLANES; iPlane++) {
            vertex_data[iVertex].vertex.x = -1.0f;
            vertex_data[iVertex].vertex.y = -1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = 1.0f;
            vertex_data[iVertex].vertex.y = -1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = -1.0f;
            vertex_data[iVertex].vertex.y = 1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = -1.0f;
            vertex_data[iVertex].vertex.y = 1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = 1.0f;
            vertex_data[iVertex].vertex.y = -1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = 1.0f;
            vertex_data[iVertex].vertex.y = 1.0f;
            vertex_data[iVertex].vertex.z = iPlane;
            iVertex++;

        }
        m_volumetricLightingVertexData.unlock();
    }
    return m_volumetricLightingVertexData;
}

KRDataBlock &KRMeshManager::getRandomParticles()
{
    if(m_randomParticleVertexData.getSize() == 0) {
        m_randomParticleVertexData.expand(sizeof(RandomParticleVertexData) * KRENGINE_MAX_RANDOM_PARTICLES * 3);
        m_randomParticleVertexData.lock();
        RandomParticleVertexData *vertex_data = (RandomParticleVertexData *)m_randomParticleVertexData.getStart();
        
        // Generate vertices for randomly placed equilateral triangles with a side length of 1 and an origin point centered so that an inscribed circle can be efficiently rendered without wasting fill
        
        float equilateral_triangle_height = sqrt(3.0f) * 0.5f;
        float inscribed_circle_radius = 1.0f / (2.0f * sqrt(3.0f));
        
        int iVertex=0;
        for(int iParticle=0; iParticle < KRENGINE_MAX_RANDOM_PARTICLES; iParticle++) {
            vertex_data[iVertex].vertex.x = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            vertex_data[iVertex].vertex.y = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            vertex_data[iVertex].vertex.z = (float)(arc4random() % 2000) / 1000.0f - 1000.0f;
            vertex_data[iVertex].uva.u = -0.5f;
            vertex_data[iVertex].uva.v = -inscribed_circle_radius;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = vertex_data[iVertex-1].vertex.x;
            vertex_data[iVertex].vertex.y = vertex_data[iVertex-1].vertex.y;
            vertex_data[iVertex].vertex.z = vertex_data[iVertex-1].vertex.z;
            vertex_data[iVertex].uva.u = 0.5f;
            vertex_data[iVertex].uva.v = -inscribed_circle_radius;
            iVertex++;
            
            vertex_data[iVertex].vertex.x = vertex_data[iVertex-1].vertex.x;
            vertex_data[iVertex].vertex.y = vertex_data[iVertex-1].vertex.y;
            vertex_data[iVertex].vertex.z = vertex_data[iVertex-1].vertex.z;
            vertex_data[iVertex].uva.u = 0.0f;
            vertex_data[iVertex].uva.v = -inscribed_circle_radius + equilateral_triangle_height;
            iVertex++;
        }
        m_randomParticleVertexData.unlock();
    }
    return m_randomParticleVertexData;
}

void KRMeshManager::startFrame(float deltaTime)
{
    m_memoryTransferredThisFrame = 0;
    if(m_draw_call_log_used) {
        // Only log draw calls on the next frame if the draw call log was used on last frame
        m_draw_call_log_used = false;
        m_draw_call_logging_enabled = true;
    }
    m_draw_calls.clear();
    
}

void KRMeshManager::endFrame(float deltaTime)
{

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

void KRMeshManager::log_draw_call(KRNode::RenderPass pass, const std::string &object_name, const std::string &material_name, int vertex_count)
{
    if(m_draw_call_logging_enabled) {
        draw_call_info info;
        info.pass = pass;
        strncpy(info.object_name, object_name.c_str(), 256);
        strncpy(info.material_name, material_name.c_str(), 256);
        info.vertex_count = vertex_count;
        m_draw_calls.push_back(info);
    }
}

std::vector<KRMeshManager::draw_call_info> KRMeshManager::getDrawCalls()
{
    m_draw_call_log_used = true;
    return m_draw_calls;
}

void KRMeshManager::doStreaming(long &memoryRemaining, long &memoryRemainingThisFrame)
{
    
}

KRMeshManager::KRVBOData::KRVBOData()
{
    m_manager = NULL;
    m_temp_vbo = false;
    m_static_vbo = false;
    m_data = NULL;
    m_index_data = NULL;
    m_vertex_attrib_flags = 0;
    m_vbo_handle = -1;
    m_vbo_handle_indexes = -1;
    m_vao_handle = -1;
    m_size = 0;
    
    m_last_frame_used = 0;
    m_last_frame_max_lod_coverage = 0.0f;
}

KRMeshManager::KRVBOData::KRVBOData(KRMeshManager *manager, KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo, bool temp_vbo)
{
    init(manager, data,index_data,vertex_attrib_flags, static_vbo, temp_vbo);
}

void KRMeshManager::KRVBOData::init(KRMeshManager *manager, KRDataBlock &data, KRDataBlock &index_data, int vertex_attrib_flags, bool static_vbo, bool temp_vbo)
{
    m_manager = manager;
    m_temp_vbo = temp_vbo;
    m_static_vbo = static_vbo;
    m_data = &data;
    m_index_data = &index_data;
    m_vertex_attrib_flags = vertex_attrib_flags;
    
    m_vbo_handle = -1;
    m_vbo_handle_indexes = -1;
    m_vao_handle = -1;
    
    m_size = m_data->getSize();
    if(m_index_data != NULL) {
        m_size += m_index_data->getSize();
    }
}

KRMeshManager::KRVBOData::~KRVBOData()
{
    
}



void KRMeshManager::KRVBOData::load()
{
    if(isLoaded()) {
        return;
    }
    m_vao_handle = -1;
    m_vbo_handle = -1;
    m_vbo_handle_indexes = -1;
    
    GLDEBUG(glGenBuffers(1, &m_vbo_handle));
    if(m_index_data->getSize() > 0) {
        GLDEBUG(glGenBuffers(1, &m_vbo_handle_indexes));
    }
    
    
    
#if GL_OES_vertex_array_object
    GLDEBUG(glGenVertexArraysOES(1, &m_vao_handle));
    GLDEBUG(glBindVertexArrayOES(m_vao_handle));
#endif
    
    GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_vbo_handle));
#if GL_OES_mapbuffer
    
    GLDEBUG(glBufferData(GL_ARRAY_BUFFER, m_data->getSize(), NULL, m_static_vbo ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
    GLDEBUG(void *map_ptr = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
    m_data->copy(map_ptr);
    GLDEBUG(glUnmapBufferOES(GL_ARRAY_BUFFER));
#else
    m_data->lock();
    GLDEBUG(glBufferData(GL_ARRAY_BUFFER, m_data->getSize(), m_data->getStart(), m_static_vbo ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
    m_data->unlock();
#endif
    
    configureAttribs(m_vertex_attrib_flags);
    
    if(m_index_data->getSize() == 0) {
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    } else {
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_handle_indexes));
        
#if GL_OES_mapbuffer
        
        GLDEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_data->getSize(), NULL, m_static_vbo ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
        GLDEBUG(void *map_ptr = glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES));
        m_index_data->copy(map_ptr);
        GLDEBUG(glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER));
#else
        m_index_data->lock();
        GLDEBUG(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_data->getSize(), m_index_data->getStart(), m_static_vbo ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW));
        m_index_data->unlock();
#endif
    }
}

void KRMeshManager::KRVBOData::unload()
{
#if GL_OES_vertex_array_object
    if(m_vao_handle != -1) {
        GLDEBUG(glDeleteVertexArraysOES(1, &m_vao_handle));
        m_vao_handle = -1;
    }
#endif
    if(m_vbo_handle != -1) {
        GLDEBUG(glDeleteBuffers(1, &m_vbo_handle));
        m_vbo_handle = -1;
    }
    
    if(m_vbo_handle_indexes != -1) {
        GLDEBUG(glDeleteBuffers(1, &m_vbo_handle_indexes));
        m_vbo_handle_indexes = -1;
    }
}

void KRMeshManager::KRVBOData::bind()
{
#if GL_OES_vertex_array_object
    GLDEBUG(glBindVertexArrayOES(m_vao_handle));
#else
    GLDEBUG(glBindBuffer(GL_ARRAY_BUFFER, m_vbo_handle));
    KRMeshManager::configureAttribs(m_vertex_attrib_flags);
    if(m_vbo_handle_indexes == -1) {
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    } else {
        GLDEBUG(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_handle_indexes));
    }
#endif
}

void KRMeshManager::KRVBOData::resetPoolExpiry(float lodCoverage)
{
    long current_frame = m_manager->getContext().getCurrentFrame();
    if(current_frame != m_last_frame_used) {
        m_last_frame_used = current_frame;
        m_last_frame_max_lod_coverage = 0.0f;
    }
    m_last_frame_max_lod_coverage = KRMAX(lodCoverage, m_last_frame_max_lod_coverage);
}