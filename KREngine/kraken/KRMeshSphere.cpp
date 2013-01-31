//
//  KRMeshSphere.cpp
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

#include "KRMeshSphere.h"


KRMeshSphere::KRMeshSphere(KRContext &context) : KRMesh(context, "__sphere")
{
    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector2> uvb;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_starts;
    std::vector<int> submesh_lengths;
    std::vector<std::string> material_names;
    std::vector<std::string> bone_names;
    std::vector<std::vector<int> > bone_indexes;
    std::vector<std::vector<float> > bone_weights;
    
    
    
    // Create a triangular facet approximation to a sphere
    // Based on algorithm from Paul Bourke: http://paulbourke.net/miscellaneous/sphere_cylinder/
    
    int iterations = 3;
    int facet_count = pow(4, iterations) * 8;

    class Facet3 {
    public:
        Facet3() {
            
        }
        ~Facet3() {
            
        }
        KRVector3 p1;
        KRVector3 p2;
        KRVector3 p3;
    };
    
    std::vector<Facet3> f = std::vector<Facet3>(facet_count);
    
    int i,it;
    float a;
    KRVector3 p[6] = {
        KRVector3(0,0,1),
        KRVector3(0,0,-1),
        KRVector3(-1,-1,0),
        KRVector3(1,-1,0),
        KRVector3(1,1,0),
        KRVector3(-1,1,0)
    };
    
    KRVector3 pa,pb,pc;
    int nt = 0,ntold;
    
    /* Create the level 0 object */
    a = 1 / sqrt(2.0);
    for (i=0;i<6;i++) {
        p[i].x *= a;
        p[i].y *= a;
    }
    f[0].p1 = p[0]; f[0].p2 = p[3]; f[0].p3 = p[4];
    f[1].p1 = p[0]; f[1].p2 = p[4]; f[1].p3 = p[5];
    f[2].p1 = p[0]; f[2].p2 = p[5]; f[2].p3 = p[2];
    f[3].p1 = p[0]; f[3].p2 = p[2]; f[3].p3 = p[3];
    f[4].p1 = p[1]; f[4].p2 = p[4]; f[4].p3 = p[3];
    f[5].p1 = p[1]; f[5].p2 = p[5]; f[5].p3 = p[4];
    f[6].p1 = p[1]; f[6].p2 = p[2]; f[6].p3 = p[5];
    f[7].p1 = p[1]; f[7].p2 = p[3]; f[7].p3 = p[2];
    nt = 8;
    
    /* Bisect each edge and move to the surface of a unit sphere */
    for (it=0;it<iterations;it++) {
        ntold = nt;
        for (i=0;i<ntold;i++) {
            pa.x = (f[i].p1.x + f[i].p2.x) / 2;
            pa.y = (f[i].p1.y + f[i].p2.y) / 2;
            pa.z = (f[i].p1.z + f[i].p2.z) / 2;
            pb.x = (f[i].p2.x + f[i].p3.x) / 2;
            pb.y = (f[i].p2.y + f[i].p3.y) / 2;
            pb.z = (f[i].p2.z + f[i].p3.z) / 2;
            pc.x = (f[i].p3.x + f[i].p1.x) / 2;
            pc.y = (f[i].p3.y + f[i].p1.y) / 2;
            pc.z = (f[i].p3.z + f[i].p1.z) / 2;
            pa.normalize();
            pb.normalize();
            pc.normalize();
            f[nt].p1 = f[i].p1; f[nt].p2 = pa; f[nt].p3 = pc; nt++;
            f[nt].p1 = pa; f[nt].p2 = f[i].p2; f[nt].p3 = pb; nt++;
            f[nt].p1 = pb; f[nt].p2 = f[i].p3; f[nt].p3 = pc; nt++;
            f[i].p1 = pa;
            f[i].p2 = pb;
            f[i].p3 = pc;
        }
    }
    
    for(int facet_index=0; facet_index < facet_count; facet_index++) {
        vertices.push_back(f[facet_index].p1);
        vertices.push_back(f[facet_index].p2);
        vertices.push_back(f[facet_index].p3);
    }
    
    submesh_starts.push_back(0);
    submesh_lengths.push_back(vertices.size());
    material_names.push_back("");
    
    LoadData(vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names, bone_names, bone_indexes, bone_weights, KRENGINE_MODEL_FORMAT_TRIANGLES);
}

KRMeshSphere::~KRMeshSphere()
{
    
}
