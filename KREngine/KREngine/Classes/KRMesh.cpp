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
#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

KRMesh::KRMesh(KRContext &context, std::string name) : KRResource(context, name) {
    m_fdPackFile = 0;
    m_pPackData = NULL;
    m_iPackFileSize = 0;
    m_cBuffers = 0;
    m_pBuffers = NULL;
}

KRMesh::~KRMesh() {
    clearData();
}

std::string KRMesh::getExtension() {
    return "krobject";
}

void KRMesh::clearData() {
    clearBuffers();
    if(m_fdPackFile) {
        if(m_pPackData != NULL) {
            munmap(m_pPackData, m_iPackFileSize);
            m_pPackData = NULL;
        }
        close(m_fdPackFile);
        m_fdPackFile = 0;
    } else {
        // If we didn't load a packed file, then the data was calculated at run time and malloc'ed
        if(m_pPackData != NULL) {
            free(m_pPackData);
            m_pPackData = NULL;
        }
    }
}

void KRMesh::clearBuffers() {
    m_submeshes.clear();
    if(m_pBuffers != NULL) {
        glDeleteBuffers(m_cBuffers, m_pBuffers);
        delete m_pBuffers;
        m_pBuffers = NULL;
    }
}

void KRMesh::loadPack(std::string path) { 
    clearData();
    struct stat statbuf;
    m_fdPackFile = open(path.c_str(), O_RDONLY);
    if(m_fdPackFile >= 0) {
        if(fstat(m_fdPackFile, &statbuf) >= 0) {
            if ((m_pPackData = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, m_fdPackFile, 0)) == (caddr_t) -1) {
            } else {
                m_iPackFileSize = statbuf.st_size;
                
                pack_header *pHeader = (pack_header *)m_pPackData;
                
                m_minx = pHeader->minx;
                m_miny = pHeader->miny;
                m_minz = pHeader->minz;
                m_maxx = pHeader->maxx;
                m_maxy = pHeader->maxy;
                m_maxz = pHeader->maxz;

            }
        }
    }
}

bool KRMesh::save(const std::string& path) {
    clearBuffers();
    
    int fdNewFile = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if(fdNewFile == -1) {
        return false;
    } else {
        // Seek to end of file and write a byte to enlarge it
        lseek(fdNewFile, m_iPackFileSize-1, SEEK_SET);
        write(fdNewFile, "", 1);
        
        // Now map it...
        void *pNewData = mmap(0, m_iPackFileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdNewFile, 0);
        if(pNewData == (caddr_t) -1) {
            close(fdNewFile);
            return false;
        } else {
            memcpy(pNewData, m_pPackData, m_iPackFileSize);
            mprotect(pNewData, m_iPackFileSize, PROT_READ);
            if(m_fdPackFile) {
                if(m_pPackData != NULL) {
                    void *malloc_data = malloc(m_iPackFileSize);
                    memcpy(malloc_data, m_pPackData, m_iPackFileSize);
                    munmap(m_pPackData, m_iPackFileSize);
                }
                close(m_fdPackFile);
            }
            m_fdPackFile = fdNewFile;
            m_pPackData = pNewData;
            return true;
        }
    }
}

void KRMesh::unmap() {
    clearBuffers();
    if(m_fdPackFile) {
        if(m_pPackData != NULL) {
            void *malloc_data = malloc(m_iPackFileSize);
            memcpy(malloc_data, m_pPackData, m_iPackFileSize);
            munmap(m_pPackData, m_iPackFileSize);
            m_pPackData = malloc_data;
        }
        close(m_fdPackFile);
    }
}

GLfloat KRMesh::getMaxDimension() {
    GLfloat m = 0.0;
    if(m_maxx - m_minx > m) m = m_maxx - m_minx;
    if(m_maxy - m_miny > m) m = m_maxy - m_miny;
    if(m_maxz - m_minz > m) m = m_maxz - m_minz;
    return m;
}

GLfloat KRMesh::getMinX() {
    return m_minx;
}
GLfloat KRMesh::getMaxX() {
    return m_maxx;
}
GLfloat KRMesh::getMinY() {
    return m_miny;
}
GLfloat KRMesh::getMaxY() {
    return m_maxy;
}
GLfloat KRMesh::getMinZ() {
    return m_minz;
}
GLfloat KRMesh::getMaxZ() {
    return m_maxz;
}

vector<KRMesh::Submesh *> KRMesh::getSubmeshes() {
    if(m_submeshes.size() == 0) {
        pack_header *pHeader = (pack_header *)m_pPackData;
        pack_material *pPackMaterials = (pack_material *)(pHeader+1);
        m_submeshes.clear();
        for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
            pack_material *pPackMaterial = pPackMaterials + iMaterial;
            
            Submesh *pSubmesh = new Submesh();
            pSubmesh->start_vertex = pPackMaterial->start_vertex;
            pSubmesh->vertex_count = pPackMaterial->vertex_count;
            strcpy(pSubmesh->szMaterialName, pPackMaterial->szName);
            m_submeshes.push_back(pSubmesh);
        }
    }
    return m_submeshes;
}

void KRMesh::renderSubmesh(int iSubmesh, int *iPrevBuffer) {
    VertexData *pVertexData = getVertexData();
    
    if(m_cBuffers == 0) {
        pack_header *pHeader = (pack_header *)m_pPackData;
        m_cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
        m_pBuffers = new GLuint[m_cBuffers];
        glGenBuffers(m_cBuffers, m_pBuffers);
        for(GLsizei iBuffer=0; iBuffer < m_cBuffers; iBuffer++) {
            GLsizei cVertexes = iBuffer < m_cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
            glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[iBuffer]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * cVertexes, pVertexData + iBuffer * MAX_VBO_SIZE, GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            
        }
    }
    
    vector<KRMesh::Submesh *> submeshes = getSubmeshes();
    Submesh *pSubmesh = submeshes[iSubmesh];
    
    int iVertex = pSubmesh->start_vertex;
    int iBuffer = iVertex / MAX_VBO_SIZE;
    iVertex = iVertex % MAX_VBO_SIZE;
    int cVertexes = pSubmesh->vertex_count;
    while(cVertexes > 0) {
        if(*iPrevBuffer != iBuffer) {
            glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[iBuffer]);
        
            if(*iPrevBuffer == -1) {
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL);
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT);
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVA);
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUVB);
            }
            
            int data_size = sizeof(VertexData);
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(0));
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(sizeof(KRVector3D)));
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, 0, data_size, BUFFER_OFFSET(sizeof(KRVector3D) * 2));
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVA, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(sizeof(KRVector3D) * 3));
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUVB, 2, GL_FLOAT, 0, data_size, BUFFER_OFFSET(sizeof(KRVector3D) * 3 + sizeof(TexCoord)));

            *iPrevBuffer = iBuffer;
        }
        
        if(iVertex + cVertexes >= MAX_VBO_SIZE) {
            glDrawArrays(GL_TRIANGLES, iVertex, (MAX_VBO_SIZE  - iVertex));
            cVertexes -= (MAX_VBO_SIZE - iVertex);
            iVertex = 0;
            iBuffer++;
        } else {
            glDrawArrays(GL_TRIANGLES, iVertex, cVertexes);
            cVertexes = 0;
        }
    }
}

KRMesh::VertexData *KRMesh::getVertexData() {
    pack_header *pHeader = (pack_header *)m_pPackData;
    pack_material *pPackMaterials = (pack_material *)(pHeader+1);
    return (VertexData *)(pPackMaterials + pHeader->submesh_count);
}

void KRMesh::LoadData(std::vector<KRVector3> vertices, std::vector<KRVector2> uva, std::vector<KRVector2> uvb, std::vector<KRVector3> normals, std::vector<KRVector3> tangents,  std::vector<int> submesh_starts, std::vector<int> submesh_lengths, std::vector<std::string> material_names) {
    
    clearData();
    
    int submesh_count = submesh_lengths.size();
    int vertex_count = vertices.size();
    m_iPackFileSize = sizeof(pack_header) + sizeof(pack_material) * submesh_count + sizeof(VertexData) * vertex_count;
    m_pPackData = malloc(m_iPackFileSize);
    
    pack_header *pHeader = (pack_header *)m_pPackData;
    memset(pHeader, 0, sizeof(pack_header));
    
    pHeader->submesh_count = submesh_lengths.size();
    pHeader->vertex_count = vertices.size();
    strcpy(pHeader->szTag, "KROBJPACK1.0   ");
    
    pack_material *pPackMaterials = (pack_material *)(pHeader+1);
    
    for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
        pack_material *pPackMaterial = pPackMaterials + iMaterial;
        pPackMaterial->start_vertex = submesh_starts[iMaterial];
        pPackMaterial->vertex_count = submesh_lengths[iMaterial];
        strcpy(pPackMaterial->szName, material_names[iMaterial].c_str());
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
            m_minx = source_vertex.x;
            m_miny = source_vertex.y;
            m_minz = source_vertex.z;
            m_maxx = source_vertex.x;
            m_maxy = source_vertex.y;
            m_maxz = source_vertex.z;
        } else {
            if(source_vertex.x < m_minx) m_minx = source_vertex.x;
            if(source_vertex.y < m_miny) m_miny = source_vertex.y;
            if(source_vertex.z < m_minz) m_minz = source_vertex.z;
            if(source_vertex.x > m_maxx) m_maxx = source_vertex.x;
            if(source_vertex.y > m_maxy) m_maxy = source_vertex.y;
            if(source_vertex.z > m_maxz) m_maxz = source_vertex.z;
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
    
    pHeader->minx = m_minx;
    pHeader->miny = m_miny;
    pHeader->minz = m_minz;
    pHeader->maxx = m_maxx;
    pHeader->maxy = m_maxy;
    pHeader->maxz = m_maxz;
    
    
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

