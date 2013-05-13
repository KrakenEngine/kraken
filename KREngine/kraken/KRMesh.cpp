//
//  KRMesh.cpp
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

#include "KRMesh.h"

#include "KRVector3.h"
#include "KRShader.h"
#include "KRShaderManager.h"
#include "KRContext.h"
#include "forsyth.h"


KRMesh::KRMesh(KRContext &context, std::string name) : KRResource(context, name)  {
    m_hasTransparency = false;
    m_materials.clear();
    m_uniqueMaterials.clear();
    m_pData = new KRDataBlock();
    setName(name);
}

KRMesh::KRMesh(KRContext &context, std::string name, KRDataBlock *data) : KRResource(context, name) {
    m_hasTransparency = false;
    m_materials.clear();
    m_uniqueMaterials.clear();
    m_pData = new KRDataBlock();
    setName(name);
    
    loadPack(data);
}

void KRMesh::setName(const std::string name) {
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

int KRMesh::GetLODCoverage(const std::string &name)
{
    int lod_coverage = 100;
    size_t last_underscore_pos = name.find_last_of('_');
    if(last_underscore_pos != std::string::npos) {
        // Found an underscore
        std::string suffix = name.substr(last_underscore_pos + 1);
        if(suffix.find_first_of("lod") == 0) {
            std::string lod_level_string = suffix.substr(3);
            char *end = NULL;
            int c = (int)strtol(lod_level_string.c_str(), &end, 10);
            if(c >= 0 && c <= 100 && *end == '\0') {
                lod_coverage = c;
                //m_lodBaseName = name.substr(0, last_underscore_pos);
            }
        }
    }
    return lod_coverage;
}



KRMesh::~KRMesh() {
    clearData();
    if(m_pData) delete m_pData;
}

std::string KRMesh::getExtension() {
    return "krmesh";
}

bool KRMesh::save(const std::string& path) {
    clearBuffers();
    return m_pData->save(path);
}

bool KRMesh::save(KRDataBlock &data) {
    clearBuffers();
    data.append(*m_pData);
    return true;
}

void KRMesh::loadPack(KRDataBlock *data) {
    clearData();
    delete m_pData;
    m_pData = data;
    updateAttributeOffsets();
    pack_header *pHeader = getHeader();
    m_minPoint = KRVector3(pHeader->minx, pHeader->miny, pHeader->minz);
    m_maxPoint = KRVector3(pHeader->maxx, pHeader->maxy, pHeader->maxz);
}

void KRMesh::render(const std::string &object_name, KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, const KRMat4 &matModel, KRTexture *pLightMap, KRNode::RenderPass renderPass, const std::vector<KRBone *> &bones) {
    
    //fprintf(stderr, "Rendering model: %s\n", m_name.c_str());
    if(renderPass != KRNode::RENDER_PASS_ADDITIVE_PARTICLES && renderPass != KRNode::RENDER_PASS_PARTICLE_OCCLUSION && renderPass != KRNode::RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE) {
        getSubmeshes();
        if(m_materials.size() == 0) {
            
            for(std::vector<KRMesh::Submesh *>::iterator itr = m_submeshes.begin(); itr != m_submeshes.end(); itr++) {
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
        int cSubmeshes = m_submeshes.size();
        if(renderPass == KRNode::RENDER_PASS_SHADOWMAP) {
            for(int iSubmesh=0; iSubmesh<cSubmeshes; iSubmesh++) {
                KRMaterial *pMaterial = m_materials[iSubmesh];
                
                if(pMaterial != NULL) {

                    if(!pMaterial->isTransparent()) {
                        // Exclude transparent and semi-transparent meshes from shadow maps
                        renderSubmesh(iSubmesh, renderPass, object_name, pMaterial->getName());
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
                            if(pMaterial->bind(&pPrevBoundMaterial, szPrevShaderKey, pCamera, point_lights, directional_lights, spot_lights, bones, viewport, matModel, pLightMap, renderPass)) {
                            
                                switch(pMaterial->getAlphaMode()) {
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_OPAQUE: // Non-transparent materials
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_TEST: // Alpha in diffuse texture is interpreted as punch-through when < 0.5
                                        renderSubmesh(iSubmesh, renderPass, object_name, pMaterial->getName());
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE: // Blended alpha with backface culling
                                        renderSubmesh(iSubmesh, renderPass, object_name, pMaterial->getName());
                                        break;
                                    case KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE: // Blended alpha rendered in two passes.  First pass renders backfaces; second pass renders frontfaces.
                                        // Render back faces first
                                        GLDEBUG(glCullFace(GL_FRONT));
                                        renderSubmesh(iSubmesh, renderPass, object_name, pMaterial->getName());

                                        // Render front faces second
                                        GLDEBUG(glCullFace(GL_BACK));
                                        renderSubmesh(iSubmesh, renderPass, object_name, pMaterial->getName());
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

GLfloat KRMesh::getMaxDimension() {
    GLfloat m = 0.0;
    if(m_maxPoint.x - m_minPoint.x > m) m = m_maxPoint.x - m_minPoint.x;
    if(m_maxPoint.y - m_minPoint.y > m) m = m_maxPoint.y - m_minPoint.y;
    if(m_maxPoint.z - m_minPoint.z > m) m = m_maxPoint.z - m_minPoint.z;
    return m;
}

bool KRMesh::hasTransparency() {
    return m_hasTransparency;
}


void KRMesh::getSubmeshes() {
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
}

void KRMesh::renderSubmesh(int iSubmesh, KRNode::RenderPass renderPass, const std::string &object_name, const std::string &material_name) {
    getSubmeshes();
    Submesh *pSubmesh = m_submeshes[iSubmesh];
    int cVertexes = pSubmesh->vertex_count;
    // fprintf(stderr, "start - object: %s material: %s vertices: %i\n", object_name.c_str(), material_name.c_str(), cVertexes);
    unsigned char *pVertexData = getVertexData();
    pack_header *pHeader = getHeader();

    
    
    if(getModelFormat() == KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES) {
        
        
        
        
        __uint16_t *index_data = getIndexData();
        int index_group = getSubmesh(iSubmesh)->index_group;
        int index_group_offset = getSubmesh(iSubmesh)->index_group_offset;
        while(cVertexes > 0) {
            
            int start_index_offset, start_vertex_offset, index_count, vertex_count;
            getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
            
            m_pContext->getModelManager()->bindVBO((unsigned char *)pVertexData + start_vertex_offset * m_vertex_size, vertex_count * m_vertex_size, index_data + start_index_offset, index_count * 2, pHeader->vertex_attrib_flags, true);
            
            int vertex_draw_count = cVertexes;
            if(vertex_draw_count > index_count - index_group_offset) vertex_draw_count = index_count - index_group_offset;
            
            glDrawElements(GL_TRIANGLES, vertex_draw_count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(index_group_offset * 2));
            m_pContext->getModelManager()->log_draw_call(renderPass, object_name, material_name, vertex_draw_count);
            cVertexes -= vertex_draw_count;
            index_group_offset = 0;
        }
        
    } else {
        int cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
        int iVertex = pSubmesh->start_vertex;
        int iBuffer = iVertex / MAX_VBO_SIZE;
        iVertex = iVertex % MAX_VBO_SIZE;
        while(cVertexes > 0) {
            GLsizei cBufferVertexes = iBuffer < cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
            int vertex_size = m_vertex_size;
            
            void *vbo_end = (unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size + vertex_size * cBufferVertexes;
            void *buffer_end = m_pData->getEnd();
            assert(vbo_end <= buffer_end);
            assert(cBufferVertexes <= 65535);
            
            
            m_pContext->getModelManager()->bindVBO((unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size, vertex_size * cBufferVertexes, NULL, 0, pHeader->vertex_attrib_flags, true);
            
            
            if(iVertex + cVertexes >= MAX_VBO_SIZE) {
                assert(iVertex + (MAX_VBO_SIZE  - iVertex) <= cBufferVertexes);
                switch (getModelFormat()) {
                    case KRENGINE_MODEL_FORMAT_TRIANGLES:
                        GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, (MAX_VBO_SIZE  - iVertex)));
                        break;
                    case KRENGINE_MODEL_FORMAT_STRIP:
                        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, iVertex, (MAX_VBO_SIZE  - iVertex)));
                        break;
                    case KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
                        GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, (MAX_VBO_SIZE  - iVertex)));
                        break;
                    case KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
                        GLDEBUG(glDrawArrays(GL_TRIANGLE_STRIP, iVertex, (MAX_VBO_SIZE  - iVertex)));
                        break;
                    default:
                        break;
                }
                m_pContext->getModelManager()->log_draw_call(renderPass, object_name, material_name, (MAX_VBO_SIZE  - iVertex));
                
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
                m_pContext->getModelManager()->log_draw_call(renderPass, object_name, material_name, cVertexes);
                
                cVertexes = 0;
            }
            
        }
    }
    // fprintf(stderr, "end object\n");
    
}

void KRMesh::LoadData(std::vector<__uint16_t> vertex_indexes, std::vector<std::pair<int, int> > vertex_index_bases, std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents,  std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names, std::vector<std::string> bone_names, std::vector<std::vector<int> > bone_indexes, std::vector<std::vector<float> > bone_weights, KRMesh::model_format_t model_format, bool calculate_normals, bool calculate_tangents) {
    
    clearData();
    
    // TODO, FINDME - These values should be passed as a parameter and set by GUI flags
    bool use_short_vertexes = false;
    bool use_short_normals = true;
    bool use_short_tangents = true;
    bool use_short_uva = true;
    bool use_short_uvb = true;
    
    if(use_short_vertexes) {
        for(std::vector<KRVector3>::iterator itr=vertices.begin(); itr != vertices.end(); itr++) {
            if(fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f || fabsf((*itr).z) > 1.0f) {
                use_short_vertexes = false;
            }
        }
    }
    
    if(use_short_normals) {
        for(std::vector<KRVector3>::iterator itr=normals.begin(); itr != normals.end(); itr++) {
            (*itr).normalize();
        }
    }
    
    if(use_short_tangents) {
        for(std::vector<KRVector3>::iterator itr=tangents.begin(); itr != tangents.end(); itr++) {
            (*itr).normalize();
        }
    }
    
    if(use_short_uva) {
        for(std::vector<KRVector2>::iterator itr=uva.begin(); itr != uva.end(); itr++) {
            if(fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f) {
                use_short_uva = false;
            }
        }
    }
    
    if(use_short_uvb) {
        for(std::vector<KRVector2>::iterator itr=uvb.begin(); itr != uvb.end(); itr++) {
            if(fabsf((*itr).x) > 1.0f || fabsf((*itr).y) > 1.0f) {
                use_short_uvb = false;
            }
        }
    }
    
    __int32_t vertex_attrib_flags = 0;
    if(vertices.size()) {
        if(use_short_vertexes) {
            vertex_attrib_flags |= (1 << KRENGINE_ATTRIB_VERTEX_SHORT);
        } else {
            vertex_attrib_flags |= (1 << KRENGINE_ATTRIB_VERTEX);
        }
    }
    if(normals.size() || calculate_normals) {
        if(use_short_normals) {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_NORMAL_SHORT);
        } else {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_NORMAL);
        }
    }
    if(tangents.size() || calculate_tangents) {
        if(use_short_tangents) {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TANGENT_SHORT);
        } else {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TANGENT);
        }
    }
    if(uva.size()) {
        if(use_short_uva) {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVA_SHORT);
        } else {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVA);
        }
    }
    if(uvb.size()) {
        if(use_short_uvb) {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVB_SHORT);
        } else {
            vertex_attrib_flags += (1 << KRENGINE_ATTRIB_TEXUVB);
        }
    }
    if(bone_names.size()) {
        vertex_attrib_flags += (1 << KRENGINE_ATTRIB_BONEINDEXES) + (1 << KRENGINE_ATTRIB_BONEWEIGHTS);
    }
    size_t vertex_size = VertexSizeForAttributes(vertex_attrib_flags);
    size_t index_count = vertex_indexes.size();
    size_t index_base_count = vertex_index_bases.size();
    size_t submesh_count = submesh_lengths.size();
    size_t vertex_count = vertices.size();
    size_t bone_count = bone_names.size();
    size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(pack_bone) * bone_count + KRALIGN(2 * index_count) + KRALIGN(8 * index_base_count) + vertex_size * vertex_count;
    
    m_pData->expand(new_file_size);
    
    pack_header *pHeader = getHeader();
    memset(pHeader, 0, sizeof(pack_header));
    pHeader->vertex_attrib_flags = vertex_attrib_flags;
    pHeader->submesh_count = (__int32_t)submesh_count;
    pHeader->vertex_count = (__int32_t)vertex_count;
    pHeader->bone_count = (__int32_t)bone_count;
    pHeader->index_count = (__int32_t)index_count;
    pHeader->index_base_count = (__int32_t)index_base_count;
    pHeader->model_format = model_format;
    strcpy(pHeader->szTag, "KROBJPACK1.2   ");
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
    
    
    __uint16_t *index_data = getIndexData();
    for(std::vector<__uint16_t>::iterator itr=vertex_indexes.begin(); itr != vertex_indexes.end(); itr++) {
        *index_data++ = *itr;
    }
    
    __uint32_t *index_base_data = getIndexBaseData();
    for(std::vector<std::pair<int, int> >::iterator itr=vertex_index_bases.begin(); itr != vertex_index_bases.end(); itr++) {
        *index_base_data++ = (*itr).first;
        *index_base_data++ = (*itr).second;
    }
    
    if(getModelFormat() == KRENGINE_MODEL_FORMAT_TRIANGLES) {
        // Calculate missing surface normals and tangents
        //cout << "  Calculate surface normals and tangents\n";
        if(calculate_normals || calculate_tangents) {
            // NOTE: This will not work properly if the vertices are already indexed
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
        }
    }
    
    optimize();
}

KRVector3 KRMesh::getMinPoint() const {
    return m_minPoint;
}

KRVector3 KRMesh::getMaxPoint() const {
    return m_maxPoint;
}

void KRMesh::clearData() {
    m_pData->unload();
}

void KRMesh::clearBuffers() {
    m_submeshes.clear();
}

int KRMesh::getLODCoverage() const {
    return m_lodCoverage;
}

std::string KRMesh::getLODBaseName() const {
    return m_lodBaseName;
}

// Predicate used with std::sort to sort by highest detail model first, decending to lowest detail LOD model
bool KRMesh::lod_sort_predicate(const KRMesh *m1, const KRMesh *m2)
{
    return m1->m_lodCoverage > m2->m_lodCoverage;
}

bool KRMesh::has_vertex_attribute(vertex_attrib_t attribute_type) const
{
    //return (getHeader()->vertex_attrib_flags & (1 << attribute_type)) != 0;
    return has_vertex_attribute(getHeader()->vertex_attrib_flags, attribute_type);
}

bool KRMesh::has_vertex_attribute(int vertex_attrib_flags, vertex_attrib_t attribute_type)
{
    return (vertex_attrib_flags & (1 << attribute_type)) != 0;
}

KRMesh::pack_header *KRMesh::getHeader() const
{
    return (pack_header *)m_pData->getStart();
}

KRMesh::pack_bone *KRMesh::getBone(int index)
{
    pack_header *header = getHeader();
    return (pack_bone *)((unsigned char *)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * header->submesh_count + sizeof(pack_bone) * index);
}

unsigned char *KRMesh::getVertexData() const {
    pack_header *pHeader = getHeader();
    return ((unsigned char *)m_pData->getStart()) + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->index_count) + KRALIGN(8 * pHeader->index_base_count);
}

__uint16_t *KRMesh::getIndexData() const {
    pack_header *pHeader = getHeader();
    return (__uint16_t *)((unsigned char *)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count);
}

__uint32_t *KRMesh::getIndexBaseData() const {
    pack_header *pHeader = getHeader();
    return (__uint32_t *)((unsigned char *)m_pData->getStart() + sizeof(pack_header) + sizeof(pack_material) * pHeader->submesh_count + sizeof(pack_bone) * pHeader->bone_count + KRALIGN(2 * pHeader->index_count));
}


KRMesh::pack_material *KRMesh::getSubmesh(int mesh_index) const
{
    return (pack_material *)((unsigned char *)m_pData->getStart() + sizeof(pack_header)) + mesh_index;
}

unsigned char *KRMesh::getVertexData(int index) const
{
    return getVertexData() + m_vertex_size * index;
}

int KRMesh::getSubmeshCount() const
{
    pack_header *header = getHeader();
    return header->submesh_count;
}

int KRMesh::getVertexCount(int submesh) const
{
    return getSubmesh(submesh)->vertex_count;
}

KRVector3 KRMesh::getVertexPosition(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
        short *v = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX_SHORT]);
        return KRVector3((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector3 KRMesh::getVertexNormal(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
        short *v = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL_SHORT]);
        return KRVector3((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector3 KRMesh::getVertexTangent(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
        short *v = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT_SHORT]);
        return KRVector3((float)v[0] / 32767.0f, (float)v[1] / 32767.0f, (float)v[2] / 32767.0f);
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT)) {
        return KRVector3((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]));
    } else {
        return KRVector3::Zero();
    }
}

KRVector2 KRMesh::getVertexUVA(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
        short *v = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA_SHORT]);
        return KRVector2((float)v[0] / 32767.0f, (float)v[1] / 32767.0f);
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA)) {
        return KRVector2((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]));
    } else {
        return KRVector2::Zero();
    }
}

KRVector2 KRMesh::getVertexUVB(int index) const
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
        short *v = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB_SHORT]);
        return KRVector2((float)v[0] / 32767.0f, (float)v[1] / 32767.0f);
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB)) {
        return KRVector2((float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]));
    } else {
        return KRVector2::Zero();
    }
}

void KRMesh::setVertexPosition(int index, const KRVector3 &v)
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
        short *vert = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX_SHORT]);
        vert[0] = v.x * 32767.0f;
        vert[1] = v.y * 32767.0f;
        vert[2] = v.z * 32767.0f;
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX)) {
        float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_VERTEX]);
        vert[0] = v.x;
        vert[1] = v.y;
        vert[2] = v.z;
    }
}

void KRMesh::setVertexNormal(int index, const KRVector3 &v)
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
        short *vert = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL_SHORT]);
        vert[0] = v.x * 32767.0f;
        vert[1] = v.y * 32767.0f;
        vert[2] = v.z * 32767.0f;
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL)) {
        float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_NORMAL]);
        vert[0] = v.x;
        vert[1] = v.y;
        vert[2] = v.z;
    }
}

void KRMesh::setVertexTangent(int index, const KRVector3 & v)
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
        short *vert = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT_SHORT]);
        vert[0] = v.x * 32767.0f;
        vert[1] = v.y * 32767.0f;
        vert[2] = v.z * 32767.0f;
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT)) {
        float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TANGENT]);
        vert[0] = v.x;
        vert[1] = v.y;
        vert[2] = v.z;
    }
}

void KRMesh::setVertexUVA(int index, const KRVector2 &v)
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
        short *vert = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA_SHORT]);
        vert[0] = v.x * 32767.0f;
        vert[1] = v.y * 32767.0f;
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA)) {
        float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVA]);
        vert[0] = v.x;
        vert[1] = v.y;
    }
}

void KRMesh::setVertexUVB(int index, const KRVector2 &v)
{
    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
        short *vert = (short *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB_SHORT]);
        vert[0] = v.x * 32767.0f;
        vert[1] = v.y * 32767.0f;
    } else if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB)) {
        float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_TEXUVB]);
        vert[0] = v.x;
        vert[1] = v.y;
    }
}


int KRMesh::getBoneIndex(int index, int weight_index) const
{
    unsigned char *vert = (unsigned char *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
    return vert[weight_index];
}

void KRMesh::setBoneIndex(int index, int weight_index, int bone_index)
{
    unsigned char *vert = (unsigned char *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEINDEXES]);
    vert[weight_index] = bone_index;
}

float KRMesh::getBoneWeight(int index, int weight_index) const
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
    return vert[weight_index];
}

void KRMesh::setBoneWeight(int index, int weight_index, float bone_weight)
{
    float *vert = (float *)(getVertexData(index) + m_vertex_attribute_offset[KRENGINE_ATTRIB_BONEWEIGHTS]);
    vert[weight_index] = bone_weight;
}

size_t KRMesh::VertexSizeForAttributes(__int32_t vertex_attrib_flags)
{
    size_t data_size = 0;
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_VERTEX)) {
        data_size += sizeof(float) * 3;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_NORMAL)) {
        data_size += sizeof(float) * 3;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TANGENT)) {
        data_size += sizeof(float) * 3;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVA)) {
        data_size += sizeof(float) * 2;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVB)) {
        data_size += sizeof(float) * 2;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_BONEINDEXES)) {
        data_size += 4; // 4 bytes
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_BONEWEIGHTS)) {
        data_size += sizeof(float) * 4;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_VERTEX_SHORT)) {
        data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_NORMAL_SHORT)) {
        data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TANGENT_SHORT)) {
        data_size += sizeof(short) * 4; // Extra short added in order to maintain 32-bit alignment. TODO, FINDME - Perhaps we can bind this as a vec4 and use the 4th component for another attribute...
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVA_SHORT)) {
        data_size += sizeof(short) * 2;
    }
    if(has_vertex_attribute(vertex_attrib_flags, KRENGINE_ATTRIB_TEXUVB_SHORT)) {
        data_size += sizeof(short) * 2;
    }
    return data_size;
}

void KRMesh::updateAttributeOffsets()
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

size_t KRMesh::AttributeOffset(__int32_t vertex_attrib, __int32_t vertex_attrib_flags)
{
    int mask = 0;
    for(int i=0; i < vertex_attrib; i++) {
        if(vertex_attrib_flags & (1 << i)) {
            mask |= (1 << i);
        }
    }
    return VertexSizeForAttributes(mask);
}

int KRMesh::getBoneCount()
{
    pack_header *header = getHeader();
    return header->bone_count;
}

char *KRMesh::getBoneName(int bone_index)
{
    return getBone(bone_index)->szName;
}

KRMesh::model_format_t KRMesh::getModelFormat() const
{
    return (model_format_t)getHeader()->model_format;
}

bool KRMesh::rayCast(const KRVector3 &line_v0, const KRVector3 &dir, const KRVector3 &tri_v0, const KRVector3 &tri_v1, const KRVector3 &tri_v2, const KRVector3 &tri_n0, const KRVector3 &tri_n1, const KRVector3 &tri_n2, KRHitInfo &hitinfo)
{
    // algorithm based on Dan Sunday's implementation at http://geomalgorithms.com/a06-_intersect-2.html
    const float SMALL_NUM = 0.00000001; // anything that avoids division overflow
    KRVector3   u, v, n;              // triangle vectors
    KRVector3   w0, w;           // ray vectors
    float       r, a, b;              // params to calc ray-plane intersect
    
    // get triangle edge vectors and plane normal
    u = tri_v1 - tri_v0;
    v = tri_v2 - tri_v0;
    n = KRVector3::Cross(u, v); // cross product
    if (n == KRVector3::Zero())             // triangle is degenerate
        return false;                  // do not deal with this case
    
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

    // ---===--- hit_point is in triangle ---===---
    
    
    if(new_hit_distance_sqr < prev_hit_distance_sqr || !hitinfo.didHit()) {
        // Update the hitinfo object if this hit is closer than the prior hit
        
        // Interpolate between the three vertex normals, performing a 3-way lerp of tri_n0, tri_n1, and tri_n2
        float distance_v0 = (tri_v0 - hit_point).magnitude();
        float distance_v1 = (tri_v1 - hit_point).magnitude();
        float distance_v2 = (tri_v2 - hit_point).magnitude();
        float distance_total = distance_v0 + distance_v1 + distance_v2;
        distance_v0 /= distance_total;
        distance_v1 /= distance_total;
        distance_v2 /= distance_total;
        KRVector3 normal = KRVector3::Normalize(tri_n0 * (1.0 - distance_v0) + tri_n1 * (1.0 - distance_v1) + tri_n2 * (1.0 - distance_v2));
        
        hitinfo = KRHitInfo(hit_point, normal);
        return true;
    } else {
        return false; // Either no hit, or the hit was farther than an existing hit
    }
}

/*
bool KRMesh::rayCast(const KRVector3 &line_v0, const KRVector3 &dir, int tri_index0, int tri_index1, int tri_index2, KRHitInfo &hitinfo) const
{
    return rayCast(line_v0, dir, getVertexPosition(tri_index0), getVertexPosition(tri_index1), getVertexPosition(tri_index2), getVertexNormal(tri_index0), getVertexNormal(tri_index1), getVertexNormal(tri_index2), hitinfo);
}
 */

bool KRMesh::rayCast(const KRVector3 &v0, const KRVector3 &dir, KRHitInfo &hitinfo) const
{
    bool hit_found = false;
    for(int submesh_index=0; submesh_index < getSubmeshCount(); submesh_index++) {
//        int vertex_start = getSubmesh(submesh_index)->start_vertex;
        int vertex_count = getVertexCount(submesh_index);
        switch(getModelFormat()) {
            case KRENGINE_MODEL_FORMAT_TRIANGLES:
            case KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
                for(int triangle_index=0; triangle_index < vertex_count / 3; triangle_index++) {
                    int tri_vert_index[3]; // FINDME, HACK!  This is not very efficient for indexed collider meshes...
                    tri_vert_index[0] = getTriangleVertexIndex(submesh_index, triangle_index*3);
                    tri_vert_index[1] = getTriangleVertexIndex(submesh_index, triangle_index*3 + 1);
                    tri_vert_index[2] = getTriangleVertexIndex(submesh_index, triangle_index*3 + 2);
                    
                    if(rayCast(v0, dir, getVertexPosition(tri_vert_index[0]), getVertexPosition(tri_vert_index[1]), getVertexPosition(tri_vert_index[2]), getVertexNormal(tri_vert_index[0]), getVertexNormal(tri_vert_index[1]), getVertexNormal(tri_vert_index[2]), hitinfo)) hit_found = true;
                }
                break;
            /*
             
             NOTE: Not yet supported:
             
            case KRENGINE_MODEL_FORMAT_STRIP:
            case KRENGINE_MODEL_FORMAT_INDEXED_STRIP:
                for(int triangle_index=0; triangle_index < vertex_count - 2; triangle_index++) {
                    int tri_vert_index[3];
                    tri_vert_index[0] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3);
                    tri_vert_index[1] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 1);
                    tri_vert_index[2] = getTriangleVertexIndex(submesh_index, vertex_start + triangle_index*3 + 2);
                    
                    if(rayCast(v0, dir, getVertexPosition(vertex_start + triangle_index), getVertexPosition(vertex_start + triangle_index+1), getVertexPosition(vertex_start + triangle_index+2), getVertexNormal(vertex_start + triangle_index), getVertexNormal(vertex_start + triangle_index+1), getVertexNormal(vertex_start + triangle_index+2), hitinfo)) hit_found = true;
                }
                break;
             */
            default:
                break;
        }
    }
    return hit_found;
}

bool KRMesh::lineCast(const KRVector3 &v0, const KRVector3 &v1, KRHitInfo &hitinfo) const
{
    KRHitInfo new_hitinfo;
    KRVector3 dir = KRVector3::Normalize(v1 - v0);
    if(rayCast(v0, dir, new_hitinfo)) {
        if((new_hitinfo.getPosition() - v0).sqrMagnitude() <= (v1 - v0).sqrMagnitude()) {
            // The hit was between v1 and v2
            hitinfo = new_hitinfo;
            return true;
        }
    }
    return false; // Either no hit, or the hit was beyond v1
}

void KRMesh::convertToIndexed()
{
    
    char *szKey = new char[m_vertex_size * 2 + 1];
    
    // Convert model to indexed vertices, identying vertexes with identical attributes and optimizing order of trianges for best usage post-vertex-transform cache on GPU
    std::vector<__uint16_t> vertex_indexes;
    std::vector<std::pair<int, int> > vertex_index_bases;
    int vertex_index_offset = 0;
    int vertex_index_base_start_vertex = 0;
    
    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector2> uvb;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_starts;
    std::vector<int> submesh_lengths;
    std::vector<std::string> material_names;
    std::vector<std::string> bone_names;
    std::vector<std::vector<int> > bone_indexes;
    std::vector<std::vector<float> > bone_weights;
    
    int bone_count = getBoneCount();
    for(int bone_index=0; bone_index < bone_count; bone_index++) {
        bone_names.push_back(getBoneName(bone_index));
    }
    
    for(int submesh_index=0; submesh_index < getSubmeshCount(); submesh_index++) {
        material_names.push_back(getSubmesh(submesh_index)->szName);
        
        int vertexes_remaining = getVertexCount(submesh_index);
        
        int vertex_count = vertexes_remaining;
        if(vertex_count > 0xffff) {
            vertex_count = 0xffff;
        }
        
        if(submesh_index == 0 || vertex_index_offset + vertex_count > 0xffff) {
            vertex_index_bases.push_back(std::pair<int, int>(vertex_indexes.size(), vertices.size()));
            vertex_index_offset = 0;
            vertex_index_base_start_vertex = vertices.size();
        }
        
        submesh_starts.push_back(vertex_index_bases.size() - 1 + (vertex_index_offset << 16));
        submesh_lengths.push_back(vertexes_remaining);
        int source_index = getSubmesh(submesh_index)->start_vertex;
        
        
        while(vertexes_remaining) {
            
            //typedef std::pair<std::vector<float>, std::vector<int> > vertex_key_t;
            typedef std::string vertex_key_t;
            
            unordered_map<vertex_key_t, int> prev_indexes = unordered_map<vertex_key_t, int>();
            
            for(int i=0; i < vertex_count; i++) {
                
                KRVector3 vertex_position = getVertexPosition(source_index);
                KRVector2 vertex_uva = getVertexUVA(source_index);
                KRVector2 vertex_uvb = getVertexUVB(source_index);
                KRVector3 vertex_normal = getVertexNormal(source_index);
                KRVector3 vertex_tangent = getVertexTangent(source_index);
                std::vector<int> vertex_bone_indexes;
                if(has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
                    vertex_bone_indexes.push_back(getBoneIndex(source_index, 0));
                    vertex_bone_indexes.push_back(getBoneIndex(source_index, 1));
                    vertex_bone_indexes.push_back(getBoneIndex(source_index, 2));
                    vertex_bone_indexes.push_back(getBoneIndex(source_index, 3));
                }
                std::vector<float> vertex_bone_weights;
                if(has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
                    vertex_bone_weights.push_back(getBoneWeight(source_index, 0));
                    vertex_bone_weights.push_back(getBoneWeight(source_index, 1));
                    vertex_bone_weights.push_back(getBoneWeight(source_index, 2));
                    vertex_bone_weights.push_back(getBoneWeight(source_index, 3));
                }
                
                
                
                unsigned char *vertex_data = (unsigned char *)getVertexData(source_index);
                for(int b=0; b < m_vertex_size; b++) {
                    const char *szHex = "0123456789ABCDEF";
                    szKey[b*2] = szHex[vertex_data[b] & 0x0f];
                    szKey[b*2+1] = szHex[((vertex_data[b] & 0xf0) >> 4)];
                }
                szKey[m_vertex_size * 2] = '\0';
                
                vertex_key_t vertex_key = szKey;
                /*
                 
                 vertex_key_t vertex_key = std::make_pair(std::vector<float>(), std::vector<int>());
                
                if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX) || has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
                    vertex_key.first.push_back(vertex_position.x);
                    vertex_key.first.push_back(vertex_position.y);
                    vertex_key.first.push_back(vertex_position.z);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL) || has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
                    vertex_key.first.push_back(vertex_normal.x);
                    vertex_key.first.push_back(vertex_normal.y);
                    vertex_key.first.push_back(vertex_normal.z);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
                    vertex_key.first.push_back(vertex_uva.x);
                    vertex_key.first.push_back(vertex_uva.y);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
                    vertex_key.first.push_back(vertex_uvb.x);
                    vertex_key.first.push_back(vertex_uvb.y);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT) || has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
                    vertex_key.first.push_back(vertex_tangent.x);
                    vertex_key.first.push_back(vertex_tangent.y);
                    vertex_key.first.push_back(vertex_tangent.z);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
                    vertex_key.second.push_back(vertex_bone_indexes[0]);
                    vertex_key.second.push_back(vertex_bone_indexes[1]);
                    vertex_key.second.push_back(vertex_bone_indexes[2]);
                    vertex_key.second.push_back(vertex_bone_indexes[3]);
                }
                if(has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
                    vertex_key.first.push_back(vertex_bone_weights[0]);
                    vertex_key.first.push_back(vertex_bone_weights[1]);
                    vertex_key.first.push_back(vertex_bone_weights[2]);
                    vertex_key.first.push_back(vertex_bone_weights[3]);
                }
                */
                int found_index = -1;
                if(prev_indexes.count(vertex_key) == 0) {
                    found_index = vertices.size() - vertex_index_base_start_vertex;
                    if(has_vertex_attribute(KRENGINE_ATTRIB_VERTEX) || has_vertex_attribute(KRENGINE_ATTRIB_VERTEX_SHORT)) {
                        vertices.push_back(vertex_position);
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_NORMAL) || has_vertex_attribute(KRENGINE_ATTRIB_NORMAL_SHORT)) {
                        normals.push_back(vertex_normal);
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_TANGENT) || has_vertex_attribute(KRENGINE_ATTRIB_TANGENT_SHORT)) {
                        tangents.push_back(vertex_tangent);
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVA_SHORT)) {
                        uva.push_back(vertex_uva);
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB) || has_vertex_attribute(KRENGINE_ATTRIB_TEXUVB_SHORT)) {
                        uvb.push_back(vertex_uvb);
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_BONEINDEXES)) {
                        bone_indexes.push_back(vertex_bone_indexes);
                        
                    }
                    if(has_vertex_attribute(KRENGINE_ATTRIB_BONEWEIGHTS)) {
                        bone_weights.push_back(vertex_bone_weights);
                    }
                    prev_indexes[vertex_key] = found_index;
                } else {
                    found_index = prev_indexes[vertex_key];
                }
                
                vertex_indexes.push_back(found_index);
                //fprintf(stderr, "Submesh: %6i  IndexBase: %3i  Index: %6i\n", submesh_index, vertex_index_bases.size(), found_index);
                
                source_index++;
            }
            
            vertexes_remaining -= vertex_count;
            vertex_index_offset += vertex_count;
            
            
            vertex_count = vertexes_remaining;
            if(vertex_count > 0xffff) {
                vertex_count = 0xffff;
            }
            
            if(vertex_index_offset + vertex_count > 0xffff) {
                vertex_index_bases.push_back(std::pair<int, int>(vertex_indexes.size(), vertices.size()));
                vertex_index_offset = 0;
                vertex_index_base_start_vertex = vertices.size();
            }
        }
    }
    
    delete szKey;
    
    fprintf(stderr, "Convert to indexed, before: %i after: %i \(%.2f%% saving)\n", getHeader()->vertex_count, vertices.size(), ((float)getHeader()->vertex_count - (float)vertices.size()) / (float)getHeader()->vertex_count * 100.0f);
    
    
    
    LoadData(vertex_indexes, vertex_index_bases, vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names, bone_names, bone_indexes, bone_weights, KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES, false, false);
}

void KRMesh::optimize()
{
    switch(getModelFormat()) {
        case KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
            optimizeIndexes();
            break;
        default:
            convertToIndexed(); // HACK, FINDME, TODO - This may not be ideal in every case and should be exposed through the API independently
            break;
    }
}

void KRMesh::getIndexedRange(int index_group, int &start_index_offset, int &start_vertex_offset, int &index_count, int &vertex_count) const {
    pack_header *h = getHeader();
    __uint32_t *index_base_data = getIndexBaseData();
    start_index_offset = index_base_data[index_group * 2];
    start_vertex_offset = index_base_data[index_group * 2 + 1];
    if(index_group + 1 < h->index_base_count) {
        index_count = index_base_data[index_group * 2 + 2] - start_index_offset;
        vertex_count = index_base_data[index_group * 2 + 3] - start_vertex_offset;
    } else {
        index_count = h->index_count - start_index_offset;
        vertex_count = h->vertex_count - start_vertex_offset;
    }
}

int KRMesh::getTriangleVertexIndex(int submesh, int index) const
{
    switch(getModelFormat()) {
        case KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES:
            {
                int start_index_offset, start_vertex_offset, index_count, vertex_count;
                int index_group = getSubmesh(submesh)->index_group;
                int index_group_offset = getSubmesh(submesh)->index_group_offset;
                int remaining_vertices = index_group_offset + index;
                getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
                while(remaining_vertices >= index_count) {
                    remaining_vertices -= index_count;
                    getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
                }
                return getIndexData()[start_index_offset + remaining_vertices] + start_vertex_offset;
            }
            break;
        default:
            return getSubmesh(submesh)->start_vertex + index;
            break;
    }
}

void KRMesh::optimizeIndexes()
{
    if(getModelFormat() != KRENGINE_MODEL_FORMAT_INDEXED_TRIANGLES) return;
    
    __uint16_t *new_indices = (__uint16_t *)malloc(0x10000 * sizeof(__uint16_t));
    __uint16_t *vertex_mapping = (__uint16_t *)malloc(0x10000 * sizeof(__uint16_t));
    unsigned char *new_vertex_data = (unsigned char *)malloc(m_vertex_size * 0x10000);
    
    // FINDME, TODO, HACK - This will segfault if the KRData object is still mmap'ed to a read-only file.  Need to detach from the file before calling this function.  Currently, this function is only being used during the import process, so it isn't going to cause any problems for now.
    
    pack_header *header = getHeader();
    
    __uint16_t *index_data = getIndexData();
    unsigned char *vertex_data = getVertexData();
    
    for(int submesh_index=0; submesh_index < header->submesh_count; submesh_index++) {
        pack_material *submesh = getSubmesh(submesh_index);
        int vertexes_remaining = submesh->vertex_count;
        int index_group = getSubmesh(submesh_index)->index_group;
        int index_group_offset = getSubmesh(submesh_index)->index_group_offset;
        while(vertexes_remaining > 0) {
            int start_index_offset, start_vertex_offset, index_count, vertex_count;
            getIndexedRange(index_group++, start_index_offset, start_vertex_offset, index_count, vertex_count);
            
            int vertexes_to_process = vertexes_remaining;
            if(vertexes_to_process + index_group_offset > 0xffff) {
                vertexes_to_process = 0xffff - index_group_offset;
            }
            
            __uint16_t *index_data_start = index_data + start_index_offset + index_group_offset;
            
            
            // ----====---- Step 1: Optimize triangle drawing order to maximize use of the GPU's post-transform vertex cache ----====----
            Forsyth::OptimizeFaces(index_data_start, vertexes_to_process, vertex_count, new_indices, 16); // FINDME, TODO - GPU post-transform vertex cache size of 16 should be configureable
            memcpy(index_data_start, new_indices, vertexes_to_process * sizeof(__uint16_t));
            vertexes_remaining -= vertexes_to_process;
            
            /*
             
             unsigned char * vertex_data_start = vertex_data + start_vertex_offset;
             
            // ----====---- Step 2: Re-order the vertex data to maintain cache coherency ----====----
            for(int i=0; i < vertex_count; i++) {
                vertex_mapping[i] = i;
            }
            int new_vertex_index=0;
            for(int index_number=0; index_number<index_count; index_number++) {
                int prev_vertex_index = index_data_start[index_number];
                if(prev_vertex_index > new_vertex_index) {
                    // Swap prev_vertex_index and new_vertex_index
                    
                    for(int i=0; i < index_count; i++) {
                        if(index_data_start[i] == prev_vertex_index) {
                            index_data_start[i] = new_vertex_index;
                        } else if(index_data_start[i] == new_vertex_index) {
                            index_data_start[i] = prev_vertex_index;
                        }
                    }
                    
                    int tmp = vertex_mapping[prev_vertex_index];
                    vertex_mapping[prev_vertex_index] = vertex_mapping[new_vertex_index];
                    vertex_mapping[new_vertex_index] = tmp;
                    
                    
                    new_vertex_index++;
                }
            }
            
            for(int i=0; i < vertex_count; i++) {
                memcpy(new_vertex_data + vertex_mapping[i] * m_vertex_size, vertex_data_start + i * m_vertex_size, m_vertex_size);
            }
            memcpy(vertex_data_start, new_vertex_data, vertex_count * m_vertex_size);
             */
            
            
            
            index_group_offset = 0;
        }
    }
    
    free(new_indices);
    free(vertex_mapping);
    free(new_vertex_data);
}
