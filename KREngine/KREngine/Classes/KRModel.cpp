//
//  KRModel.cpp
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
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>


#include "KRModel.h"

#include "KRVector3.h"
#import "KRShader.h"
#import "KRShaderManager.h"
#import "KRContext.h"


KRModel::KRModel(KRContext &context, std::string name) : KRResource(context, name)  {
    m_hasTransparency = false;
    m_materials.clear();
    m_uniqueMaterials.clear();
    m_pData = new KRDataBlock();
    setName(name);
}

KRModel::KRModel(KRContext &context, std::string name, KRDataBlock *data) : KRResource(context, name) {
    m_hasTransparency = false;
    m_materials.clear();
    m_uniqueMaterials.clear();
    m_pData = new KRDataBlock();
    setName(name);
    
    loadPack(data);
}

void KRModel::setName(const std::string name) {
    m_lodCoverage = 100;
    m_lodBaseName = name;
    
    size_t last_underscore_pos = name.find_last_of('_');
    if(last_underscore_pos != std::string::npos) {
        // Found an underscore
        std::string suffix = name.substr(last_underscore_pos + 1);
        if(suffix.find_first_of("lod") == 0) {
            std::string lod_level_string = suffix.substr(3);
            char *end = NULL;
            int c = (int)strtol(lod_level_string.c_str(), &end, 10);
            if(c >= 0 && c <= 100 && *end == '\0') {
                m_lodCoverage = c;
                m_lodBaseName = name.substr(0, last_underscore_pos);
            }
        }
    }

}

KRModel::~KRModel() {
    clearData();
    if(m_pData) delete m_pData;
}

std::string KRModel::getExtension() {
    return "krobject";
}

bool KRModel::save(const std::string& path) {
    clearBuffers();
    return m_pData->save(path);
}

void KRModel::loadPack(KRDataBlock *data) {
    clearData();
    delete m_pData;
    m_pData = data;
    updateAttributeOffsets();
    pack_header *pHeader = getHeader();
    m_minPoint = KRVector3(pHeader->minx, pHeader->miny, pHeader->minz);
    m_maxPoint = KRVector3(pHeader->maxx, pHeader->maxy, pHeader->maxz);
}

#if TARGET_OS_IPHONE

void KRModel::render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, const KRMat4 &matModel, KRTexture *pLightMap, KRNode::RenderPass renderPass, const std::vector<KRBone *> &bones) {
    
    //fprintf(stderr, "Rendering model: %s\n", m_name.c_str());
    if(renderPass != KRNode::RENDER_PASS_ADDITIVE_PARTICLES && renderPass != KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
    
        if(m_materials.size() == 0) {
            vector<KRModel::Submesh *> submeshes = getSubmeshes();
            
            for(std::vector<KRModel::Submesh *>::iterator itr = submeshes.begin(); itr != submeshes.end(); itr++) {
                const char *szMaterialName = (*itr)->szMaterialName;
                KRMaterial *pMaterial = getContext().getMaterialManager()->getMaterial(szMaterialName);
                m_materials.push_back(pMaterial);
                if(pMaterial) {
                    m_uniqueMaterials.insert(pMaterial);
                } else {
                    fprintf(stderr, "Missing material: %s\n", szMaterialName);
                }
            }
            
            m_hasTransparency = false;
            for(std::set<KRMaterial *>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
                if((*mat_itr)->isTransparent()) {
                    m_hasTransparency = true;
                    break;
                }
            }
        }
        
        KRMaterial *pPrevBoundMaterial = NULL;
        char szPrevShaderKey[256];
        szPrevShaderKey[0] = '\0';
        int cSubmeshes = getSubmeshes().size();
        if(renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
            for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                KRMaterial *pMaterial = m_materials[iSubmesh];
                
                if(pMaterial != NULL) {

                    if(!pMaterial->isTransparent()) {
                        // Exclude transparent and semi-transparent meshes from shadow maps
                        renderSubmesh(iSubmesh);
                    }
                }
                
            }
        } else {
            // Apply submeshes in per-material batches to reduce number of state changes
            for(std::set<KRMaterial *>::iterator mat_itr = m_uniqueMaterials.begin(); mat_itr != m_uniqueMaterials.end(); mat_itr++) {
                for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                    KRMaterial *pMaterial = m_materials[iSubmesh];
                    
                    if(pMaterial != NULL && pMaterial == (*mat_itr)) {
                        if((!pMaterial->isTransparent() && renderPass != KRNode::RENDER_PASS_FORWARD_TRANSPARENT) || (pMaterial->isTransparent() && renderPass == KRNode::RENDER_PASS_FORWARD_TRANSPARENT)) {
                            if(pMaterial->bind(&pPrevBoundMaterial, szPrevShaderKey, pCamera, lights, bones, viewport, matModel, pLightMap, renderPass)) {
                            
                                switch(pMaterial->getAlphaMode()) {
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE: // Non-transparent materials
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_TEST: // Alpha in diffuse texture is interpreted as punch-through when < 0.5
                                        renderSubmesh(iSubmesh);
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE: // Blended alpha with backface culling
                                        renderSubmesh(iSubmesh);
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE: // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                                        // Render back faces first
                                        GLDEBUG(glCullFace(GL_BACK));
                                        renderSubmesh(iSubmesh);
                                        
                                        // Render front faces second
                                        GLDEBUG(glCullFace(GL_BACK));
                                        renderSubmesh(iSubmesh);
                                        break;
                                }
                            }
                            
                           
                        }
                    }
                }
            }
        }
    }
}

#endif

GLfloat KRModel::getMaxDimension() {
    GLfloat m = 0.0;
    if(m_maxPoint.x - m_minPoint.x > m) m = m_maxPoint.x - m_minPoint.x;
    if(m_maxPoint.y - m_minPoint.y > m) m = m_maxPoint.y - m_minPoint.y;
    if(m_maxPoint.z - m_minPoint.z > m) m = m_maxPoint.z - m_minPoint.z;
    return m;
}

bool KRModel::hasTransparency() {
    return m_hasTransparency;
}


vector<KRModel::Submesh *> KRModel::getSubmeshes() {
    if(m_submeshes.size() == 0) {
        pack_header *pHeader = getHeader();
        pack_material *pPackMaterials = (pack_material *)(pHeader+1);
        m_submeshes.clear();
        for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
            pack_material *pPackMaterial = pPackMaterials + iMaterial;
            
            Submesh *pSubmesh = new Submesh();
            pSubmesh->start_vertex = pPackMaterial->start_vertex;
            pSubmesh->vertex_count = pPackMaterial->vertex_count;
            
            strncpy(pSubmesh->szMaterialName, pPackMaterial->szName, KRENGINE_MAX_NAME_LENGTH);
            pSubmesh->szMaterialName[KRENGINE_MAX_NAME_LENGTH-1] = '\0';
            //fprintf(stderr, "Submesh material: \"%s\"\n", pSubmesh->szMaterialName);
            m_submeshes.push_back(pSubmesh);
        }
    }
    return m_submeshes;
}

void KRModel::renderSubmesh(int iSubmesh) {
    unsigned char *pVertexData = getVertexData();
    
    pack_header *pHeader = getHeader();
    int cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
    
    vector<KRModel::Submesh *> submeshes = getSubmeshes();
    Submesh *pSubmesh = submeshes[iSubmesh];
    
    int iVertex = pSubmesh->start_vertex;
    int iBuffer = iVertex / MAX_VBO_SIZE;
    iVertex = iVertex % MAX_VBO_SIZE;
    int cVertexes = pSubmesh->vertex_count;
    while(cVertexes > 0) {
        GLsizei cBufferVertexes = iBuffer < cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
        int vertex_size = m_vertex_size;
        
        void *vbo_end = (unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size + vertex_size * cBufferVertexes;
        void *buffer_end = m_pData->getEnd();
        assert(vbo_end <= buffer_end);
        assert(cBufferVertexes <= 65535);
        
        
        m_pContext->getModelManager()->bindVBO((unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size, vertex_size * cBufferVertexes, has_vertex_attribute(KRENGINE_ATTRIB_VERTEX), has_vertex_attribute(KRENGINE_ATTRIB_NORMAL), has_vertex_attribute(KRENGINE_ATTRIB_TANGENT), has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA), has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB), has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES),
                                               has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS));
        
        
        if(iVertex + cVertexes >= MAX_VBO_SIZE) {
            assert(iVertex + (MAX_VBO_SIZE  - iVertex) <= cBufferVertexes);
            switch (getModelFormat()) {
                case KRENGINE_MODEL_FORMAT_TRIANGLES:
                    GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, (MAX_VBO_SIZE  - iVertex)));
                    break;
                case KRENGINE_MODEL_FORMAT_STRIP:
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, iVertex, (MAX_VBO_SIZE  - iVertex)));
                    break;
                default:
                    break;
            }
            
            cVertexes -= (MAX_VBO_SIZE - iVertex);
            iVertex = 0;
            iBuffer++;
        } else {
            assert(iVertex + cVertexes <= cBufferVertexes);
            
            switch (getModelFormat()) {
                case KRENGINE_MODEL_FORMAT_TRIANGLES:
                    GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, cVertexes));
                    break;
                case KRENGINE_MODEL_FORMAT_STRIP:
                    GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, iVertex, cVertexes));
                    break;
                default:
                    break;
            }
            
            cVertexes = 0;
        }
        
    }
}

void KRModel::LoadData(std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents,  std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names, std::vector<std::string> bone_names, std::vector<std::vector<int> > bone_indexes, std::vector<std::vector<float> > bone_weights, KRModel::model_format_t model_format) {
    
    clearData();
    
    bool calculate_normals = true;
    bool calculate_tangents = true;
    
    
    __int32_t vertex_attrib_flags = 0;
    if(vertices.size()) {
        vertex_attrib_flags |= (1 << KRENGINE_ATTRIB_VERTEX);
    }
    if(normals.size() || calculate_normals) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_NORMAL);
    }
    if(tangents.size() || calculate_tangents) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TANGENT);
    }
    if(uva.size()) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVA);
    }
    if(uvb.size()) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVB);
    }
    if(bone_names.size()) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_BONEINDEXES) + (1 << KRENGINE_ATTRIB_BONEWEIGHTS);
    }
    size_t vertex_size = VertexSizeForAttributes(vertex_attrib_flags);
    
    size_t submesh_count = submesh_lengths.size();
    size_t vertex_count = vertices.size();
    size_t bone_count = bone_names.size();
    size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(pack_bone) * bone_count + vertex_size * vertex_count;
    m_pData->expand(new_file_size);
    
    pack_header *pHeader = getHeader();
    memset(pHeader, 0, sizeof(pack_header));
    pHeader->vertex_attrib_flags = vertex_attrib_flags;
    pHeader->submesh_count = (__int32_t)submesh_count;
    pHeader->vertex_count = (__int32_t)vertex_count;
    pHeader->bone_count = (__int32_t)bone_count;
    pHeader->model_format = model_format;
    strcpy(pHeader->szTag, "KROBJPACK1.1   ");
    updateAttributeOffsets();
    
    pack_material *pPackMaterials = (pack_material *)(pHeader+1);
    
    for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
        pack_material *pPackMaterial = pPackMaterials + iMaterial;
        pPackMaterial->start_vertex = submesh_starts[iMaterial];
        pPackMaterial->vertex_count = submesh_lengths[iMaterial];
        memset(pPackMaterial->szName, 0, KRENGINE_MAX_NAME_LENGTH);
        strncpy(pPackMaterial->szName, material_names[iMaterial].c_str(), KRENGINE_MAX_NAME_LENGTH);
    }
    
    for(int bone_index=0; bone_index < bone_count; bone_index++) {
        pack_bone *bone = getBone(bone_index);
        memset(bone->szName, 0, KRENGINE_MAX_NAME_LENGTH);
        strncpy(bone->szName, bone_names[bone_index].c_str(), KRENGINE_MAX_NAME_LENGTH);
    }
    
    bool bFirstVertex = true;
    
//    VertexData *pVertexData = (VertexData *)(pPackMaterials + pHeader->submesh_count);
//    VertexData *pVertex = pVertexData;
    memset(getVertexData(), 0, m_vertex_size * vertices.size());
    for(int iVertex=0; iVertex < vertices.size(); iVertex++) {
        KRVector3 source_vertex = vertices[iVertex];
        setVertexPosition(iVertex, source_vertex);
        if(bone_names.size()) {
            for(int bone_weight_index=0; bone_weight_index<KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; bone_weight_index++) {
                setBoneIndex(iVertex, bone_weight_index, bone_indexes[iVertex][bone_weight_index]);
                setBoneWeight(iVertex, bone_weight_index, bone_weights[iVertex][bone_weight_index]);
            }
        }
        if(bFirstVertex) {
            bFirstVertex = false;
            m_minPoint = source_vertex;
            m_maxPoint = source_vertex;
        } else {
            if(source_vertex.x < m_minPoint.x) m_minPoint.x = source_vertex.x;
            if(source_vertex.y < m_minPoint.y) m_minPoint.y = source_vertex.y;
            if(source_vertex.z < m_minPoint.z) m_minPoint.z = source_vertex.z;
            if(source_vertex.x > m_maxPoint.x) m_maxPoint.x = source_vertex.x;
            if(source_vertex.y > m_maxPoint.y) m_maxPoint.y = source_vertex.y;
            if(source_vertex.z > m_maxPoint.z) m_maxPoint.z = source_vertex.z;
        }
        if(uva.size() > iVertex) {
            setVertexUVA(iVertex, uva[iVertex]);
        }
        if(uvb.size() > iVertex) {
            setVertexUVB(iVertex, uvb[iVertex]);
        }
        if(normals.size() > iVertex) {
            setVertexNormal(iVertex, normals[iVertex]);
        }
        if(tangents.size() > iVertex) {
            setVertexTangent(iVertex, tangents[iVertex]);
        }
    }
    
    pHeader->minx = m_minPoint.x;
    pHeader->miny = m_minPoint.y;
    pHeader->minz = m_minPoint.z;
    pHeader->maxx = m_maxPoint.x;
    pHeader->maxy = m_maxPoint.y;
    pHeader->maxz = m_maxPoint.z;
    
    
    // Calculate missing surface normals and tangents
    //cout << "  Calculate surface normals and tangents\n";
    
    for(int iVertex=0; iVertex < vertices.size(); iVertex+= 3) {
        KRVector3 p1 = getVertexPosition(iVertex);
        KRVector3 p2 = getVertexPosition(iVertex+1);
        KRVector3 p3 = getVertexPosition(iVertex+2);
        KRVector3 v1 = p2 - p1;
        KRVector3 v2 = p3 - p1;
        
        
        // -- Calculate normal if missing --
        if(calculate_normals) {
            KRVector3 first_normal = getVertexNormal(iVertex);
            if(first_normal.x == 0.0f && first_normal.y == 0.0f && first_normal.z == 0.0f) {
                // Note - We don't take into consideration smoothing groups or smoothing angles when generating normals; all generated normals represent flat shaded polygons
                KRVector3 normal = KRVector3::Cross(v1, v2);
                
                normal.normalize();
                setVertexNormal(iVertex, normal);
                setVertexNormal(iVertex+1, normal);
                setVertexNormal(iVertex+2, normal);
            }
        }
        
        // -- Calculate tangent vector for normal mapping --
        if(calculate_tangents) {
            KRVector3 first_tangent = getVertexTangent(iVertex);
            if(first_tangent.x == 0.0f && first_tangent.y == 0.0f && first_tangent.z == 0.0f) {

                KRVector2 uv0 = getVertexUVA(iVertex);
                KRVector2 uv1 = getVertexUVA(iVertex + 1);
                KRVector2 uv2 = getVertexUVA(iVertex + 2);
                
                KRVector2 st1 = KRVector2(uv1.x - uv0.x, uv1.y - uv0.y);
                KRVector2 st2 = KRVector2(uv2.x - uv0.x, uv2.y - uv0.y);
                double coef = 1/ (st1.x * st2.y - st2.x * st1.y);
                
                KRVector3 tangent(
                                  coef * ((v1.x * st2.y)  + (v2.x * -st1.y)),
                                  coef * ((v1.y * st2.y)  + (v2.y * -st1.y)),
                                  coef * ((v1.z * st2.y)  + (v2.z * -st1.y))
                                  );
                
                tangent.normalize();
                setVertexTangent(iVertex, tangent);
                setVertexTangent(iVertex+1, tangent);
                setVertexTangent(iVertex+2, tangent);
            }
        }
    }
    
    optimize();
}

KRVector3 KRModel::getMinPoint() const {
    return m_minPoint;
}

KRVector3 KRModel::getMaxPoint() const {
    return m_maxPoint;
}

void KRModel::clearData() {
    m_pData->unload();
}

void KRModel::clearBuffers() {
    m_submeshes.clear();
}

int KRModel::getLODCoverage() const {
    return m_lodCoverage;
}

std::string KRModel::getLODBaseName() const {
    return m_lodBaseName;
}

// Predicate used with std::sort to sort by highest detail model first, decending to lowest detail LOD model
bool KRModel::lod_sort_predicate(const KRModel *m1, const KRModel *m2)
{
    return m1->m_lodCoverage > m2->m_lodCoverage;
}

bool KRModel::has_vertex_attribute(vertex_attrib_t attribute_type) const
{
    return (getHeader()->vertex_attrib_flags & (1 << attribute_type)) != 0;
}

KRModel::pack_header *KRModel::getHeader() const
{
    return (pack_header *)m_pData->getStart();
}

KRModel::pack_bone *KRModel::getBone(int index)
{
    pack_header *header = getHeader();
    return (pack_bone *)((unsigned char *)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * header->submesh_count + sizeof(pack_bone) * index);
}

unsigned char *KRModel::getVertexData() const {
    pack_header *pHeader = getHeader();
    return ((unsigned char *)m_pData->getStart()) + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count;
}

KRModel::pack_material *KRModel::getSubmesh(int mesh_index) const
{
    return (pack_material *)((unsigned char *)m_pData->getStart() + sizeof(pack_header)) + mesh_index;
}

unsigned char *KRModel::getVertexData(int index) const
{
    return getVertexData() + m_vertex_size * index;
}

int KRModel::getSubmeshCount() const
{
    pack_header *header = getHeader();
    return header->submesh_count;
}

int KRModel::getVertexCount(int submesh) const
{
    return getSubmesh(submesh)->vertex_count;
}

KRVector3 KRModel::getVertexPosition(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector3 KRModel::getVertexNormal(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector3 KRModel::getVertexTangent(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector2 KRModel::getVertexUVA(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA)) {
        return KRVector2((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]));
    } else {
        return KRVector2::Zero();
    }
}

KRVector2 KRModel::getVertexUVB(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB)) {
        return KRVector2((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]));
    } else {
        return KRVector2::Zero();
    }
}

void KRModel::setVertexPosition(int index, const KRVector3 &v)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
}

void KRModel::setVertexNormal(int index, const KRVector3 &v)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
}

void KRModel::setVertexTangent(int index, const KRVector3 & v)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]);
    vert[0] = v.x;
    vert[1] = v.y;
    vert[2] = v.z;
}

void KRModel::setVertexUVA(int index, const KRVector2 &v)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]);
    vert[0] = v.x;
    vert[1] = v.y;
}

void KRModel::setVertexUVB(int index, const KRVector2 &v)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]);
    vert[0] = v.x;
    vert[1] = v.y;
}


int KRModel::getBoneIndex(int index, int weight_index) const
{
    unsigned char *vert = (unsigned char *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
    return vert[weight_index];
}

void KRModel::setBoneIndex(int index, int weight_index, int bone_index)
{
    unsigned char *vert = (unsigned char *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
    vert[weight_index] = bone_index;
}

float KRModel::getBoneWeight(int index, int weight_index) const
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
    return vert[weight_index];
}

void KRModel::setBoneWeight(int index, int weight_index, float bone_weight)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
    vert[weight_index] = bone_weight;
}

size_t KRModel::VertexSizeForAttributes(__int32_t vertex_attrib_flags)
{
    size_t data_size = 0;
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_VERTEX)) {
        data_size += sizeof(float) * 3;
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_NORMAL)) {
        data_size += sizeof(float) * 3;
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_TANGENT)) {
        data_size += sizeof(float) * 3;
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_TEXUVA)) {
        data_size += sizeof(float) * 2;
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_TEXUVB)) {
        data_size += sizeof(float) * 2;
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_BONEINDEXES)) {
        data_size += 4; // 4 bytes
    }
    if(vertex_attrib_flags & (1 << KRENGINE_ATTRIB_BONEWEIGHTS)) {
        data_size += sizeof(float) * 4;
    }
    return data_size;
}

void KRModel::updateAttributeOffsets()
{
    pack_header *header = getHeader();
    int mask = 0;
    for(int i=0; i < KRENGINE_NUM_ATTRIBUTES; i++) {
        if(has_vertex_attribute((vertex_attrib_t)i)) {
            m_vertex_attribute_offset[i] = VertexSizeForAttributes(header->vertex_attrib_flags & mask);
        } else {
            m_vertex_attribute_offset[i] = -1;
        }
        mask = (mask << 1) | 1;
    }
    m_vertex_size = VertexSizeForAttributes(header->vertex_attrib_flags);
}

size_t KRModel::AttributeOffset(__int32_t vertex_attrib, __int32_t vertex_attrib_flags)
{
    int mask = 0;
    for(int i=0; i < vertex_attrib; i++) {
        if(vertex_attrib_flags & (1 << i)) {
            mask |= (1 << i);
        }
    }
    return VertexSizeForAttributes(mask);
}

int KRModel::getBoneCount()
{
    pack_header *header = getHeader();
    return header->bone_count;
}

char *KRModel::getBoneName(int bone_index)
{
    return getBone(bone_index)->szName;
}

void KRModel::optimize()
{
    // TODO - Add algorithm to convert model to indexed vertices, identying vertexes with identical attributes and optimizing order of trianges for best usage post-vertex-transform cache on GPU
}

KRModel::model_format_t KRModel::getModelFormat() const
{
    return (model_format_t)getHeader()->model_format;
}

bool KRModel::rayCast(const KRVector3 &line_v0, const KRVector3 &line_v1, const KRVector3 &tri_v0, const KRVector3 &tri_v1, const KRVector3 &tri_v2, const KRVector3 &tri_n0, const KRVector3 &tri_n1, const KRVector3 &tri_n2, KRHitInfo &hitinfo)
{
    // algorithm based on Dan Sunday's implementation at http://geomalgorithms.com/a06-_intersect-2.html
    const float SMALL_NUM = 0.00000001; // anything that avoids division overflow
    KRVector3   u, v, n;              // triangle vectors
    KRVector3   dir, w0, w;           // ray vectors
    float       r, a, b;              // params to calc ray-plane intersect
    
    // get triangle edge vectors and plane normal
    u = tri_v1 - tri_v0;
    v = tri_v2 - tri_v0;
    n = KRVector3::Cross(u, v); // cross product
    if (n == KRVector3::Zero())             // triangle is degenerate
        return false;                  // do not deal with this case
    
    dir = line_v1 - line_v0;              // ray direction vector
    w0 = line_v0 - tri_v0;
    a = -KRVector3::Dot(n, w0);
    b = KRVector3::Dot(n,dir);
    if (fabs(b) < SMALL_NUM) {     // ray is  parallel to triangle plane
        if (a == 0)                 
            return false; // ray lies in triangle plane
        else {
            return false; // ray disjoint from plane
        }
    }
    
    // get intersect point of ray with triangle plane
    r = a / b;
    if (r < 0.0)                    // ray goes away from triangle
        return false;                   // => no intersect
    // for a segment, also test if (r > 1.0) => no intersect
    
    
    KRVector3 hit_point = line_v0 + dir * r;            // intersect point of ray and plane
    
    // is hit_point inside triangle?
    float    uu, uv, vv, wu, wv, D;
    uu = KRVector3::Dot(u,u);
    uv = KRVector3::Dot(u,v);
    vv = KRVector3::Dot(v,v);
    w = hit_point - tri_v0;
    wu = KRVector3::Dot(w,u);
    wv = KRVector3::Dot(w,v);
    D = uv * uv - uu * vv;
    
    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;
    if (s < 0.0 || s > 1.0)         // hit_point is outside triangle
        return false;
    t = (uv * wu - uu * wv) / D;
    if (t < 0.0 || (s + t) > 1.0)  // hit_point is outside triangle
        return false;
    
    float new_hit_distance_sqr = (hit_point - line_v0).sqrMagnitude();
    float prev_hit_distance_sqr = (hitinfo.getPosition() - line_v0).sqrMagnitude();
    if(new_hit_distance_sqr < prev_hit_distance_sqr) {
        // Update the hitinfo object if this hit is closer than the prior hit
        
        // Interpolate between the three vertex normals, performing a 3-way lerp of tri_n0, tri_n1, and tri_n2
        KRVector3 distances = KRVector3::Normalize(KRVector3((tri_v0 - hit_point).magnitude(), (tri_v1 - hit_point).magnitude(), (tri_v2 - hit_point).magnitude()));
        KRVector3 normal = tri_n0 * (1.0 - distances[0]) + tri_n1 * (1.0 - distances[1]) + tri_n2 * (1.0 - distances[3]);
        
        hitinfo = KRHitInfo(hit_point, KRVector3());
    }
    
    return true; // hit_point is in triangle
}

bool KRModel::rayCast(const KRVector3 &line_v0, const KRVector3 &line_v1, int tri_index0, int tri_index1, int tri_index2, KRHitInfo &hitinfo) const
{
    return rayCast(line_v0, line_v1, getVertexPosition(tri_index0), getVertexPosition(tri_index1), getVertexPosition(tri_index2), getVertexNormal(tri_index0), getVertexNormal(tri_index1), getVertexNormal(tri_index2), hitinfo);
}

bool KRModel::rayCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo) const
{
    bool hit_found = false;
    for(int submesh_index=0; submesh_index < getSubmeshCount(); submesh_index++) {
        int vertex_count = getVertexCount(submesh_index);
        switch(getModelFormat()) {
            case KRENGINE_MODEL_FORMAT_TRIANGLES:
                for(int triangle_index=0; triangle_index < vertex_count / 3; triangle_index++) {
                    hit_found |= rayCast(v0, v1, getVertexPosition(triangle_index*3), getVertexPosition(triangle_index*3+1), getVertexPosition(triangle_index*3+2), getVertexNormal(triangle_index*3), getVertexNormal(triangle_index*3+1), getVertexNormal(triangle_index*3+2), hitinfo);
                }
                break;
            case KRENGINE_MODEL_FORMAT_STRIP:
                for(int triangle_index=0; triangle_index < vertex_count - 2; triangle_index++) {
                    hit_found |= rayCast(v0, v1, getVertexPosition(triangle_index), getVertexPosition(triangle_index+1), getVertexPosition(triangle_index+2), getVertexNormal(triangle_index), getVertexNormal(triangle_index+1), getVertexNormal(triangle_index+2), hitinfo);
                }
                break;
            default:
                break;
        }
    }
    return hit_found;
}

bool KRModel::lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo) const
{
    KRHitInfo new_hitinfo;
    if(rayCast(v0, v1, new_hitinfo)) {
        if((new_hitinfo.getPosition() - v0).sqrMagnitude() <= (v1 - v0).sqrMagnitude()) {
            // The hit was between v1 and v2
            hitinfo = new_hitinfo;
            return true;
        }
    }
    return false; // Either no hit, or the hit was beyond v1
}
