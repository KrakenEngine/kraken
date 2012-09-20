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

#include "KRMesh.h"
#import "KRShader.h"
#import "KRContext.h"
#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

KRMesh::KRMesh(KRContext &context, std::string name) : KRResource(context, name) {
    m_pData = new KRDataBlock();
}

KRMesh::~KRMesh() {
    clearData();
    if(m_pData) delete m_pData;
}

std::string KRMesh::getExtension() {
    return "krobject";
}

void KRMesh::clearData() {
    m_pData->unload();
}

void KRMesh::clearBuffers() {
    m_submeshes.clear();
}

void KRMesh::loadPack(KRDataBlock *data) {
    clearData();
    delete m_pData;
    m_pData = data;
    pack_header *pHeader = (pack_header *)m_pData->getStart();
    m_minPoint = KRVector3(pHeader->minx, pHeader->miny, pHeader->minz);
    m_maxPoint = KRVector3(pHeader->maxx, pHeader->maxy, pHeader->maxz);
}

bool KRMesh::save(const std::string& path) {
    clearBuffers();
    return m_pData->save(path);
}

GLfloat KRMesh::getMaxDimension() {
    GLfloat m = 0.0;
    if(m_maxPoint.x - m_minPoint.x > m) m = m_maxPoint.x - m_minPoint.x;
    if(m_maxPoint.y - m_minPoint.y > m) m = m_maxPoint.y - m_minPoint.y;
    if(m_maxPoint.z - m_minPoint.z > m) m = m_maxPoint.z - m_minPoint.z;
    return m;
}

vector<KRMesh::Submesh *> KRMesh::getSubmeshes() {
    if(m_submeshes.size() == 0) {
        pack_header *pHeader = (pack_header *)m_pData->getStart();
        pack_material *pPackMaterials = (pack_material *)(pHeader+1);
        m_submeshes.clear();
        for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
            pack_material *pPackMaterial = pPackMaterials + iMaterial;
            
            Submesh *pSubmesh = new Submesh();
            pSubmesh->start_vertex = pPackMaterial->start_vertex;
            pSubmesh->vertex_count = pPackMaterial->vertex_count;
            
            strncpy(pSubmesh->szMaterialName, pPackMaterial->szName, 256);
            pSubmesh->szMaterialName[255] = '\0';
            //fprintf(stderr, "Submesh material: \"%s\"\n", pSubmesh->szMaterialName);
            m_submeshes.push_back(pSubmesh);
        }
    }
    return m_submeshes;
}

void KRMesh::renderSubmesh(int iSubmesh) {
    VertexData *pVertexData = getVertexData();
    
    pack_header *pHeader = (pack_header *)m_pData->getStart();
    int cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
    
    vector<KRMesh::Submesh *> submeshes = getSubmeshes();
    Submesh *pSubmesh = submeshes[iSubmesh];
    
    int iVertex = pSubmesh->start_vertex;
    int iBuffer = iVertex / MAX_VBO_SIZE;
    iVertex = iVertex % MAX_VBO_SIZE;
    int cVertexes = pSubmesh->vertex_count;
    while(cVertexes > 0) {
        GLsizei cBufferVertexes = iBuffer < cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
        int vertex_size = sizeof(VertexData) ;
        assert(pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size >= m_pData->getStart());
        
        void *vbo_end = (unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size + vertex_size * cBufferVertexes;
        void *buffer_end = m_pData->getEnd();
        assert(vbo_end <= buffer_end);
        assert(cBufferVertexes <= 65535);
        
        m_pContext->getModelManager()->bindVBO((unsigned char *)pVertexData + iBuffer * MAX_VBO_SIZE * vertex_size, vertex_size * cBufferVertexes, true, true, true, true, true);

    
        if(iVertex + cVertexes >= MAX_VBO_SIZE) {
            assert(iVertex + (MAX_VBO_SIZE  - iVertex) <= cBufferVertexes);
            GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, (MAX_VBO_SIZE  - iVertex)));
            
            cVertexes -= (MAX_VBO_SIZE - iVertex);
            iVertex = 0;
            iBuffer++;
        } else {
            assert(iVertex + cVertexes <= cBufferVertexes);

            GLDEBUG(glDrawArrays(GL_TRIANGLES, iVertex, cVertexes));
            cVertexes = 0;
        }

    }
}

KRMesh::VertexData *KRMesh::getVertexData() {
    pack_header *pHeader = (pack_header *)m_pData->getStart();
    pack_material *pPackMaterials = (pack_material *)(pHeader+1);
    return (VertexData *)(pPackMaterials + pHeader->submesh_count);
}

void KRMesh::LoadData(std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents,  std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names) {
    
    clearData();
    
    int submesh_count = submesh_lengths.size();
    int vertex_count = vertices.size();
    size_t new_file_size = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(VertexData) * vertex_count;
    m_pData->expand(new_file_size);
    
    pack_header *pHeader = (pack_header *)m_pData->getStart();
    memset(pHeader, 0, sizeof(pack_header));
    
    pHeader->submesh_count = submesh_lengths.size();
    pHeader->vertex_count = vertices.size();
    strcpy(pHeader->szTag, "KROBJPACK1.0   ");
    
    pack_material *pPackMaterials = (pack_material *)(pHeader+1);
    
    for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
        pack_material *pPackMaterial = pPackMaterials + iMaterial;
        pPackMaterial->start_vertex = submesh_starts[iMaterial];
        pPackMaterial->vertex_count = submesh_lengths[iMaterial];
        strncpy(pPackMaterial->szName, material_names[iMaterial].c_str(), 63);
    }
    
    bool bFirstVertex = true;
    
    VertexData *pVertexData = (VertexData *)(pPackMaterials + pHeader->submesh_count);
    VertexData *pVertex = pVertexData;
    for(int iVertex=0; iVertex < vertices.size(); iVertex++) {
        memset(pVertex, 0, sizeof(VertexData));
        KRVector3 source_vertex = vertices[iVertex];
        pVertex->vertex.x = source_vertex.x;
        pVertex->vertex.y = source_vertex.y;
        pVertex->vertex.z = source_vertex.z;
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
            KRVector2 source_uva = uva[iVertex];
            pVertex->uva.u = source_uva.x;
            pVertex->uva.v = source_uva.y;
        } else {
            pVertex->uva.u = 0.0;
            pVertex->uva.v = 0.0;
        }
        if(uvb.size() > iVertex) {
            KRVector2 source_uvb = uvb[iVertex];
            pVertex->uvb.u = source_uvb.x;
            pVertex->uvb.v = source_uvb.y;
        } else {
            pVertex->uvb.u = 0.0;
            pVertex->uvb.v = 0.0;
        }
        if(normals.size() > iVertex) {
            KRVector3 source_normal = normals[iVertex];
            pVertex->normal.x = source_normal.x;
            pVertex->normal.y = source_normal.y;
            pVertex->normal.z = source_normal.z;
        } else {
            pVertex->normal.x = 0.0f;
            pVertex->normal.y = 0.0f;
            pVertex->normal.z = 0.0f;
        }
        if(tangents.size() > iVertex) {
            KRVector3 source_tangent = tangents[iVertex];
            pVertex->tangent.x = source_tangent.x;
            pVertex->tangent.y = source_tangent.y;
            pVertex->tangent.z = source_tangent.z;
        } else {
            pVertex->tangent.x = 0.0f;
            pVertex->tangent.y = 0.0f;
            pVertex->tangent.z = 0.0f;
        }
        
        pVertex++;
    }
    
    pHeader->minx = m_minPoint.x;
    pHeader->miny = m_minPoint.y;
    pHeader->minz = m_minPoint.z;
    pHeader->maxx = m_maxPoint.x;
    pHeader->maxy = m_maxPoint.y;
    pHeader->maxz = m_maxPoint.z;
    
    
    // Calculate missing surface normals and tangents
    //cout << "  Calculate surface normals and tangents\n";
    VertexData *pStart = pVertexData;
    VertexData *pEnd = pStart + vertex_count;
    
    for(VertexData *pVertex = pStart; pVertex < pEnd; pVertex+=3) {
        KRVector3 p1(pVertex[0].vertex.x, pVertex[0].vertex.y, pVertex[0].vertex.z);
        KRVector3 p2(pVertex[1].vertex.x, pVertex[1].vertex.y, pVertex[1].vertex.z);
        KRVector3 p3(pVertex[2].vertex.x, pVertex[2].vertex.y, pVertex[2].vertex.z);
        KRVector3 v1 = p2 - p1;
        KRVector3 v2 = p3 - p1;
        
        // -- Calculate normal --
        if(pVertex->normal.x == 0 && pVertex->normal.y == 0 && pVertex->normal.z == 0) {
            
            KRVector3 normal = KRVector3::Cross(v1, v2);
            
            normal.normalize();
            
            pVertex[0].normal.x = normal.x;
            pVertex[0].normal.y = normal.y;
            pVertex[0].normal.z = normal.z;
            
            pVertex[1].normal.x = normal.x;
            pVertex[1].normal.y = normal.y;
            pVertex[1].normal.z = normal.z;
            
            pVertex[2].normal.x = normal.x;
            pVertex[2].normal.y = normal.y;
            pVertex[2].normal.z = normal.z;
        }
        
        // -- Calculate tangent vector for normal mapping --
        if(pVertex->tangent.x == 0 && pVertex->tangent.y == 0 && pVertex->tangent.z == 0) {
            TexCoord st1; // = pVertex[2].texcoord;
            TexCoord st2; // = pVertex[1].texcoord;
            st1.u = pVertex[1].uva.u - pVertex[0].uva.u;
            st1.v = pVertex[1].uva.v - pVertex[0].uva.v;
            st2.u = pVertex[2].uva.u - pVertex[0].uva.u;
            st2.v = pVertex[2].uva.v - pVertex[0].uva.v;
            double coef = 1/ (st1.u * st2.v - st2.u * st1.v);

            pVertex[0].tangent.x = coef * ((v1.x * st2.v)  + (v2.x * -st1.v));
            pVertex[0].tangent.y = coef * ((v1.y * st2.v)  + (v2.y * -st1.v));
            pVertex[0].tangent.z = coef * ((v1.z * st2.v)  + (v2.z * -st1.v));

            KRVector3 tangent(
                              coef * ((v1.x * st2.v)  + (v2.x * -st1.v)),
                              coef * ((v1.y * st2.v)  + (v2.y * -st1.v)),
                              coef * ((v1.z * st2.v)  + (v2.z * -st1.v))
                              );

            tangent.normalize();

            pVertex[0].tangent.x = tangent.x;
            pVertex[0].tangent.y = tangent.y;
            pVertex[0].tangent.z = tangent.z;                             
            pVertex[1].tangent.x = tangent.x;
            pVertex[1].tangent.y = tangent.y;
            pVertex[1].tangent.z = tangent.z;
            pVertex[2].tangent.x = tangent.x;
            pVertex[2].tangent.y = tangent.y;
            pVertex[2].tangent.z = tangent.z;
        }
    }
}

KRVector3 KRMesh::getMinPoint() const {
    return m_minPoint;
}

KRVector3 KRMesh::getMaxPoint() const {
    return m_maxPoint;
}