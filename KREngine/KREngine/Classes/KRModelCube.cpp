//
//  KRModelCube.cpp
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

#include "KRModelCube.h"


KRModelCube::KRModelCube(KRContext &context) : KRModel(context, "__cube")
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
    
    
    vertices.push_back(KRVector3(1.0, 1.0, 1.0));
    vertices.push_back(KRVector3(-1.0, 1.0, 1.0));
    vertices.push_back(KRVector3(1.0,-1.0, 1.0));
    vertices.push_back(KRVector3(-1.0,-1.0, 1.0));
    vertices.push_back(KRVector3(-1.0,-1.0,-1.0));
    vertices.push_back(KRVector3(-1.0, 1.0, 1.0));
    vertices.push_back(KRVector3(-1.0, 1.0,-1.0));
    vertices.push_back(KRVector3(1.0, 1.0, 1.0));
    vertices.push_back(KRVector3(1.0, 1.0,-1.0));
    vertices.push_back(KRVector3(1.0,-1.0, 1.0));
    vertices.push_back(KRVector3(1.0,-1.0,-1.0));
    vertices.push_back(KRVector3(-1.0,-1.0,-1.0));
    vertices.push_back(KRVector3(1.0, 1.0,-1.0));
    vertices.push_back(KRVector3(-1.0, 1.0,-1.0));
    
    
    submesh_starts.push_back(0);
    submesh_lengths.push_back(vertices.size());
    material_names.push_back("");
    
    LoadData(vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names, bone_names, bone_indexes, bone_weights, KRENGINE_MODEL_FORMAT_STRIP);
}

KRModelCube::~KRModelCube()
{
    
}
