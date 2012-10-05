//
//  KRModelManager.h
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

#ifndef KRMODELMANAGER_H
#define KRMODELMANAGER_H

#define KRENGINE_MAX_VBO_HANDLES 10000
#define KRENGINE_MAX_VBO_MEM 128000000

#import "KREngine-common.h"
#import "KRContextObject.h"
#import "KRDataBlock.h"

class KRContext;
class KRModel;

#include <map>
#import <string>
using std::map;

class KRModelManager : public KRContextObject {
public:
    KRModelManager(KRContext &context);
    virtual ~KRModelManager();
    
    void rotateBuffers(bool new_frame);
    
    KRModel *loadModel(const char *szName, KRDataBlock *pData);
    std::vector<KRModel *> getModel(const char *szName);
    KRModel *getFirstModel();
    
    std::vector<std::string> getModelNames();
    std::multimap<std::string, KRModel *> getModels();
    
    
    void bindVBO(GLvoid *data, GLsizeiptr size, bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb);
    void unbindVBO();
    long getMemUsed();
    
    void configureAttribs(bool enable_vertex, bool enable_normal, bool enable_tangent, bool enable_uva, bool enable_uvb);
    
private:
    std::multimap<std::string, KRModel *> m_models; // Multiple models with the same name/key may be inserted, representing multiple LOD levels of the model
    
    typedef struct vbo_info {
        GLuint handle;
        GLsizeiptr size;
        GLvoid *data;
    } vbo_info_type;
    
    long m_vboMemUsed;
    vbo_info_type m_currentVBO;
    
    std::map<GLvoid *, vbo_info_type> m_vbosActive;
    std::map<GLvoid *, vbo_info_type> m_vbosPool;
    
    bool m_bVBOAttribEnabled_Vertex;
    bool m_bVBOAttribEnabled_Normal;
    bool m_bVBOAttribEnabled_Tangent;
    bool m_bVBOAttribEnabled_UVA;
    bool m_bVBOAttribEnabled_UVB;
    
    
};

#endif
