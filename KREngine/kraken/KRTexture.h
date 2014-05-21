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
class KRCamera;

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
    
    typedef enum {
        TEXTURE_USAGE_NONE = 0x00,
        TEXTURE_USAGE_UI = 0x01,
        TEXTURE_USAGE_SKY_CUBE = 0x02,
        TEXTURE_USAGE_LIGHT_MAP = 0x04,
        TEXTURE_USAGE_DIFFUSE_MAP = 0x08,
        TEXTURE_USAGE_AMBIENT_MAP = 0x10,
        TEXTURE_USAGE_SPECULAR_MAP = 0x20,
        TEXTURE_USAGE_NORMAL_MAP = 0x40,
        TEXTURE_USAGE_REFLECTION_MAP = 0x80,
        TEXTURE_USAGE_REFECTION_CUBE = 0x100,
        TEXTURE_USAGE_LIGHT_FLARE = 0x200,
        TEXTURE_USAGE_SHADOW_DEPTH = 0x400,
        TEXTURE_USAGE_PARTICLE = 0x800,
        TEXTURE_USAGE_SPRITE = 0x1000
    } texture_usage_t;
    
    float getStreamPriority();
    
    virtual void resetPoolExpiry(float lodCoverage, texture_usage_t textureUsage);
    virtual bool isAnimated();
    
    virtual KRTexture *compress(bool premultiply_alpha = false);
    int getCurrentLodMaxDim();
    int getNewLodMaxDim(); // For use by streamer only
    int getMaxMipMap();
    int getMinMipMap();
    bool hasMipmaps();

    kraken_stream_level getStreamLevel(KRTexture::texture_usage_t textureUsage);
    float getLastFrameLodCoverage() const;
    
    void _swapHandles();
    
protected:
    virtual bool createGLTexture(int lod_max_dim) = 0;
    GLuint getHandle();
    
    
    GLuint m_iHandle;
    GLuint m_iNewHandle;
    std::atomic_flag m_handle_lock;
    
    int m_current_lod_max_dim;
    int m_new_lod_max_dim;
    
    uint32_t m_max_lod_max_dim;
    uint32_t m_min_lod_max_dim;
    
    long m_last_frame_used;
    float m_last_frame_max_lod_coverage;
    texture_usage_t m_last_frame_usage;
    
private:
    std::atomic<long> m_textureMemUsed;
    std::atomic<long> m_newTextureMemUsed;
};


#endif /* defined(KRTEXTURE_H) */
