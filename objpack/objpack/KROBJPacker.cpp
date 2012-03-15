//
//  objpacker.cpp
//  objpack
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
#include <vector.h>

#include "KROBJPacker.h"
#import <KREngine_osx/KRVector3.h>


KROBJPacker::KROBJPacker() {
    
}

KROBJPacker::~KROBJPacker() {
    
}

void KROBJPacker::pack(const char *szPath) {
    int fdFile = 0;
    int fileSize = 0;
    void *pFile = NULL;
    char szSymbol[16][64];
    
    std::vector<std::string> materials;
    
    Vertex3D *pVertices = NULL;
    KRVector3D *pNormals = NULL;
    TexCoord *pTexCoords = NULL;
    int *pFaces = NULL;
    
    vector<pack_material *> m_materials;
    VertexData *m_pVertexData = NULL;
    
    pack_header header;
    strcpy(header.szTag, "KROBJPACK1.0   ");
        
    struct stat statbuf;
    fdFile = open(szPath, O_RDONLY);
    if(fdFile >= 0) {
        if(fstat(fdFile, &statbuf) >= 0) {
            if ((pFile = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdFile, 0)) == (caddr_t) -1) {
            } else {
                fileSize = statbuf.st_size;
                
                // Pass 1 - Get counts
                cout << "  Pass 1 - Getting Counts\n";
                
                int cVertices = 0;
                int cNormals = 0;
                int cTexCoords = 0;
                int cVertexData = 0;
                
                
                cVertices = 0;
                int cFaces = 1;
                int cMaterialFaceStart = 1;
                
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
                            materials.push_back(std::string(szSymbol[1]));
                        }
                        
                    }
                }

                
                // Pass 2 - Populate vertexes and faces
                cout << "  Pass 2 - Populate vertexes and faces\n";

                Vertex3D *pVertices = (Vertex3D *)malloc(sizeof(Vertex3D) * cVertices);
                KRVector3D *pNormals = (KRVector3D *)malloc(sizeof(KRVector3D) *cNormals);
                TexCoord *pTexCoords = (TexCoord *)malloc(sizeof(TexCoord) * cTexCoords);
                int *pFaces = (int *)malloc(sizeof(int *) * (cFaces + 1));
                
                
                Vertex3D *pVertice = pVertices;
                KRVector3D *pNormal = pNormals;
                TexCoord *pTexCoord = pTexCoords;
                int *pFace = pFaces;
                int *pMaterialFaces = pFace++;
                *pMaterialFaces = 0;
                
                
                std::vector<std::string>::iterator material_itr = materials.begin();
                
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
                            int cFaceVertices = cSymbols - 1;
                            
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
                pack_material *pMaterial = new pack_material();
                pMaterial->start_vertex = iVertex;
                pMaterial->vertex_count = 0;
                memset(pMaterial->szName, 64, 0);
                if(material_itr < materials.end()) {
                    strncpy(pMaterial->szName, (*material_itr++).c_str(), 64);
                }
                m_materials.push_back(pMaterial);
                
                
                pFace = pFaces;
                while(*pFace != 0 && iVertex < cVertexData) {
                    pMaterial->start_vertex = iVertex;
                    
                    int *pMaterialEndFace = pFace + *pFace++;
                    while(pFace < pMaterialEndFace  && iVertex < cVertexData) {
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
                                KRVector3D *pNormal =  pNormals + pFace[iFaceVertex*3+3];
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
                        pMaterial = new pack_material();
                        pMaterial->start_vertex = iVertex;
                        pMaterial->vertex_count = 0;
                        memset(pMaterial->szName, 64, 0);
                        
                        if(material_itr < materials.end()) {
                            strncpy(pMaterial->szName, (*material_itr++).c_str(), 64);
                        }
                        m_materials.push_back(pMaterial);
                    }
                }
                
                
                header.minx = 0.0;
                header.miny = 0.0;
                header.minz = 0.0;
                header.maxx = 0.0;
                header.maxy = 0.0;
                header.maxz = 0.0;
                
                // Calculate surface normals and tangents
                cout << "  Pass 3 - Calculate surface normals and tangents\n";
                
                for(std::vector<pack_material *>::iterator itr = m_materials.begin(); itr != m_materials.end(); itr++) {
                    VertexData *pStart = m_pVertexData + (*itr)->start_vertex;
                    VertexData *pEnd = pStart + (*itr)->vertex_count;
                    for(VertexData *pVertex = pStart; pVertex < pEnd; pVertex+=3) {
                        if(pVertex->vertex.x < header.minx) header.minx = pVertex->vertex.x;
                        if(pVertex->vertex.x > header.maxx) header.maxx = pVertex->vertex.x;
                        if(pVertex->vertex.y < header.miny) header.miny = pVertex->vertex.y;
                        if(pVertex->vertex.y > header.maxy) header.maxy = pVertex->vertex.y;
                        if(pVertex->vertex.z < header.minz) header.minz = pVertex->vertex.z;
                        if(pVertex->vertex.z > header.maxz) header.maxz = pVertex->vertex.z; 
                    }
                    
                    
                    for(VertexData *pVertex = pStart; pVertex < pEnd; pVertex+=3) {
                        KRVector3 p1(pVertex[0].vertex.x, pVertex[0].vertex.y, pVertex[0].vertex.z);
                        KRVector3 p2(pVertex[1].vertex.x, pVertex[1].vertex.y, pVertex[1].vertex.z);
                        KRVector3 p3(pVertex[2].vertex.x, pVertex[2].vertex.y, pVertex[2].vertex.z);
                        KRVector3 v1 = p2 - p1;
                        KRVector3 v2 = p3 - p1;
                        
                        // -- Calculate normal --
                        if(pVertex->normal.x == 0 && pVertex->normal.y == 0 && pVertex->normal.z == 0) {
                            
                            
                            KRVector3 normal = v1.cross( v2 );
                            
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
                
                // Write output file
                std::string out_file_name = szPath;
                out_file_name.append(".pack");
                cout << "  Writing obj.pack file:" << out_file_name << " ... \n";
                FILE *out_file = fopen(out_file_name.c_str(), "w");
                
                header.material_count = 0;
                for(std::vector<pack_material *>::iterator itr = m_materials.begin(); itr != m_materials.end(); itr++) {
                    pack_material *pMaterial = (*itr);
                    if(pMaterial->vertex_count) { // Skip materials with no vertices
                        header.material_count++;
                    }
                }
                
                header.vertex_count = cVertexData;
                
                fwrite(&header, sizeof(header), 1, out_file);
                
                for(std::vector<pack_material *>::iterator itr = m_materials.begin(); itr != m_materials.end(); itr++) {
                    pack_material *pMaterial = (*itr);
                    if(pMaterial->vertex_count) { // Skip materials with no vertices
                        fwrite(pMaterial, sizeof(pack_material), 1, out_file);
                        cout << "    " << pMaterial->szName << ": " << pMaterial->vertex_count << " vertices\n";
                    }
                }
                
                
                fwrite(m_pVertexData, sizeof(VertexData), cVertexData, out_file);
                
                fclose(out_file);

            }
        }
    }
    
    if(pFile != NULL) {
        munmap(pFile, fileSize);
    }
    
    if(fdFile != 0) {
        close(fdFile);
    }
    
    if(m_pVertexData) {
        free(m_pVertexData);
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
