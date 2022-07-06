//
//  KRPipelineManager.h
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

#include "KRCamera.h"
#include "KRDataBlock.h"
#include "KRNode.h"
#include "KRSurface.h"
#include "KRMesh.h"

using std::map;
using std::vector;

#include "KRPipeline.h"

#ifndef KRPIPELINEMANAGER_H
#define KRPIPELINEMANAGER_H

class KRPipeline;
class PipelineInfo;
class KRCamera;

class KRPipelineManager : public KRContextObject {
public:

    KRPipelineManager(KRContext &context);
    virtual ~KRPipelineManager();
    KRPipeline* get(const char* szKey);
    
    KRPipeline* getPipeline(KRSurface& surface, const PipelineInfo& info);
    bool selectPipeline(KRSurface& surface, KRCamera &camera, KRPipeline *pPipeline, const KRViewport &viewport, const Matrix4 &matModel, const std::vector<KRPointLight *> *point_lights, const std::vector<KRDirectionalLight *> *directional_lights, const std::vector<KRSpotLight *> *spot_lights, int bone_count, const KRNode::RenderPass &renderPass, const Vector3 &rim_color, float rim_power, const Vector4 &fade_color);
    bool selectPipeline(KRSurface& surface, const PipelineInfo& info, const KRViewport& viewport, const Matrix4& matModel, const Vector3& rim_color, float rim_power, const Vector4& fade_color);

    size_t getPipelineHandlesUsed();
    
    KRPipeline *m_active_pipeline;

private:
  typedef std::map<std::pair<std::string, std::vector<int> >, KRPipeline*> PipelineMap;
  PipelineMap m_pipelines;
};

#endif
