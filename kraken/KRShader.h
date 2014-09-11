//
//  KRShader.h
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



#ifndef KRSHADER_H
#define KRSHADER_H

#include "KREngine-common.h"
#include "KRShader.h"
#include "KRMat4.h"
#include "KRCamera.h"
#include "KRNode.h"
#include "KRViewport.h"

class KRShader  : public KRContextObject {
public:
    KRShader(KRContext &context, char *szKey, std::string options, std::string vertShaderSource, const std::string fragShaderSource);
    virtual ~KRShader();
    const char *getKey() const;
    
    bool bind(KRCamera &camera, const KRViewport &viewport, const KRMat4 &matModel, const std::vector<KRPointLight *> &point_lights, const std::vector<KRDirectionalLight *> &directional_lights, const std::vector<KRSpotLight *>&spot_lights, const KRNode::RenderPass &renderPass, const KRVector3 &rim_color, float rim_power, const KRVector4 &fade_color);
    
    enum {
        KRENGINE_UNIFORM_MATERIAL_AMBIENT = 0,
        KRENGINE_UNIFORM_MATERIAL_DIFFUSE,
        KRENGINE_UNIFORM_MATERIAL_SPECULAR,
        KRENGINE_UNIFORM_MATERIAL_REFLECTION,
        KRENGINE_UNIFORM_MATERIAL_ALPHA,
        KRENGINE_UNIFORM_MATERIAL_SHININESS,
        KRENGINE_UNIFORM_LIGHT_POSITION,
        KRENGINE_UNIFORM_LIGHT_DIRECTION_MODEL_SPACE,
        KRENGINE_UNIFORM_LIGHT_DIRECTION_VIEW_SPACE,
        KRENGINE_UNIFORM_LIGHT_COLOR,
        KRENGINE_UNIFORM_LIGHT_DECAY_START,
        KRENGINE_UNIFORM_LIGHT_CUTOFF,
        KRENGINE_UNIFORM_LIGHT_INTENSITY,
        KRENGINE_UNIFORM_FLARE_SIZE,
        KRENGINE_UNIFORM_VIEW_SPACE_MODEL_ORIGIN,
        KRENGINE_UNIFORM_MVP,
        KRENGINE_UNIFORM_INVP,
        KRENGINE_UNIFORM_INVMVP,
        KRENGINE_UNIFORM_INVMVP_NO_TRANSLATE,
        KRENGINE_UNIFORM_MODEL_VIEW_INVERSE_TRANSPOSE,
        KRENGINE_UNIFORM_MODEL_INVERSE_TRANSPOSE,
        KRENGINE_UNIFORM_MODEL_VIEW,
        KRENGINE_UNIFORM_MODEL_MATRIX,
        KRENGINE_UNIFORM_PROJECTION_MATRIX,
        KRENGINE_UNIFORM_CAMERAPOS_MODEL_SPACE,
        KRENGINE_UNIFORM_VIEWPORT,
        KRENGINE_UNIFORM_VIEWPORT_DOWNSAMPLE,
        KRENGINE_UNIFORM_DIFFUSETEXTURE,
        KRENGINE_UNIFORM_SPECULARTEXTURE,
        KRENGINE_UNIFORM_REFLECTIONCUBETEXTURE,
        KRENGINE_UNIFORM_REFLECTIONTEXTURE,
        KRENGINE_UNIFORM_NORMALTEXTURE,
        KRENGINE_UNIFORM_DIFFUSETEXTURE_SCALE,
        KRENGINE_UNIFORM_SPECULARTEXTURE_SCALE,
        KRENGINE_UNIFORM_REFLECTIONTEXTURE_SCALE,
        KRENGINE_UNIFORM_NORMALTEXTURE_SCALE,
        KRENGINE_UNIFORM_AMBIENTTEXTURE_SCALE,
        KRENGINE_UNIFORM_DIFFUSETEXTURE_OFFSET,
        KRENGINE_UNIFORM_SPECULARTEXTURE_OFFSET,
        KRENGINE_UNIFORM_REFLECTIONTEXTURE_OFFSET,
        KRENGINE_UNIFORM_NORMALTEXTURE_OFFSET,
        KRENGINE_UNIFORM_AMBIENTTEXTURE_OFFSET,
        KRENGINE_UNIFORM_SHADOWMVP1,
        KRENGINE_UNIFORM_SHADOWMVP2,
        KRENGINE_UNIFORM_SHADOWMVP3,
        KRENGINE_UNIFORM_SHADOWTEXTURE1,
        KRENGINE_UNIFORM_SHADOWTEXTURE2,
        KRENGINE_UNIFORM_SHADOWTEXTURE3,
        KRENGINE_UNIFORM_LIGHTMAPTEXTURE,
        KRENGINE_UNIFORM_GBUFFER_FRAME,
        KRENGINE_UNIFORM_GBUFFER_DEPTH,
        KRENGINE_UNIFORM_DEPTH_FRAME,
        KRENGINE_UNIFORM_VOLUMETRIC_ENVIRONMENT_FRAME,
        KRENGINE_UNIFORM_RENDER_FRAME,
        KRENGINE_UNIFORM_ABSOLUTE_TIME,
        KRENGINE_UNIFORM_FOG_NEAR,
        KRENGINE_UNIFORM_FOG_FAR,
        KRENGINE_UNIFORM_FOG_DENSITY,
        KRENGINE_UNIFORM_FOG_COLOR,
        KRENGINE_UNIFORM_FOG_SCALE,
        KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_EXPONENTIAL,
        KRENGINE_UNIFORM_DENSITY_PREMULTIPLIED_SQUARED,
        KRENGINE_UNIFORM_SLICE_DEPTH_SCALE,
        KRENGINE_UNIFORM_PARTICLE_ORIGIN,
        KRENGINE_UNIFORM_BONE_TRANSFORMS,
        KRENGINE_UNIFORM_RIM_COLOR,
        KRENGINE_UNIFORM_RIM_POWER,
        KRENGINE_UNIFORM_FADE_COLOR,
        KRENGINE_NUM_UNIFORMS
    };
    /*
    typedef enum {
        KRENGINE_UNIFORM_TYPE_UNKNOWN,
        KRENGINE_UNIFORM_TYPE_FLOAT,
        KRENGINE_UNIFORM_TYPE_INT,
        KRENGINE_UNIFORM_TYPE_VECTOR2,
        KRENGINE_UNIFORM_TYPE_VECTOR3,
        KRENGINE_UNIFORM_TYPE_VECTOR4,
        KRENGINE_UNIFORM_TYPE_MAT4
    } uniform_type_t;
     uniform_type_t m_uniform_type[KRENGINE_NUM_UNIFORMS];
     */
    
    static const char *KRENGINE_UNIFORM_NAMES[];
    GLint m_uniforms[KRENGINE_NUM_UNIFORMS];
    
    int m_uniform_value_index[KRENGINE_NUM_UNIFORMS];
    
    std::vector<float> m_uniform_value_float;
    std::vector<int> m_uniform_value_int;
    std::vector<KRVector2> m_uniform_value_vector2;
    std::vector<KRVector3> m_uniform_value_vector3;
    std::vector<KRVector4> m_uniform_value_vector4;
    std::vector<KRMat4> m_uniform_value_mat4;
    
    
    char m_szKey[256];
    
    void setUniform(int location, float value);
    void setUniform(int location, int value);
    void setUniform(int location, const KRVector2 &value);
    void setUniform(int location, const KRVector3 &value);
    void setUniform(int location, const KRVector4 &value);
    void setUniform(int location, const KRMat4 &value);
    
private:
    GLuint m_iProgram;
};

#endif
