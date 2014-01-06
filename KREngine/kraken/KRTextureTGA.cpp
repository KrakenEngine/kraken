//
//  KRTextureTGA.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRTextureTGA.h"
#include "KREngine-common.h"
#include "KRContext.h"
#include "KRTextureKTX.h"

typedef struct {
    char  idlength;
    char  colourmaptype;
    char  imagetype;
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
} __attribute__((packed)) TGA_HEADER;


KRTextureTGA::KRTextureTGA(KRContext &context, KRDataBlock *data, std::string name) : KRTexture2D(context, data, name)
{
    data->lock();
    TGA_HEADER *pHeader = (TGA_HEADER *)data->getStart();
    
    m_max_lod_max_dim = pHeader->width > pHeader->height ? pHeader->width : pHeader->height;
    m_min_lod_max_dim = m_max_lod_max_dim; // Mipmaps not yet supported for TGA images
    switch(pHeader->imagetype) {
        case 2: // rgb
            switch(pHeader->bitsperpixel) {
                case 24:
                {
                    m_imageSize = pHeader->width * pHeader->height * 4;
                }
                break;
                case 32:
                {
                    m_imageSize = pHeader->width * pHeader->height * 4;
                }
                break;
                    
                default:
                {
                    assert(false);
                }
                break;
            }
            break;
        default:
        {
            assert(false);
            break;
        }
    }
    
    data->unlock();
}

KRTextureTGA::~KRTextureTGA()
{
    
}

bool KRTextureTGA::uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, int prev_lod_max_dim, bool compress)
{
    m_pData->lock();
    TGA_HEADER *pHeader = (TGA_HEADER *)m_pData->getStart();
    unsigned char *pData = (unsigned char *)pHeader + (long)pHeader->idlength + (long)pHeader->colourmaplength * (long)pHeader->colourmaptype + sizeof(TGA_HEADER);

//
// FINDME - many of the GL constants in here are not defined in GLES2
#ifdef TARGET_OS_IPHONE
    GLenum base_internal_format = GL_BGRA;
#else
    GLenum base_internal_format = pHeader->bitsperpixel == 24 ? GL_BGR : GL_BGRA;
#endif
    
    GLenum internal_format = 0;
    
#ifndef TARGET_OS_IPHONE
    if(compress) {
        internal_format = pHeader->bitsperpixel == 24 ? GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
#endif
    
    if(pHeader->colourmaptype != 0) {
        m_pData->unlock();
        return false; // Mapped colors not supported
    }
    
    GLenum err;
    
    switch(pHeader->imagetype) {
        case 2: // rgb
            switch(pHeader->bitsperpixel) {
                case 24:
                    {
                        unsigned char *converted_image = (unsigned char *)malloc(pHeader->width * pHeader->height * 4);
//#ifdef __APPLE__
//                        vImage_Buffer   source_image = { pData, pHeader->height, pHeader->width, pHeader->width*3 };
//                        vImage_Buffer   dest_image = { converted_image, pHeader->height, pHeader->width, pHeader->width*4 };
//                        vImageConvert_RGB888toRGBA8888(&source_image, NULL, 0xff, &dest_image, false, kvImageDoNotTile);
//#else
                        unsigned char *pSource = pData;
                        unsigned char *pDest = converted_image;
                        unsigned char *pEnd = pData + pHeader->height * pHeader->width * 3;
                        while(pSource < pEnd) {
                            *pDest++ = pSource[0];
                            *pDest++ = pSource[1];
                            *pDest++ = pSource[2];
                            *pDest++ = 0xff;
                            pSource += 3;
                        }
//#endif
                        glTexImage2D(target, 0, internal_format, pHeader->width, pHeader->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)converted_image);
                        free(converted_image);
                        err = glGetError();
                        if (err != GL_NO_ERROR) {
                            m_pData->unlock();
                            return false;
                        }
                        current_lod_max_dim = m_max_lod_max_dim;
                    }
                    break;
                case 32:
                    {
                        glTexImage2D(target, 0, internal_format, pHeader->width, pHeader->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)pData);
                        err = glGetError();
                        if (err != GL_NO_ERROR) {
                            m_pData->unlock();
                            return false;
                        }
                        current_lod_max_dim = m_max_lod_max_dim;
                    }
                    break;
                default:
                    m_pData->unlock();
                    return false; // 16-bit images not yet supported
            }
            break;
        default:
            m_pData->unlock();
            return false; // Image type not yet supported
    }

    m_pData->unlock();
    return true;
}

KRTexture *KRTextureTGA::compress()
{
    m_pData->lock();
    
    std::list<KRDataBlock *> blocks;
    
    getContext().getTextureManager()->_setActiveTexture(0);
    
    GLuint compressed_handle = 0;
    GLDEBUG(glGenTextures(1, &compressed_handle));
    
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, compressed_handle));
    
    int current_max_dim = 0;
    if(!uploadTexture(GL_TEXTURE_2D, m_max_lod_max_dim, current_max_dim, 0, true)) {
        assert(false); // Failed to upload the texture
    }
    GLDEBUG(glGenerateMipmap(GL_TEXTURE_2D));
    
    GLint width = 0, height = 0, internal_format, base_internal_format;

#ifndef TARGET_OS_IPHONE
    GLDEBUG(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width));
    GLDEBUG(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height));
    GLDEBUG(glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format));
    
    switch(internal_format)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            base_internal_format = GL_BGRA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            base_internal_format = GL_BGRA;
            break;
        default:
            assert(false); // Not yet supported
            break;
    }
    
    GLuint lod_level = 0;
    GLint compressed_size = 0;
    GLenum err = GL_NO_ERROR;
    while(err == GL_NO_ERROR) {
        glGetTexLevelParameteriv(GL_TEXTURE_2D, lod_level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size);
        err = glGetError();
        if(err == GL_NO_ERROR) {
            KRDataBlock *new_block = new KRDataBlock();
            new_block->expand(compressed_size);
            new_block->lock();
            GLDEBUG(glGetCompressedTexImage(GL_TEXTURE_2D, lod_level, new_block->getStart()));
            new_block->unlock();
            blocks.push_back(new_block);
            
            lod_level++;
        }
    }
    if(err != GL_INVALID_VALUE) {
        // err will equal GL_INVALID_VALUE when
        // assert(false); // Unexpected error
    }
#endif
    
    GLDEBUG(glBindTexture(GL_TEXTURE_2D, 0));
    getContext().getTextureManager()->selectTexture(0, NULL);
    GLDEBUG(glDeleteTextures(1, &compressed_handle));
    
    KRTextureKTX *new_texture = new KRTextureKTX(getContext(), getName(), internal_format, base_internal_format, width, height, blocks);
    
    KRResource *test_resource = new_texture;
    
    m_pData->unlock();
    
    for(auto block_itr = blocks.begin(); block_itr != blocks.end(); block_itr++) {
        KRDataBlock *block = *block_itr;
        delete block;
    }
    
    return new_texture;
}

long KRTextureTGA::getMemRequiredForSize(int max_dim)
{
    return m_imageSize;
}

std::string KRTextureTGA::getExtension()
{
    return "tga";
}
