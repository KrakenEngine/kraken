//
//  KRTexture2D.cpp
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

#include "KRTexture2D.h"
#include "KRTextureManager.h"

#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#import <stdint.h>
#import <assert.h>


#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

static char gPVRTexIdentifier[5] = "PVR!";

enum
{
	kPVRTextureFlagTypePVRTC_2 = 24,
	kPVRTextureFlagTypePVRTC_4
};

typedef struct _PVRTexHeader
{
	uint32_t headerLength;
	uint32_t height;
	uint32_t width;
	uint32_t numMipmaps;
	uint32_t flags;
	uint32_t dataLength;
	uint32_t bpp;
	uint32_t bitmaskRed;
	uint32_t bitmaskGreen;
	uint32_t bitmaskBlue;
	uint32_t bitmaskAlpha;
	uint32_t pvrTag;
	uint32_t numSurfs;
} PVRTexHeader;

KRTexture2D::KRTexture2D(KRDataBlock *data, KRTextureManager *manager) : KRTexture(data, manager) {
    m_iHandle = 0;
    m_current_lod_max_dim = 0;
    m_textureMemUsed = 0;
    load();
}

KRTexture2D::~KRTexture2D() {
    long textureMemFreed = 0;
    releaseHandle(textureMemFreed);
    delete m_pData;
}



bool KRTexture2D::load() {
#if TARGET_OS_IPHONE
    PVRTexHeader *header = (PVRTexHeader *)m_pData->getStart();
    uint32_t formatFlags = header->flags & PVR_TEXTURE_FLAG_TYPE_MASK;
    if (formatFlags == kPVRTextureFlagTypePVRTC_4) {
        m_internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
    } else if(formatFlags == kPVRTextureFlagTypePVRTC_2) {
        m_internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    } else {
        assert(false);
    }
    
    uint32_t pvrTag = header->pvrTag;
    if (gPVRTexIdentifier[0] != ((pvrTag >>  0) & 0xff) ||
        gPVRTexIdentifier[1] != ((pvrTag >>  8) & 0xff) ||
        gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
        gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff))
    {
        return false;
    }
    
    m_iWidth = header->width; // Note: call __builtin_bswap32 when needed to switch endianness
    m_iHeight = header->height;
    m_bHasAlpha = header->bitmaskAlpha;
    
    uint8_t *bytes = ((uint8_t *)m_pData->getStart()) + sizeof(PVRTexHeader);
    uint32_t dataLength = header->dataLength, dataOffset = 0, dataSize = 0;
    uint32_t width = m_iWidth, height = m_iHeight, bpp = 4;
    uint32_t blockSize = 0, widthBlocks = 0, heightBlocks = 0;
    
    // Calculate the data size for each texture level and respect the minimum number of blocks
    while(dataOffset < dataLength) {
        if (formatFlags == kPVRTextureFlagTypePVRTC_4) {
            blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
            widthBlocks = width / 4;
            heightBlocks = height / 4;
            bpp = 4;
        } else {
            blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
            widthBlocks = width / 8;
            heightBlocks = height / 4;
            bpp = 2;
        }
        
        // Clamp to minimum number of blocks
        if (widthBlocks < 2) {
            widthBlocks = 2;
        }
        if (heightBlocks < 2) {
            heightBlocks = 2;
        }
        dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
        
        dataBlockStruct newBlock;
        newBlock.start = bytes+dataOffset;
        newBlock.length = dataSize;
        
        m_blocks.push_back(newBlock);
        
        dataOffset += dataSize;
        
        width = width >> 1;
        if(width < 1) {
            width = 1;
        }
        height = height >> 1;
        if(height < 1) {
            height = 1;
        }
    }
    
    m_max_lod_max_dim = m_iWidth > m_iHeight ? m_iWidth : m_iHeight;
    m_min_lod_max_dim = width >> height ? width : height;
#endif
    return true;

}



bool KRTexture2D::createGLTexture(int lod_max_dim) {    
    m_current_lod_max_dim = 0;
	int width = m_iWidth;
	int height = m_iHeight;
	GLenum err;
    
    if(m_blocks.size() == 0) {
        return false;
    }
	
    GLDEBUG(glGenTextures(1, &m_iHandle));
    if(m_iHandle == 0) {
        return false;
    }
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, m_iHandle));
	
	if (m_blocks.size() > 1) {
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    } else {
        GLDEBUG(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    }
    int i=0;
    for(std::list<dataBlockStruct>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
        dataBlockStruct block = *itr;
        if(width <= lod_max_dim && height <= lod_max_dim) {
            if(width > m_current_lod_max_dim) {
                m_current_lod_max_dim = width;
            }
            if(height > m_current_lod_max_dim) {
                m_current_lod_max_dim = height;
            }
            GLDEBUG(glCompressedTexImage2D(GL_TEXTURE_2D, i, m_internalFormat, width, height, 0, block.length, block.start));
            
            err = glGetError();
            if (err != GL_NO_ERROR) {
                GLDEBUG(glDeleteTextures(1, &m_iHandle));
                m_textureMemUsed = 0;
                m_iHandle = 0;
                lod_max_dim = 0;
                return false;
            }
            m_textureMemUsed += block.length;
            i++;
        }
		
        width = width >> 1;
        if(width < 1) {
            width = 1;
        }
        height = height >> 1;
        if(height < 1) {
            height = 1;
        }
	}
    
    return true;
}

GLuint KRTexture2D::getHandle(long &textureMemUsed, int max_dim, bool can_resize) {
    // Constrain target LOD to be within mipmap levels of texture
    int target_dim = max_dim;
    if(target_dim < m_min_lod_max_dim) target_dim = m_min_lod_max_dim;
    if(target_dim > m_max_lod_max_dim) target_dim = m_max_lod_max_dim;
    
    if(can_resize && m_current_lod_max_dim != target_dim) {
        releaseHandle(textureMemUsed);
    }
    if(m_iHandle == 0) {
        if(createGLTexture(target_dim)) {
            textureMemUsed += getMemSize();
        } else {
            assert(false);
        }
        
        //createGLTexture();
    }
    return m_iHandle;
}

void KRTexture2D::releaseHandle(long &textureMemUsed) {
    if(m_iHandle != 0) {
        textureMemUsed -= getMemSize();
        GLDEBUG(glDeleteTextures(1, &m_iHandle));
        m_iHandle = 0;
        m_textureMemUsed = 0;
    }
}

long KRTexture2D::getMemSize() {
    return m_textureMemUsed; // TODO - This is not 100% accurate, as loaded format may differ in size while in GPU memory
}
