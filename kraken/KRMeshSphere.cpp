//
//  KRMeshSphere.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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

#include "KRMeshSphere.h"

using namespace hydra;

KRMeshSphere::KRMeshSphere(KRContext& context) : KRMesh(context, "__sphere")
{
  m_constant = true;

  KRMesh::mesh_info mi;

  // Create a triangular facet approximation to a sphere
  // Based on algorithm from Paul Bourke: http://paulbourke.net/miscellaneous/sphere_cylinder/

  int iterations = 3;
  int facet_count = (int)(pow(4, iterations) * 8.0f);

  std::vector<Triangle3> f = std::vector<Triangle3>(facet_count);

  int i, it;
  float a;
  Vector3 p[6] = {
      Vector3::Create(0,0,1),
      Vector3::Create(0,0,-1),
      Vector3::Create(-1,-1,0),
      Vector3::Create(1,-1,0),
      Vector3::Create(1,1,0),
      Vector3::Create(-1,1,0)
  };

  Vector3 pa, pb, pc;
  int nt = 0, ntold;

  /* Create the level 0 object */
  a = 1.0f / sqrtf(2.0f);
  for (i = 0; i < 6; i++) {
    p[i].x *= a;
    p[i].y *= a;
  }
  f[0][0] = p[0]; f[0][1] = p[3]; f[0][2] = p[4];
  f[1][0] = p[0]; f[1][1] = p[4]; f[1][2] = p[5];
  f[2][0] = p[0]; f[2][1] = p[5]; f[2][2] = p[2];
  f[3][0] = p[0]; f[3][1] = p[2]; f[3][2] = p[3];
  f[4][0] = p[1]; f[4][1] = p[4]; f[4][2] = p[3];
  f[5][0] = p[1]; f[5][1] = p[5]; f[5][2] = p[4];
  f[6][0] = p[1]; f[6][1] = p[2]; f[6][2] = p[5];
  f[7][0] = p[1]; f[7][1] = p[3]; f[7][2] = p[2];
  nt = 8;

  /* Bisect each edge and move to the surface of a unit sphere */
  for (it = 0; it < iterations; it++) {
    ntold = nt;
    for (i = 0; i < ntold; i++) {
      pa.x = (f[i][0].x + f[i][1].x) / 2;
      pa.y = (f[i][0].y + f[i][1].y) / 2;
      pa.z = (f[i][0].z + f[i][1].z) / 2;
      pb.x = (f[i][1].x + f[i][2].x) / 2;
      pb.y = (f[i][1].y + f[i][2].y) / 2;
      pb.z = (f[i][1].z + f[i][2].z) / 2;
      pc.x = (f[i][2].x + f[i][0].x) / 2;
      pc.y = (f[i][2].y + f[i][0].y) / 2;
      pc.z = (f[i][2].z + f[i][0].z) / 2;
      pa.normalize();
      pb.normalize();
      pc.normalize();
      f[nt][0] = f[i][0]; f[nt][1] = pa; f[nt][2] = pc; nt++;
      f[nt][0] = pa; f[nt][1] = f[i][1]; f[nt][2] = pb; nt++;
      f[nt][0] = pb; f[nt][1] = f[i][2]; f[nt][2] = pc; nt++;
      f[i][0] = pa;
      f[i][1] = pb;
      f[i][2] = pc;
    }
  }

  for (int facet_index = 0; facet_index < facet_count; facet_index++) {
    mi.vertices.push_back(f[facet_index][0]);
    mi.vertices.push_back(f[facet_index][1]);
    mi.vertices.push_back(f[facet_index][2]);
  }

  mi.submesh_starts.push_back(0);
  mi.submesh_lengths.push_back((int)mi.vertices.size());
  mi.material_names.push_back("");


  mi.format = ModelFormat::KRENGINE_MODEL_FORMAT_TRIANGLES;

  // Generate normals pointing away from center of sphere.
  for (int vertex_index = 0; vertex_index < mi.vertices.size(); vertex_index++) {
    mi.normals.push_back(Vector3::Normalize(mi.vertices[vertex_index] - Vector3::Zero()));
  }

  LoadData(mi, true, true);
}

KRMeshSphere::~KRMeshSphere()
{

}
