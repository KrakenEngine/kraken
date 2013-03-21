//
//  KRMeshManager.h
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

#ifndef KRMESHMANAGER_H
#define KRMESHMANAGER_H

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"

class KRContext;
class KRMesh;

class KRMeshManager : public KRContextObject {
public:
    static const int KRENGINE_MAX_VOLUMETRIC_PLANES=500;
    static const int KRENGINE_MAX_RANDOM_PARTICLES=150000;
    
    KRMeshManager(KRContext &context);
    virtual ~KRMeshManager();
    
    void rotateBuffers(bool new_frame);
    
    KRMesh *loadModel(const char *szName, KRDataBlock *pData);
    std::vector<KRMesh *> getModel(const char *szName);
    void addModel(KRMesh *model);
    
    std::vector<std::string> getModelNames();
    std::multimap<std::string, KRMesh *> getModels();
    
    
    void bindVBO(GLvoid *data, GLsizeiptr size, GLvoid *index_data, GLsizeiptr index_data_size, bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb, bool enable_bone_indexes, bool enable_bone_weights, bool static_vbo);
    void releaseVBO(GLvoid *data);
    void unbindVBO();
    long getMemUsed();
    
    void configureAttribs(bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb, bool enable_bone_indexes, bool enable_bone_weights);
    
    
    typedef struct {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    } KRVector3D;
    
    typedef struct {
        GLfloat u;
        GLfloat v;
    } TexCoord;
    
    typedef struct {
        KRVector3D vertex;
        TexCoord uva;
    } RandomParticleVertexData;
    
    typedef struct {
        KRVector3D vertex;
    } VolumetricLightingVertexData;
    
    RandomParticleVertexData *getRandomParticles();
    VolumetricLightingVertexData *getVolumetricLightingVertexes();
    
private:
    std::multimap<std::string, KRMesh *> m_models; // Multiple models with the same name/key may be inserted, representing multiple LOD levels of the model
    
    typedef struct vbo_info {
        GLuint vbo_handle;
        GLuint vbo_handle_indexes;
        GLuint vao_handle;
        GLsizeiptr size;
        GLvoid *data;
    } vbo_info_type;
    
    long m_vboMemUsed;
    vbo_info_type m_currentVBO;
    
    std::map<GLvoid *, vbo_info_type> m_vbosActive;
    std::map<GLvoid *, vbo_info_type> m_vbosPool;
    
    RandomParticleVertexData *m_randomParticleVertexData;
    VolumetricLightingVertexData *m_volumetricLightingVertexData;
    
};

#endif
