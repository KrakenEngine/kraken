//
//  KRTexture.cpp
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

#include "KRTexture.h"

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

KRTexture::KRTexture() {
    m_iName = 0;
    m_fdFile = 0;
    m_pFile = NULL;
    m_fileSize = 0;
}

KRTexture::~KRTexture() {
    if(m_iName != 0) {
        glDeleteTextures(1, &m_iName);
    }
    if(m_pFile != NULL) {
        munmap(m_pFile, m_fileSize);
    }
    if(m_fdFile != 0) {
        close(m_fdFile);
    }
}

bool KRTexture::loadFromFile(const char *szFile) {
    struct stat statbuf;
    m_fdFile = open(szFile, O_RDONLY);
    if(m_fdFile < 0) {
        return false;
    } else {
        if(fstat(m_fdFile,&statbuf) < 0) {
            return false;
        } else {
            void *pFile;
            if ((pFile = mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, m_fdFile, 0))
                                    == (caddr_t) -1) {
                return false;
            } else {
                m_fileSize = statbuf.st_size;
                m_pFile = pFile;
                
                PVRTexHeader *header = (PVRTexHeader *)pFile;
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
                
                uint8_t *bytes = ((uint8_t *)pFile) + sizeof(PVRTexHeader);
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
                
                return true;
            }
        }
    }
}

bool KRTexture::createGLTexture() {
	int width = m_iWidth;
	int height = m_iHeight;
	GLenum err;
	
	if (m_blocks.size() > 0)
	{
		if (m_iName != 0) {
            glDeleteTextures(1, &m_iName);
        }
		
		glGenTextures(1, &m_iName);
		glBindTexture(GL_TEXTURE_2D, m_iName);
	}
	
	if (m_blocks.size() > 1) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    int i=0;
    for(std::list<dataBlockStruct>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
        dataBlockStruct block = *itr;
		glCompressedTexImage2D(GL_TEXTURE_2D, i, m_internalFormat, width, height, 0, block.length, block.start);
		
		err = glGetError();
		if (err != GL_NO_ERROR) {
			return false;
		}
		
        width = width >> 1;
        if(width < 1) {
            width = 1;
        }
        height = height >> 1;
        if(height < 1) {
            height = 1;
        }
        
        i++;
	}
    
    return true;
}

GLuint KRTexture::getName() {
    if(m_iName == 0) {
        createGLTexture();
    }
    return m_iName;
}