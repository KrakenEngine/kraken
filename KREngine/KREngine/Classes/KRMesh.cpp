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

KRMesh::KRMesh() {
    m_fdPackFile = 0;
    m_pPackData = NULL;
    m_iPackFileSize = 0;
    m_cBuffers = 0;
    m_pBuffers = NULL;
}

KRMesh::~KRMesh() {
    if(m_fdPackFile) {
        if(m_pPackData != NULL) {
            munmap(m_pPackData, m_iPackFileSize);
            m_pPackData = NULL;
        }
        close(m_fdPackFile);
    } else {
        // If we didn't load a packed file, then the data was calculated at run time and malloc'ed
        if(m_pPackData != NULL) {
            free(m_pPackData);
            m_pPackData = NULL;
        }
    }
    
    clearBuffers();
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
    struct stat statbuf;
    m_fdPackFile = open(path.c_str(), O_RDONLY);
    if(m_fdPackFile >= 0) {
        if(fstat(m_fdPackFile, &statbuf) >= 0) {
            if ((m_pPackData = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, m_fdPackFile, 0)) == (caddr_t) -1) {
            } else {
                m_iPackFileSize = statbuf.st_size;
                
                clearBuffers();
                
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

bool KRMesh::writePack(std::string path) {
    clearBuffers();
    
    int fdNewFile = open(path.c_str(), O_RDWR);
    if(fdNewFile == 0) {
        return false;
    } else {
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
                glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUV);
            }
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(0));
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D)));
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D) + sizeof(KRVector3D)));
            
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D) + sizeof(KRVector3D) * 2));


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
