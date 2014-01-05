//
//  KRTextureKTX.cpp
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

#include "KRTextureKTX.h"
#include "KRTextureManager.h"

#include "KREngine-common.h"

Byte _KTXFileIdentifier[12] = {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
};

KRTextureKTX::KRTextureKTX(KRContext &context, KRDataBlock *data, std::string name) : KRTexture2D(context, data, name) {
    m_pData->copy(&m_header, 0, sizeof(KTXHeader));
    if(memcmp(_KTXFileIdentifier, m_header.identifier, 12) != 0) {
        assert(false); // Header not recognized
    }
    if(m_header.endianness != 0x04030201) {
        assert(false); // Endianness not (yet) supported
    }
    if(m_header.pixelDepth != 0) {
        assert(false); // 3d textures not (yet) supported
    }
    if(m_header.numberOfArrayElements != 0) {
        assert(false); // Array textures not (yet) supported
    }
    if(m_header.numberOfFaces != 1) {
        assert(false); // Cube-map textures are only supported as a file for each separate face (for now)
    }
    
    uint32_t blockStart = sizeof(KTXHeader) + m_header.bytesOfKeyValueData;
    uint32_t width = m_header.pixelWidth, height = m_header.pixelHeight;
    
    for(int mipmap_level=0; mipmap_level < KRMAX(m_header.numberOfMipmapLevels, 1); mipmap_level++) {
        uint32_t blockLength;
        data->copy(&blockLength, blockStart, 4);
        blockStart += 4;
        
        m_blocks.push_back(m_pData->getSubBlock(blockStart, blockLength));
        
        blockStart += blockLength;
        blockStart = KRALIGN(blockStart);
        
        width = width >> 1;
        if(width < 1) {
            width = 1;
        }
        height = height >> 1;
        if(height < 1) {
            height = 1;
        }
    }
    
    m_max_lod_max_dim = KRMAX(m_header.pixelWidth, m_header.pixelHeight);
    m_min_lod_max_dim = KRMAX(width, height);
}

KRTextureKTX::KRTextureKTX(KRContext &context, std::string name, GLenum internal_format, GLenum base_internal_format, int width, int height, const std::list<KRDataBlock *> &blocks) : KRTexture2D(context, new KRDataBlock(), name)
{
    memcpy(m_header.identifier, _KTXFileIdentifier, 12);
    m_header.endianness = 0x04030201;
    m_header.glType = 0;
    m_header.glTypeSize = 1;
    m_header.glFormat = 0;
    m_header.glInternalFormat = internal_format;
    m_header.glBaseInternalFormat = base_internal_format;
    m_header.pixelWidth = width;
    m_header.pixelHeight = height;
    m_header.pixelDepth = 0;
    m_header.numberOfArrayElements = 0;
    m_header.numberOfFaces = 1;
    m_header.numberOfMipmapLevels = (UInt32)blocks.size();
    m_header.bytesOfKeyValueData = 0;
    
    m_pData->append(&m_header, sizeof(m_header));
    for(auto block_itr = blocks.begin(); block_itr != blocks.end(); block_itr++) {
        KRDataBlock *source_block = *block_itr;
        UInt32 block_size = (UInt32)source_block->getSize();
        m_pData->append(&block_size, 4);
        m_pData->append(*source_block);
        m_blocks.push_back(m_pData->getSubBlock((int)m_pData->getSize() - (int)block_size, (int)block_size));
        size_t alignment_padding_size = KRALIGN(m_pData->getSize()) - m_pData->getSize();
        Byte alignment_padding[4] = {0, 0, 0, 0};
        if(alignment_padding_size > 0) {
            m_pData->append(&alignment_padding, alignment_padding_size);
        }
    }
}

KRTextureKTX::~KRTextureKTX() {
    for(std::list<KRDataBlock *>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
        KRDataBlock *block = *itr;
        delete block;
    }
    m_blocks.clear();
}

long KRTextureKTX::getMemRequiredForSize(int max_dim)
{
    int target_dim = max_dim;
    if(target_dim < m_min_lod_max_dim) target_dim = target_dim;
    
    // Determine how much memory will be consumed
    
	int width = m_header.pixelWidth;
	int height = m_header.pixelHeight;
    long memoryRequired = 0;
    
    for(std::list<KRDataBlock *>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
        KRDataBlock *block = *itr;
        if(width <= target_dim && height <= target_dim) {
            memoryRequired += block->getSize();
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
    
    return memoryRequired;
}

bool KRTextureKTX::uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, int prev_lod_max_dim, bool compress)
{
    int target_dim = lod_max_dim;
    if(target_dim < m_min_lod_max_dim) target_dim = m_min_lod_max_dim;
    
    if(m_blocks.size() == 0) {
        return false;
    }
    
    // Determine how much memory will be consumed
	int width = m_header.pixelWidth;
	int height = m_header.pixelHeight;
    long memoryRequired = 0;
    long memoryTransferred = 0;
    
    
#if GL_EXT_texture_storage
    
    if(target == GL_TEXTURE_CUBE_MAP_POSITIVE_X || target == GL_TEXTURE_2D) {
        // Call glTexStorage2DEXT only for the first uploadTexture used when creating a texture
        int level_count=0;
        int max_lod_width=0;
        int max_lod_height=0;
        for(std::list<KRDataBlock *>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
            if(width <= target_dim && height <= target_dim) {
                if(max_lod_width == 0) {
                    max_lod_width = width;
                    max_lod_height = height;
                }
                
                level_count++;
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
        width = m_header.pixelWidth;
        height = m_header.pixelHeight;
        
        if(target == GL_TEXTURE_CUBE_MAP_POSITIVE_X) {
            glTexStorage2DEXT(GL_TEXTURE_CUBE_MAP, level_count, (GLenum)m_header.glInternalFormat, max_lod_width, max_lod_height);
        } else if(target == GL_TEXTURE_2D) {
            glTexStorage2DEXT(target, level_count, (GLenum)m_header.glInternalFormat, max_lod_width, max_lod_height);
        }
    }
#endif
    
    // Upload texture data
    int destination_level=0;
    int source_level = 0;
    for(std::list<KRDataBlock *>::iterator itr = m_blocks.begin(); itr != m_blocks.end(); itr++) {
        KRDataBlock *block = *itr;
        if(width <= target_dim && height <= target_dim) {
            
            if(width > current_lod_max_dim) {
                current_lod_max_dim = width;
            }
            if(height > current_lod_max_dim) {
                current_lod_max_dim = height;
            }
#if GL_APPLE_copy_texture_levels && GL_EXT_texture_storage
            if(target == GL_TEXTURE_2D && width <= prev_lod_max_dim && height <= prev_lod_max_dim) {
                //GLDEBUG(glCompressedTexImage2D(target, i, (GLenum)m_header.glInternalFormat, width, height, 0, block.length, NULL)); // Allocate, but don't copy
                //                GLDEBUG(glTexImage2D(target, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL));
                GLDEBUG(glCopyTextureLevelsAPPLE(m_iNewHandle, m_iHandle, source_level, 1));
            } else {
                block->lock();
                GLDEBUG(glCompressedTexSubImage2D(target, destination_level, 0, 0, width, height, (GLenum)m_header.glInternalFormat, (GLsizei)block->getSize(), block->getStart()));
                block->unlock();
                
                memoryTransferred += block->getSize(); // memoryTransferred does not include throughput of mipmap levels copied through glCopyTextureLevelsAPPLE
            }
#else
            block->lock();
#if GL_EXT_texture_storage
            GLDEBUG(glCompressedTexSubImage2D(target, destination_level, 0, 0, width, height, (GLenum)m_header.glInternalFormat, (GLsizei)block->getSize(), block->getStart()));
#else
            GLDEBUG(glCompressedTexImage2D(target, destination_level, (GLenum)m_header.glInternalFormat, width, height, 0, (GLsizei)block->getSize(), block->getStart()));
#endif
            block->unlock();
            memoryTransferred += block->getSize(); // memoryTransferred does not include throughput of mipmap levels copied through glCopyTextureLevelsAPPLE
#endif
            memoryRequired += block->getSize();
            //
            //            err = glGetError();
            //            if (err != GL_NO_ERROR) {
            //                assert(false);
            //                return false;
            //            }
            //
            
            destination_level++;
        }
        
        if(width <= prev_lod_max_dim && height <= prev_lod_max_dim) {
            source_level++;
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

std::string KRTextureKTX::getExtension()
{
    return "ktx";
}

