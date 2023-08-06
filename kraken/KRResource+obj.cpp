//
//  KRResource+obj.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#include "KRResource.h"
#include "KRMesh.h"

#include "mimir.h"

using namespace mimir;

KRMesh* KRResource::LoadObj(KRContext& context, const std::string& path)
{
  KRMesh* new_mesh = new KRMesh(context, util::GetFileBase(path));

  KRMesh::mesh_info mi;

  std::vector<std::string> material_names_t;

  Block data;

  char szSymbol[500][256];

  int* pFaces = NULL;

  vector<KRMesh::pack_material*> m_materials;

  if (data.load(path)) {
    //  -----=====----- Get counts -----=====----- 

    int cVertexData = 0;


    int cFaces = 1;
    int cMaterialFaceStart = 1;

    char* pScan = (char*)data.getStart();
    char* pEnd = (char*)data.getEnd();
    while (pScan < pEnd) {

      // Scan through whitespace
      while (pScan < pEnd && (*pScan == ' ' || *pScan == '\t' || *pScan == '\r' || *pScan == '\n')) {
        pScan++;
      }

      if (*pScan == '#') {
        // Line is a comment line

        // Scan to the end of the line
        while (pScan < pEnd && *pScan != '\r' && *pScan != '\n') {
          pScan++;
        }
      } else {
        int cSymbols = 0;
        while (pScan < pEnd && *pScan != '\n' && *pScan != '\r') {

          char* pDest = szSymbol[cSymbols++];
          while (pScan < pEnd && *pScan != ' ' && *pScan != '\n' && *pScan != '\r') {
            *pDest++ = *pScan++;
          }
          *pDest = '\0';

          // Scan through whitespace, but don't advance to next line
          while (pScan < pEnd && (*pScan == ' ' || *pScan == '\t')) {
            pScan++;
          }
        }

        if (strcmp(szSymbol[0], "v") == 0) {
          // Vertex (v)
        } else if (strcmp(szSymbol[0], "vt") == 0) {
          // Vertex Texture UV Coordinate (vt)
        } else if (strcmp(szSymbol[0], "vn") == 0) {
          // Vertex Normal (vn)
        } else if (strcmp(szSymbol[0], "f") == 0) {
          // Face (f)
          int cFaceVertexes = (cSymbols - 3) * 3; // 3 vertexes per triangle.  Triangles have 4 symbols.  Quads have 5 symbols and generate two triangles.
          cVertexData += cFaceVertexes;
          cFaces += cFaceVertexes * 3 + 1; // Allocate space for count of vertices, Vertex Index, Texture Coordinate Index, and Normal Index

        } else if (strcmp(szSymbol[0], "usemtl") == 0) {
          // Use Material (usemtl)
          if (cMaterialFaceStart - cFaces > 0) {
            cFaces++;

          }
          material_names_t.push_back(std::string(szSymbol[1]));
        }

      }
    }


    //  -----=====-----  Populate vertexes and faces -----=====----- 

    int* pFaces = (int*)malloc(sizeof(int) * (cFaces + 1));
    assert(pFaces != NULL);

    std::vector<Vector3> indexed_vertices;
    std::vector<Vector2> indexed_uva;
    std::vector<Vector3> indexed_normals;

    int* pFace = pFaces;
    int* pMaterialFaces = pFace++;
    *pMaterialFaces = 0;




    // --------

    pScan = (char*)data.getStart();
    while (pScan < pEnd) {

      // Scan through whitespace
      while (pScan < pEnd && (*pScan == ' ' || *pScan == '\t' || *pScan == '\r' || *pScan == '\n')) {
        pScan++;
      }

      if (*pScan == '#') {
        // Line is a comment line

        // Scan to the end of the line
        while (pScan < pEnd && *pScan != '\r' && *pScan != '\n') {
          pScan++;
        }
      } else {
        int cSymbols = 0;
        while (pScan < pEnd && *pScan != '\n' && *pScan != '\r') {

          char* pDest = szSymbol[cSymbols++];
          while (pScan < pEnd && *pScan != ' ' && *pScan != '\n' && *pScan != '\r') {
            *pDest++ = *pScan++;
          }
          *pDest = '\0';

          // Scan through whitespace, but don't advance to next line
          while (pScan < pEnd && (*pScan == ' ' || *pScan == '\t')) {
            pScan++;
          }
        }

        if (strcmp(szSymbol[0], "v") == 0) {
          // Vertex (v)
          float x, y, z;
          char* pChar = szSymbol[1];
          x = strtof(pChar, &pChar);
          pChar = szSymbol[2];
          y = strtof(pChar, &pChar);
          pChar = szSymbol[3];
          z = strtof(pChar, &pChar);
          indexed_vertices.push_back(Vector3::Create(x, y, z));
        } else if (strcmp(szSymbol[0], "vt") == 0) {
          // Vertex Texture UV Coordinate (vt)
          char* pChar = szSymbol[1];
          float u, v;

          u = strtof(pChar, &pChar);
          pChar = szSymbol[2];
          v = strtof(pChar, &pChar);
          indexed_uva.push_back(Vector2::Create(u, v));
        } else if (strcmp(szSymbol[0], "vn") == 0) {
          // Vertex Normal (vn)
          float x, y, z;
          char* pChar = szSymbol[1];
          x = strtof(pChar, &pChar);
          pChar = szSymbol[2];
          y = strtof(pChar, &pChar);
          pChar = szSymbol[3];
          z = strtof(pChar, &pChar);
          indexed_normals.push_back(Vector3::Create(x, y, z));
        } else if (strcmp(szSymbol[0], "f") == 0) {
          // Face (f)
          int cFaceVertices = cSymbols - 1;

          *pFace++ = cFaceVertices;
          for (int iSymbol = 1; iSymbol < cSymbols; iSymbol++) {
            char* pChar = szSymbol[iSymbol];
            if (*pChar == '.' || (*pChar >= '0' && *pChar <= '9')) {
              *pFace++ = strtol(pChar, &pChar, 10) - 1; // Vertex Index

              if (*pChar == '/') {
                pChar++;
                if (*pChar == '/') {
                  *pFace++ = -1;
                } else {
                  *pFace++ = strtol(pChar, &pChar, 10) - 1; // Texture Coordinate Index
                }
              } else {
                *pFace++ = -1;
              }

              if (*pChar == '/') {
                pChar++;
                if (*pChar == '/') {
                  *pFace++ = -1;
                } else {
                  *pFace++ = strtol(pChar, &pChar, 10) - 1; // Normal Index
                }
              } else {
                *pFace++ = -1;
              }
              while (*pChar == '/') {
                pChar++;
                strtol(pChar, &pChar, 10);
              }
            }
          }


        } else if (strcmp(szSymbol[0], "usemtl") == 0) {
          // Use Material (usemtl)
          if (pFace - pMaterialFaces > 1) {
            *pMaterialFaces = (int)(pFace - pMaterialFaces - 1);
            pMaterialFaces = pFace++;
          }
        }
      }
    }


    *pMaterialFaces = (int)(pFace - pMaterialFaces - 1);
    *pFace++ = 0;


    int iVertex = 0;


    std::vector<std::string>::iterator material_itr = material_names_t.begin();
    KRMesh::pack_material* pMaterial = new KRMesh::pack_material();
    pMaterial->start_vertex = iVertex;
    pMaterial->vertex_count = 0;
    memset(pMaterial->szName, 0, 256);
    if (material_itr < material_names_t.end()) {
      strncpy(pMaterial->szName, (*material_itr++).c_str(), 256);
    }
    m_materials.push_back(pMaterial);


    pFace = pFaces;
    while (*pFace != 0 && iVertex < cVertexData) {
      pMaterial->start_vertex = iVertex;

      int* pMaterialEndFace = pFace + *pFace;
      ++pFace;
      while (pFace < pMaterialEndFace && iVertex < cVertexData) {
        int cFaceVertexes = *pFace;
        Vector3 firstFaceVertex;
        Vector3 prevFaceVertex;
        Vector3 firstFaceNormal;
        Vector3 prevFaceNormal;
        Vector2 firstFaceUva;
        Vector2 prevFaceUva;
        for (int iFaceVertex = 0; iFaceVertex < cFaceVertexes; iFaceVertex++) {
          if (iFaceVertex > 2) {
            // There have already been 3 vertices.  Now we need to split the quad into a second triangle composed of the 1st, 3rd, and 4th vertices
            iVertex += 2;

            mi.vertices.push_back(firstFaceVertex);
            mi.uva.push_back(firstFaceUva);
            mi.normals.push_back(firstFaceNormal);

            mi.vertices.push_back(prevFaceVertex);
            mi.uva.push_back(prevFaceUva);
            mi.normals.push_back(prevFaceNormal);
          }
          Vector3 vertex = indexed_vertices[pFace[iFaceVertex * 3 + 1]];
          Vector2 new_uva;
          if (pFace[iFaceVertex * 3 + 2] >= 0) {
            new_uva = indexed_uva[pFace[iFaceVertex * 3 + 2]];
          }
          Vector3 normal;
          if (pFace[iFaceVertex * 3 + 3] >= 0) {
            Vector3 normal = indexed_normals[pFace[iFaceVertex * 3 + 3]];
          }

          mi.vertices.push_back(vertex);
          mi.uva.push_back(new_uva);
          mi.normals.push_back(normal);

          if (iFaceVertex == 0) {
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
      if (*pFace != 0) {
        pMaterial = new KRMesh::pack_material();
        pMaterial->start_vertex = iVertex;
        pMaterial->vertex_count = 0;
        memset(pMaterial->szName, 0, 256);

        if (material_itr < material_names_t.end()) {
          strncpy(pMaterial->szName, (*material_itr++).c_str(), 256);
        }
        m_materials.push_back(pMaterial);
      }
    }

    for (int iMaterial = 0; iMaterial < m_materials.size(); iMaterial++) {
      KRMesh::pack_material* pNewMaterial = m_materials[iMaterial];
      if (pNewMaterial->vertex_count > 0) {
        mi.material_names.push_back(std::string(pNewMaterial->szName));
        mi.submesh_starts.push_back(pNewMaterial->start_vertex);
        mi.submesh_lengths.push_back(pNewMaterial->vertex_count);
      }
      delete pNewMaterial;
    }

    // TODO: Bones not yet supported for OBJ
//        std::vector<std::string> bone_names;
//        std::vector<Matrix4> bone_bind_poses;
//        std::vector<std::vector<int> > bone_indexes;
//        std::vector<std::vector<float> > bone_weights;
//        
//        std::vector<__uint16_t> vertex_indexes;
//        std::vector<std::pair<int, int> > vertex_index_bases;

    mi.format = ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;
    new_mesh->LoadData(mi, true, false);
  }

  if (pFaces) {
    free(pFaces);
  }

  return new_mesh;
}
