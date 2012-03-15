//
//  KRMesh.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-15.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
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
                
                
                pack_header *pHeader = (pack_header *)m_pPackData;
                
                m_minx = pHeader->minx;
                m_miny = pHeader->miny;
                m_minz = pHeader->minz;
                m_maxx = pHeader->maxx;
                m_maxy = pHeader->maxy;
                m_maxz = pHeader->maxz;
                
                pack_material *pPackMaterials = (pack_material *)(pHeader+1);
                
                for(int iMaterial=0; iMaterial < pHeader->submesh_count; iMaterial++) {
                    pack_material *pPackMaterial = pPackMaterials + iMaterial;
                    
                    Submesh *pSubmesh = new Submesh();
                    pSubmesh->start_vertex = pPackMaterial->start_vertex;
                    pSubmesh->vertex_count = pPackMaterial->vertex_count;
                    strcpy(pSubmesh->szMaterialName, pPackMaterial->szName);
                    m_submeshes.push_back(pSubmesh);
                }
                
                m_pVertexData = (VertexData *)(pPackMaterials + pHeader->submesh_count);
                
                
                m_cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
                m_pBuffers = new GLuint[m_cBuffers];
                glGenBuffers(m_cBuffers, m_pBuffers);
                for(GLsizei iBuffer=0; iBuffer < m_cBuffers; iBuffer++) {
                    GLsizei cVertexes = iBuffer < m_cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
                    glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[iBuffer]);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * cVertexes, m_pVertexData + iBuffer * MAX_VBO_SIZE, GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    
                }
            }
        }
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
    return m_submeshes;
}

void KRMesh::renderSubmesh(int iSubmesh) {
    Submesh *pSubmesh = m_submeshes[iSubmesh];
    
    
    int iVertex = pSubmesh->start_vertex;
    int iBuffer = iVertex / MAX_VBO_SIZE;
    iVertex = iVertex % MAX_VBO_SIZE;
    int cVertexes = pSubmesh->vertex_count;
    while(cVertexes > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[iBuffer]);
        
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(0));
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
        
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_NORMAL, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D)));
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_NORMAL);
        
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TANGENT, 3, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D) + sizeof(Vector3D)));
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TANGENT);
        
        glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_TEXUV, 2, GL_FLOAT, 0, sizeof(VertexData), BUFFER_OFFSET(sizeof(Vertex3D) + sizeof(Vector3D) * 2));
        glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_TEXUV);
        
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
