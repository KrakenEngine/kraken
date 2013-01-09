//
//  KRResource_obj.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "KRResource.h"
#include "KRMesh.h"

std::vector<KRResource *> KRResource::LoadObj(KRContext &context, const std::string& path)
{
    std::vector<KRResource *> resources;
    
    KRMesh *new_mesh = new KRMesh(context, KRResource::GetFileBase(path));
    resources.push_back(new_mesh);
    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector2> uvb;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_lengths;
    std::vector<int> submesh_starts;
    std::vector<std::string> material_names;
    
    std::vector<std::string> material_names_t;
    
    KRDataBlock data;
    
    char szSymbol[500][256];
    
    int *pFaces = NULL;
    
    vector<KRMesh::pack_material *> m_materials;
    
    if(data.load(path)) {
        //  -----=====----- Get counts -----=====----- 
        
        int cVertexData = 0;
        
        
        int cFaces = 1;
        int cMaterialFaceStart = 1;
        
        char *pScan = (char *)data.getStart();
        char *pEnd = (char *)data.getEnd();
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
                } else if(strcmp(szSymbol[0], "vt") == 0) {
                    // Vertex Texture UV Coordinate (vt)
                } else if(strcmp(szSymbol[0], "vn") == 0) {
                    // Vertex Normal (vn)
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
                    material_names_t.push_back(std::string(szSymbol[1]));
                }
                
            }
        }
        
        
        //  -----=====-----  Populate vertexes and faces -----=====----- 
        
        int *pFaces = (int *)malloc(sizeof(int *) * (cFaces + 1));
        assert(pFaces != NULL);
        
        std::vector<KRVector3> indexed_vertices;
        std::vector<KRVector2> indexed_uva;
        std::vector<KRVector3> indexed_normals;
        
        int *pFace = pFaces;
        int *pMaterialFaces = pFace++;
        *pMaterialFaces = 0;
        
        
        
        
        // --------
        
        pScan = (char *)data.getStart();
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
                    float x, y, z;
                    char *pChar = szSymbol[1];
                    x = strtof(pChar, &pChar);
                    pChar = szSymbol[2];
                    y = strtof(pChar, &pChar);
                    pChar = szSymbol[3];
                    z = strtof(pChar, &pChar);
                    indexed_vertices.push_back(KRVector3(x,y,z));
                } else if(strcmp(szSymbol[0], "vt") == 0) {
                    // Vertex Texture UV Coordinate (vt)
                    char *pChar = szSymbol[1];
                    float u,v;
                    
                    u = strtof(pChar, &pChar);
                    pChar = szSymbol[2];
                    v = strtof(pChar, &pChar);                            
                    indexed_uva.push_back(KRVector2(u,v));
                } else if(strcmp(szSymbol[0], "vn") == 0) {
                    // Vertex Normal (vn)
                    float x,y,z;
                    char *pChar = szSymbol[1];
                    x = strtof(pChar, &pChar);
                    pChar = szSymbol[2];
                    y = strtof(pChar, &pChar);
                    pChar = szSymbol[3];
                    z = strtof(pChar, &pChar);
                    indexed_normals.push_back(KRVector3(x,y,z));
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
        
        
        int iVertex = 0;
        
        
        std::vector<std::string>::iterator material_itr = material_names_t.begin();
        KRMesh::pack_material *pMaterial = new KRMesh::pack_material();
        pMaterial->start_vertex = iVertex;
        pMaterial->vertex_count = 0;
        memset(pMaterial->szName, 256, 0);
        if(material_itr < material_names_t.end()) {
            strncpy(pMaterial->szName, (*material_itr++).c_str(), 256);
        }
        m_materials.push_back(pMaterial);
        
        
        pFace = pFaces;
        while(*pFace != 0 && iVertex < cVertexData) {
            pMaterial->start_vertex = iVertex;
            
            int *pMaterialEndFace = pFace + *pFace++;
            while(pFace < pMaterialEndFace  && iVertex < cVertexData) {
                int cFaceVertexes = *pFace;
                KRVector3 firstFaceVertex;
                KRVector3 prevFaceVertex;
                KRVector3 firstFaceNormal;
                KRVector3 prevFaceNormal;
                KRVector2 firstFaceUva;
                KRVector2 prevFaceUva;
                for(int iFaceVertex=0; iFaceVertex < cFaceVertexes; iFaceVertex++) {
                    if(iFaceVertex > 2) {
                        // There have already been 3 vertices.  Now we need to split the quad into a second triangle composed of the 1st, 3rd, and 4th vertices
                        iVertex+=2;
                        
                        vertices.push_back(firstFaceVertex);
                        uva.push_back(firstFaceUva);
                        normals.push_back(firstFaceNormal);
                        
                        vertices.push_back(prevFaceVertex);
                        uva.push_back(prevFaceUva);
                        normals.push_back(prevFaceNormal);
                    }
                    KRVector3 vertex = indexed_vertices[pFace[iFaceVertex*3+1]];
                    KRVector2 new_uva;
                    if(pFace[iFaceVertex*3+2] >= 0) {
                        new_uva = indexed_uva[pFace[iFaceVertex*3+2]];
                    }
                    KRVector3 normal;
                    if(pFace[iFaceVertex*3+3] >= 0){
                        KRVector3 normal = indexed_normals[pFace[iFaceVertex*3+3]];
                    }
                    
                    vertices.push_back(vertex);
                    uva.push_back(new_uva);
                    normals.push_back(normal);
                    
                    if(iFaceVertex==0) {
                        firstFaceVertex = vertex;
                        firstFaceUva = new_uva;
                        firstFaceNormal = normal;
                    }
                    prevFaceVertex = vertex;
                    prevFaceUva = new_uva;
                    prevFaceNormal = normal;
                    
                    iVertex++;
                }
                pFace += cFaceVertexes * 3 + 1;
            }
            pMaterial->vertex_count = iVertex - pMaterial->start_vertex;
            if(*pFace != 0) {
                pMaterial = new KRMesh::pack_material();
                pMaterial->start_vertex = iVertex;
                pMaterial->vertex_count = 0;
                memset(pMaterial->szName, 256, 0);
                
                if(material_itr < material_names_t.end()) {
                    strncpy(pMaterial->szName, (*material_itr++).c_str(), 256);
                }
                m_materials.push_back(pMaterial);
            }
        }
        
        for(int iMaterial=0; iMaterial < m_materials.size(); iMaterial++) {
            KRMesh::pack_material *pNewMaterial = m_materials[iMaterial];
            if(pNewMaterial->vertex_count > 0) {
                material_names.push_back(std::string(pNewMaterial->szName));
                submesh_starts.push_back(pNewMaterial->start_vertex);
                submesh_lengths.push_back(pNewMaterial->vertex_count);
            }
            delete pNewMaterial;
        }
        
        // TODO: Bones not yet supported for OBJ
        std::vector<std::string> bone_names;
        std::vector<std::vector<int> > bone_indexes;
        std::vector<std::vector<float> > bone_weights;
        
        new_mesh->LoadData(vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names, bone_names, bone_indexes, bone_weights, KRMesh::KRENGINE_MODEL_FORMAT_TRIANGLES);
    }
    
    if(pFaces) {
        free(pFaces);
    }
    
    return resources;
}