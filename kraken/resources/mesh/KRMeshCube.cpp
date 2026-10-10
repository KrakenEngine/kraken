//
//  KRMeshCube.cpp
//  Kraken Engine
//
//  Copyright 2026 Kearwood Gilbert. All rights reserved.
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

#include "KRMeshCube.h"

using namespace hydra;

KRMeshCube::KRMeshCube(KRContext& context) : KRMesh(context, "__cube")
{
  m_constant = true;

  KRMesh::PrimitiveDesc pd{};

  /*
  // Cube without normals
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, -1.0));


  pd.submesh_starts.push_back(0);
  pd.submesh_lengths.push_back((int)pd.vertices.size());
  pd.material_names.push_back("__white");
  pd.format = Topology::TriangleStrips;
  */

  // Cube with normals
  int b = 0;
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, 1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, 1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, 1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, 1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, 1.0));
  
  pd.indexes.push_back(b+0);
  pd.indexes.push_back(b+1);
  pd.indexes.push_back(b+2);
  pd.indexes.push_back(b+2);
  pd.indexes.push_back(b+1);
  pd.indexes.push_back(b+3);
  b += 4;

  pd.vertices.push_back(Vector3::Create(1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, 1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, 1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, 1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, 1.0, 0.0));

  pd.indexes.push_back(b + 3);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 0);
  b += 4;

  pd.vertices.push_back(Vector3::Create(1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, -1.0));
  pd.normals.push_back(Vector3::Create(1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(1.0, 0.0, 0.0));

  pd.indexes.push_back(b + 0);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 3);
  b += 4;

  pd.vertices.push_back(Vector3::Create(1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, 0.0, -1.0));

  pd.indexes.push_back(b + 3);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 0);
  b += 4;

  pd.vertices.push_back(Vector3::Create(1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(1.0, -1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, -1.0));
  pd.normals.push_back(Vector3::Create(0.0, -1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, -1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, -1.0, 0.0));
  pd.normals.push_back(Vector3::Create(0.0, -1.0, 0.0));

  pd.indexes.push_back(b + 0);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 3);
  b += 4;

  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, 1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, 1.0, -1.0));
  pd.vertices.push_back(Vector3::Create(-1.0, -1.0, -1.0));
  pd.normals.push_back(Vector3::Create(-1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(-1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(-1.0, 0.0, 0.0));
  pd.normals.push_back(Vector3::Create(-1.0, 0.0, 0.0));

  pd.indexes.push_back(b + 3);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 2);
  pd.indexes.push_back(b + 1);
  pd.indexes.push_back(b + 0);
  b += 4;


  pd.materialName = "__white";
  pd.format = Topology::Triangles;

  MeshDesc md{};
  md.primitives.push_back(pd);
  LoadDesc(md, true, true);
}

KRMeshCube::~KRMeshCube()
{

}
