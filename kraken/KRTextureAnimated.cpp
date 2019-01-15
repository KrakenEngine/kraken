//
//  KRTextureAnimated.cpp
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

#include "KREngine-common.h"
#include "KRTextureAnimated.h"
#include "KRTexture2D.h"
#include "KRContext.h"

KRTextureAnimated::KRTextureAnimated(KRContext &context, std::string name) : KRTexture(context, name)
{
    // Format of name:
    //   animate:texturebasename,xx,yy
    // Where - texturebasename is a prefix for the other textures
    //       - xx is the number of frames
    //       - yy is the framerate
    
    // TODO - Add error handling for mal-formatted animated texture formats
    size_t first_comma_pos = name.find(",");
    size_t second_comma_pos = name.find(",", first_comma_pos + 1);
    
    
    m_texture_base_name = name.substr(8, first_comma_pos - 8);
    m_frame_count = atoi(name.substr(first_comma_pos+1, second_comma_pos - first_comma_pos -1).c_str());
    m_frame_rate = (float)atof(name.substr(second_comma_pos+1).c_str());
    
    m_max_lod_max_dim = 2048;
    m_min_lod_max_dim = 64;
    
    for(int i=0; i<m_frame_count; i++) {
        KRTexture2D *frame_texture = textureForFrame(i);
        if(frame_texture) {
            if(frame_texture->getMaxMipMap() < (int)m_max_lod_max_dim) m_max_lod_max_dim = frame_texture->getMaxMipMap();
            if(frame_texture->getMinMipMap() > (int)m_min_lod_max_dim) m_min_lod_max_dim = frame_texture->getMinMipMap();
        }
    }
}

std::string KRTextureAnimated::textureNameForFrame(int frame)
{
    char szFrameNumber[10];
    sprintf(szFrameNumber, "%i", frame);
    return m_texture_base_name + szFrameNumber;
}

KRTexture2D *KRTextureAnimated::textureForFrame(int frame)
{
    return (KRTexture2D *)getContext().getTextureManager()->getTexture(textureNameForFrame(frame));
}

KRTextureAnimated::~KRTextureAnimated()
{

}

bool KRTextureAnimated::createGLTexture(int lod_max_dim)
{
    return true;
}

long KRTextureAnimated::getMemRequiredForSize(int max_dim)
{
    return 0; // Memory is allocated by individual frame textures
}


void KRTextureAnimated::resetPoolExpiry(float lodCoverage, texture_usage_t textureUsage)
{
    KRTexture::resetPoolExpiry(lodCoverage, textureUsage);
    for(int i=0; i<m_frame_count; i++) {
        KRTexture2D *frame_texture = textureForFrame(i);
        if(frame_texture) {
            frame_texture->resetPoolExpiry(lodCoverage, textureUsage); // Ensure that frames of animated textures do not expire from the texture pool prematurely, as they are referenced indirectly
        }
    }
}

void KRTextureAnimated::bind(GLuint texture_unit)
{
    resetPoolExpiry(0.0f, TEXTURE_USAGE_NONE); // TODO - Need to set parameters here for streaming priority?
    KRTexture::bind(texture_unit);
    int frame_number = (int)floor(fmodf(getContext().getAbsoluteTime() * m_frame_rate, (float)m_frame_count));
    KRTexture2D *frame_texture = textureForFrame(frame_number);
    if(frame_texture) {
        frame_texture->bind(texture_unit);
    }
}

long KRTextureAnimated::getReferencedMemSize()
{
    long referenced_mem = 0;
    for(int i=0; i<m_frame_count; i++) {
        KRTexture2D *frame_texture = textureForFrame(i);
        if(frame_texture) {
            referenced_mem += frame_texture->getMemSize();
        }
    }
    return referenced_mem;
}

bool KRTextureAnimated::isAnimated()
{
    return true;
}

std::string KRTextureAnimated::getExtension()
{
    return ""; // Animated textures are just references; there are no files to output
}

bool KRTextureAnimated::save(const std::string &path)
{
    return true; // Animated textures are just references; there are no files to output
}

bool KRTextureAnimated::save(KRDataBlock &data)
{
    return true; // Animated textures are just references; there are no files to output
}

void KRTextureAnimated::resize(int max_dim)
{
    // Purposely not calling the superclass method
}