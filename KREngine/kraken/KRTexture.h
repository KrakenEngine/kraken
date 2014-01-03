//
//  KRTexture.h
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



#ifndef KRTEXTURE_H
#define KRTEXTURE_H

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRResource.h"


class KRDataBlock;

class KRTexture : public KRResource {
public:
    KRTexture(KRContext &context, std::string name);
    virtual ~KRTexture();

    virtual void bind(GLuint texture_unit);
    void releaseHandles();
    long getMemSize();
    virtual long getReferencedMemSize();
    
    virtual long getMemRequiredForSize(int max_dim) = 0;
    virtual void resize(int max_dim);
    
    long getLastFrameUsed();
    
    virtual void resetPoolExpiry();
    virtual bool isAnimated();
    
    virtual KRTexture *compress();
    int getCurrentLodMaxDim();
    int getMaxMipMap();
    int getMinMipMap();
    bool hasMipmaps();

    bool canStreamOut() const;
    
    void _swapHandles();
protected:
    virtual bool createGLTexture(int lod_max_dim) = 0;
    GLuint getHandle();
    
    
    GLuint m_iHandle;
    GLuint m_iNewHandle;
    std::atomic_flag m_handle_lock;
    
    int m_current_lod_max_dim;
    
    uint32_t m_max_lod_max_dim;
    uint32_t m_min_lod_max_dim;
    
    long m_last_frame_used;
    long m_last_frame_bound;
    
private:
    std::atomic<long> m_textureMemUsed;
    std::atomic<long> m_newTextureMemUsed;
};


#endif /* defined(KRTEXTURE_H) */
