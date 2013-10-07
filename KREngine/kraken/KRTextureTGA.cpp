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
    data->unlock();
}

KRTextureTGA::~KRTextureTGA()
{
    
}

bool KRTextureTGA::uploadTexture(GLenum target, int lod_max_dim, int &current_lod_max_dim, long &textureMemUsed, int prev_lod_max_dim, GLuint prev_handle)
{    
    TGA_HEADER *pHeader = (TGA_HEADER *)m_pData->getStart();
    unsigned char *pData = (unsigned char *)pHeader + (long)pHeader->idlength + (long)pHeader->colourmaplength * (long)pHeader->colourmaptype + sizeof(TGA_HEADER);

    if(pHeader->colourmaptype != 0) {
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
                        glTexImage2D(target, 0, GL_RGBA, pHeader->width, pHeader->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)converted_image);
                        free(converted_image);
                        err = glGetError();
                        if (err != GL_NO_ERROR) {
                            return false;
                        }
                        int memAllocated = pHeader->width * pHeader->height * 4;
                        textureMemUsed += memAllocated;
                        getContext().getTextureManager()->memoryChanged(memAllocated);
                        getContext().getTextureManager()->addMemoryTransferredThisFrame(memAllocated);
                        current_lod_max_dim = m_max_lod_max_dim;
                    }
                    break;
                case 32:
                    {
                        glTexImage2D(target, 0, GL_RGBA, pHeader->width, pHeader->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid *)pData);
                        err = glGetError();
                        if (err != GL_NO_ERROR) {
                            return false;
                        }
                        int memAllocated = pHeader->width * pHeader->height * 4;
                        textureMemUsed += memAllocated;
                        getContext().getTextureManager()->memoryChanged(memAllocated);
                        getContext().getTextureManager()->addMemoryTransferredThisFrame(memAllocated);
                        current_lod_max_dim = m_max_lod_max_dim;
                    }
                    break;
                default:
                    return false; // 16-bit images not yet supported
            }
            break;
        default:
            return false; // Image type not yet supported
    }

    return true;
}

long KRTextureTGA::getMemRequiredForSize(int max_dim)
{
    TGA_HEADER *pHeader = (TGA_HEADER *)m_pData->getStart();
    switch(pHeader->imagetype) {
        case 2: // rgb
            switch(pHeader->bitsperpixel) {
                case 24:
                {
                     return pHeader->width * pHeader->height * 4;
                }
                break;
                case 32:
                {
                    return pHeader->width * pHeader->height * 4;
                }
                break;
            }
            break;
    }
    
    return 0;
}

std::string KRTextureTGA::getExtension()
{
    return "tga";
}
