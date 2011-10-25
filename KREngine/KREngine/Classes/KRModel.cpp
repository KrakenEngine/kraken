//
//  KRModel.cpp
//  gldemo
//
//  Copyright 2011 Kearwood Gilbert. All rights reserved.
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

#define MAX_VBO_SIZE 65535
// MAX_VBO_SIZE must be divisible by 3 so triangles aren't split across VBO objects...

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

KRModel::KRModel(std::string path, KRMaterialManager *pMaterialManager) {

    m_fdPackFile = 0;
    m_pPackFile = NULL;
    m_iPackFileSize = 0;
    m_cBuffers = 0;
    m_pBuffers = NULL;
    
    // loadWavefront(path, pMaterialManager);
    loadPack(path, pMaterialManager);
}

void KRModel::loadPack(std::string path, KRMaterialManager *pMaterialManager) {    
    struct stat statbuf;
    m_fdPackFile = open(path.c_str(), O_RDONLY);
    if(m_fdPackFile >= 0) {
        if(fstat(m_fdPackFile, &statbuf) >= 0) {
            if ((m_pPackFile = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, m_fdPackFile, 0)) == (caddr_t) -1) {
            } else {
                m_iPackFileSize = statbuf.st_size;
                
                
                pack_header *pHeader = (pack_header *)m_pPackFile;
                
                m_minx = pHeader->minx;
                m_miny = pHeader->miny;
                m_minz = pHeader->minz;
                m_maxx = pHeader->maxx;
                m_maxy = pHeader->maxy;
                m_maxz = pHeader->maxz;
                
                pack_material *pPackMaterials = (pack_material *)(pHeader+1);
                
                for(int iMaterial=0; iMaterial < pHeader->material_count; iMaterial++) {
                    pack_material *pPackMaterial = pPackMaterials + iMaterial;
                    
                    Material *pMaterial = new Material();
                    pMaterial->start_vertex = pPackMaterial->start_vertex;
                    pMaterial->vertex_count = pPackMaterial->vertex_count;
                    pMaterial->pMaterial = pMaterialManager->getMaterial(pPackMaterial->szName);
                    m_materials.push_back(pMaterial);
                }
                
                m_pVertexData = (VertexData *)(pPackMaterials + pHeader->material_count);
                
                
                m_cBuffers = (pHeader->vertex_count + MAX_VBO_SIZE - 1) / MAX_VBO_SIZE;
                m_pBuffers = new GLuint[m_cBuffers];
                glGenBuffers(m_cBuffers, m_pBuffers);
                for(GLsizei iBuffer=0; iBuffer < m_cBuffers; iBuffer++) {
//                    if(iBuffer < 30) {
                        GLsizei cVertexes = iBuffer < m_cBuffers - 1 ? MAX_VBO_SIZE : pHeader->vertex_count % MAX_VBO_SIZE;
                        glBindBuffer(GL_ARRAY_BUFFER, m_pBuffers[iBuffer]);
                        glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * cVertexes, m_pVertexData + iBuffer * MAX_VBO_SIZE, GL_STATIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
//                    }
                    
                }
            }
        }
    }
}

void KRModel::loadWavefront(std::string path, KRMaterialManager *pMaterialManager) {
    const char *szPath = path.c_str();
    
    int fdFile = 0;
    int fileSize = 0;
    void *pFile = NULL;
    char szSymbol[16][64];
    
    std::vector<KRMaterial *> materials;
    
    
    Vertex3D *pVertices = NULL;
    Vector3D *pNormals = NULL;
    TexCoord *pTexCoords = NULL;
    int *pFaces = NULL;
    
    struct stat statbuf;
    fdFile = open(szPath, O_RDONLY);
    if(fdFile >= 0) {
        if(fstat(fdFile, &statbuf) >= 0) {
            if ((pFile = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdFile, 0)) == (caddr_t) -1) {
            } else {
                fileSize = statbuf.st_size;
                
                
                
                // Pass 1 - Get counts
                
                int cVertices = 0;
                int cNormals = 0;
                int cTexCoords = 0;
                int cVertexData = 0;
                
                
                cVertices = 0;
                int cFaces = 1;
                GLint cMaterialFaceStart = 1;
                
                
                
                // ---------
                
                
                char *pScan = (char *)pFile;
                char *pEnd = (char *)pFile + fileSize;
                while(pScan < pEnd) {
                    
                    // Scan through whitespace
                    while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t' || *pScan == '\r' || *pScan == '\n')) {
                        pScan++;
                    }
                    
                    if(*pScan == '#') {
                        // Line is a comment line
                        
                        // Scan to the end of the line
                        while(pScan < pEnd && *pScan != '\r' && *pScan != '\n') {
                            pScan++;
                        }
                    } else {
                        int cSymbols = 0;
                        while(pScan < pEnd && *pScan != '\n' && *pScan != '\r') {
                            
                            char *pDest = szSymbol[cSymbols++];
                            while(pScan < pEnd && *pScan != ' ' && *pScan != '\n' && *pScan != '\r') {
                                *pDest++ = *pScan++;
                            }
                            *pDest = '\0';
                            
                            // Scan through whitespace, but don't advance to next line
                            while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t')) {
                                pScan++;
                            }
                        }
                        
                        if(strcmp(szSymbol[0], "v") == 0) {
                            // Vertex (v)
                            cVertices++;
                        } else if(strcmp(szSymbol[0], "vt") == 0) {
                            // Vertex Texture UV Coordinate (vt)
                            cTexCoords++;
                        } else if(strcmp(szSymbol[0], "vn") == 0) {
                            // Vertex Normal (vn)
                            cNormals++;
                        } else if(strcmp(szSymbol[0], "f") == 0) {
                            // Face (f)
                            int cFaceVertexes = (cSymbols - 3) * 3; // 3 vertexes per triangle.  Triangles have 4 symbols.  Quads have 5 symbols and generate two triangles.
                            cVertexData += cFaceVertexes; 
                            cFaces += cFaceVertexes * 3 + 1; // Allocate space for count of vertices, Vertex Index, Texture Coordinate Index, and Normal Index
                            
                        } else if(strcmp(szSymbol[0], "usemtl") == 0) {
                            // Use Material (usemtl)
                            if(cMaterialFaceStart - cFaces > 0) {
                                cFaces++;
                                
                            }
                            materials.push_back(pMaterialManager->getMaterial(szSymbol[1]));
                        }
                        
                    }
                }
                
                
                // Pass 2 - Populate vertexes and faces
                Vertex3D *pVertices = (Vertex3D *)malloc(sizeof(Vertex3D) * cVertices);
                Vector3D *pNormals = (Vector3D *)malloc(sizeof(Vector3D) *cNormals);
                TexCoord *pTexCoords = (TexCoord *)malloc(sizeof(TexCoord) * cTexCoords);
                int *pFaces = (int *)malloc(sizeof(int *) * (cFaces + 1));
                
                
                Vertex3D *pVertice = pVertices;
                Vector3D *pNormal = pNormals;
                TexCoord *pTexCoord = pTexCoords;
                int *pFace = pFaces;
                int *pMaterialFaces = pFace++;
                *pMaterialFaces = 0;
                
                
                std::vector<KRMaterial *>::iterator material_itr = materials.begin();
                
                // --------
                
                pScan = (char *)pFile;
                while(pScan < pEnd) {
                    
                    // Scan through whitespace
                    while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t' || *pScan == '\r' || *pScan == '\n')) {
                        pScan++;
                    }
                    
                    if(*pScan == '#') {
                        // Line is a comment line
                        
                        // Scan to the end of the line
                        while(pScan < pEnd && *pScan != '\r' && *pScan != '\n') {
                            pScan++;
                        }
                    } else {
                        int cSymbols = 0;
                        while(pScan < pEnd && *pScan != '\n' && *pScan != '\r') {
                            
                            char *pDest = szSymbol[cSymbols++];
                            while(pScan < pEnd && *pScan != ' ' && *pScan != '\n' && *pScan != '\r') {
                                *pDest++ = *pScan++;
                            }
                            *pDest = '\0';
                            
                            // Scan through whitespace, but don't advance to next line
                            while(pScan < pEnd && (*pScan == ' ' || *pScan == '\t')) {
                                pScan++;
                            }
                        }
                        
                        if(strcmp(szSymbol[0], "v") == 0) {
                            // Vertex (v)
                            char *pChar = szSymbol[1];
                            pVertice -> x = strtof(pChar, &pChar);
                            pChar = szSymbol[2];
                            pVertice -> y = strtof(pChar, &pChar);
                            pChar = szSymbol[3];
                            pVertice -> z = strtof(pChar, &pChar);
                            pVertice++;
                        } else if(strcmp(szSymbol[0], "vt") == 0) {
                            // Vertex Texture UV Coordinate (vt)
                            char *pChar = szSymbol[1];
                            pTexCoord -> u = strtof(pChar, &pChar);
                            pChar = szSymbol[2];
                            pTexCoord -> v = strtof(pChar, &pChar);
                            pTexCoord++;
                        } else if(strcmp(szSymbol[0], "vn") == 0) {
                            // Vertex Normal (vn)
                            char *pChar = szSymbol[1];
                            pNormal -> x = strtof(pChar, &pChar);
                            pChar = szSymbol[2];
                            pNormal -> y = strtof(pChar, &pChar);
                            pChar = szSymbol[3];
                            pNormal -> z = strtof(pChar, &pChar);
                            pNormal++;
                        } else if(strcmp(szSymbol[0], "f") == 0) {
                            // Face (f)
                            GLint cFaceVertices = cSymbols - 1;
                            
                            *pFace++ = cFaceVertices;
                            for(int iSymbol=1; iSymbol < cSymbols; iSymbol++) {
                                char *pChar = szSymbol[iSymbol];
                                if(*pChar == '.' || (*pChar >= '0' && *pChar <= '9')) {
                                    *pFace++ = strtol(pChar, &pChar, 10) - 1; // Vertex Index
                                    
                                    if(*pChar == '/') {
                                        pChar++;
                                        if(*pChar == '/') {
                                            *pFace++ = -1;
                                        } else {
                                            *pFace++ = strtol(pChar, &pChar, 10) - 1; // Texture Coordinate Index
                                        }
                                    } else {
                                        *pFace++ = -1;
                                    }
                                    
                                    if(*pChar == '/') {
                                        pChar++;
                                        if(*pChar == '/') {
                                            *pFace++ = -1;
                                        } else {
                                            *pFace++ = strtol(pChar, &pChar, 10) - 1; // Normal Index
                                        }
                                    } else {
                                        *pFace++ = -1;
                                    }
                                    while(*pChar == '/') {
                                        pChar++;
                                        strtol(pChar, &pChar, 10);
                                    }
                                }
                            }
                            
                            
                        } else if(strcmp(szSymbol[0], "usemtl") == 0) {
                            // Use Material (usemtl)
                            if(pFace - pMaterialFaces > 1) {
                                *pMaterialFaces = pFace - pMaterialFaces - 1;
                                pMaterialFaces = pFace++;
                            }
                        }
                    }
                }
                
                
                *pMaterialFaces = pFace - pMaterialFaces - 1;
                *pFace++ = 0;
                
                
                m_pVertexData = (VertexData *)malloc(sizeof(VertexData) * cVertexData);
                
                VertexData *pData = m_pVertexData;
                
                int iVertex = 0;
                Material *pMaterial = new Material();
                pMaterial->start_vertex = iVertex;
                pMaterial->vertex_count = 0;
                if(material_itr < materials.end()) {
                    pMaterial->pMaterial = *material_itr++;
                } else {
                    pMaterial->pMaterial = NULL;
                }
                m_materials.push_back(pMaterial);
                
                
                pFace = pFaces;
                while(*pFace != 0 && iVertex < cVertexData) {
                    pMaterial->start_vertex = iVertex;
                    
                    int *pMaterialEndFace = pFace + *pFace++;
                    while(pFace < pMaterialEndFace) {
                        int cFaceVertexes = *pFace;
                        VertexData *pFirstFaceVertex = NULL;
                        VertexData *pPrevFaceVertex = NULL;
                        for(int iFaceVertex=0; iFaceVertex < cFaceVertexes; iFaceVertex++) {
                            if(iFaceVertex > 2) {
                                // There have already been 3 vertices.  Now we need to split the quad into a second triangle composed of the 1st, 3rd, and 4th vertices
                                memcpy(pData++, pFirstFaceVertex, sizeof(VertexData));
                                memcpy(pData++, pPrevFaceVertex, sizeof(VertexData));
                                iVertex+=2;
                            }
                            Vertex3D *pVertex = pVertices + pFace[iFaceVertex*3+1];
                            if(iFaceVertex==0) {
                                pFirstFaceVertex = pData;
                            }
                            pPrevFaceVertex = pData;
                            pData->vertex.x = pVertex -> x;
                            pData->vertex.y = pVertex -> y;
                            pData->vertex.z = pVertex -> z;
                            
                            if(pFace[iFaceVertex*3+2] >= 0) {
                                TexCoord *pTexCoord = pTexCoords + pFace[iFaceVertex*3+2];
                                pData->texcoord.u = pTexCoord -> u;
                                pData->texcoord.v = pTexCoord -> v;
                            } else {
                                pData->texcoord.u = 0;
                                pData->texcoord.v = 0;
                            }
                            
                            if(pFace[iFaceVertex*3+3] >= 0){
                                Vector3D *pNormal =  pNormals + pFace[iFaceVertex*3+3];
                                pData->normal.x = pNormal -> x;
                                pData->normal.y = pNormal -> y;
                                pData->normal.z = pNormal -> z;
                            } else {
                                pData->normal.x = 0;
                                pData->normal.y = 0;
                                pData->normal.z = 0;
                            }
                            
                            pData++;
                            iVertex++;
                        }
                        pFace += cFaceVertexes * 3 + 1;
                    }
                    pMaterial->vertex_count = iVertex - pMaterial->start_vertex;
                    if(*pFace != 0) {
                        pMaterial = new Material();
                        pMaterial->start_vertex = iVertex;
                        pMaterial->vertex_count = 0;
                        if(material_itr < materials.end()) {
                            pMaterial->pMaterial = *material_itr++;
                        } else {
                            pMaterial->pMaterial = NULL;
                        }
                        m_materials.push_back(pMaterial);
                    }
                }
            }
        }
    }
    
    
    m_minx = 0.0;
    m_miny = 0.0;
    m_minz = 0.0;
    m_maxx = 0.0;
    m_maxy = 0.0;
    m_maxz = 0.0;
    
    
    
    // Calculate surface normals and tangents
    // http://www.terathon.com/code/tangent.html
    // http://www.fabiensanglard.net/bumpMapping/index.php
    
    for(std::vector<Material *>::iterator itr = m_materials.begin(); itr != m_materials.end(); itr++) {
        VertexData *pStart = m_pVertexData + (*itr)->start_vertex;
        VertexData *pEnd = pStart + (*itr)->vertex_count;
        for(VertexData *pVertex = pStart; pVertex < pEnd; pVertex+=3) {
            if(pVertex->vertex.x < m_minx) m_minx = pVertex->vertex.x;
            if(pVertex->vertex.x > m_maxx) m_maxx = pVertex->vertex.x;
            if(pVertex->vertex.y < m_miny) m_miny = pVertex->vertex.y;
            if(pVertex->vertex.y > m_maxy) m_maxy = pVertex->vertex.y;
            if(pVertex->vertex.z < m_minz) m_minz = pVertex->vertex.z;
            if(pVertex->vertex.z > m_maxz) m_maxz = pVertex->vertex.z; 
        }
        
        
        for(VertexData *pVertex = pStart; pVertex < pEnd; pVertex+=3) {
            Vector3 p1(pVertex[0].vertex.x, pVertex[0].vertex.y, pVertex[0].vertex.z);
            Vector3 p2(pVertex[1].vertex.x, pVertex[1].vertex.y, pVertex[1].vertex.z);
            Vector3 p3(pVertex[2].vertex.x, pVertex[2].vertex.y, pVertex[2].vertex.z);
            Vector3 v1 = p2 - p1;
            Vector3 v2 = p3 - p1;
            
            // -- Calculate normal --
            if(pVertex->normal.x == 0 && pVertex->normal.y == 0 && pVertex->normal.z == 0) {
                
                
                Vector3 normal = v1.cross( v2 );
                
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
            
            // -- Calculate tangent --
            TexCoord st1; // = pVertex[2].texcoord;
            TexCoord st2; // = pVertex[1].texcoord;
            st1.u = pVertex[1].texcoord.u - pVertex[0].texcoord.u;
            st1.v = pVertex[1].texcoord.v - pVertex[0].texcoord.v;
            st2.u = pVertex[2].texcoord.u - pVertex[0].texcoord.u;
            st2.v = pVertex[2].texcoord.v - pVertex[0].texcoord.v;
            double coef = 1/ (st1.u * st2.v - st2.u * st1.v);
            
            pVertex[0].tangent.x = coef * ((v1.x * st2.v)  + (v2.x * -st1.v));
            pVertex[0].tangent.y = coef * ((v1.y * st2.v)  + (v2.y * -st1.v));
            pVertex[0].tangent.z = coef * ((v1.z * st2.v)  + (v2.z * -st1.v));
            
            Vector3 tangent(
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
    
    
    if(pFile != NULL) {
        munmap(pFile, fileSize);
    }
    
    if(fdFile != 0) {
        close(fdFile);
    }
    
    if(pVertices) {
        free(pVertices);
    }
    if(pNormals) {
        free(pNormals);
    }
    if(pTexCoords) {
        free(pTexCoords);
    }
    if(pFaces) {
        free(pFaces);
    }
}

KRModel::~KRModel() {
    
    if(m_pPackFile != NULL) {
        munmap(m_pPackFile, m_iPackFileSize);
    }
    if(m_fdPackFile) {
        close(m_fdPackFile);
    } else {
        // If we didn't load a packed file, then the data was calculated at run time and malloc'ed
        if(m_pVertexData != NULL) {
            free(m_pVertexData);
        }
    }
    
    if(m_pBuffers != NULL) {
        glDeleteBuffers(m_cBuffers, m_pBuffers);
        delete m_pBuffers;
    }
}

void KRModel::render(KRCamera *pCamera, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &mvpMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers) {
    for(std::vector<Material *>::iterator itr = m_materials.begin(); itr != m_materials.end(); itr++) {

        KRMaterial *pMaterial = (*itr)->pMaterial;
        
        if(pMaterial != NULL) {
            if(!bRenderShadowMap) {
                pMaterial->bind(pCamera, mvpMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers);
            }

            if(!bRenderShadowMap || !pMaterial->isTransparent()) {
                // Exclude transparent and semi-transparent meshes from shadow maps
            
                int iVertex = (*itr)->start_vertex;
                int iBuffer = iVertex / MAX_VBO_SIZE;
                iVertex = iVertex % MAX_VBO_SIZE;
                int cVertexes = (*itr)->vertex_count;
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
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
    }
    
}


GLfloat KRModel::getMaxDimension() {
    GLfloat m = 0.0;
    if(m_maxx - m_minx > m) m = m_maxx - m_minx;
    if(m_maxy - m_miny > m) m = m_maxy - m_miny;
    if(m_maxz - m_minz > m) m = m_maxz - m_minz;
    return m;
}

GLfloat KRModel::getMinX() {
    return m_minx;
}
GLfloat KRModel::getMaxX() {
    return m_maxx;
}
GLfloat KRModel::getMinY() {
    return m_miny;
}
GLfloat KRModel::getMaxY() {
    return m_maxy;
}
GLfloat KRModel::getMinZ() {
    return m_minz;
}
GLfloat KRModel::getMaxZ() {
    return m_maxz;
}
