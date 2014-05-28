//
//  KRTextureManager.h
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

#ifndef KRTEXTUREMANAGER_H
#define KRTEXTUREMANAGER_H

#include "KREngine-common.h"

#include "KRTexture.h"
#include "KRContextObject.h"
#include "KREngine-common.h"
#include "KRDataBlock.h"
#include "KRContext.h"
#include "KRStreamer.h"

class KRTextureManager : public KRContextObject {
public:
    KRTextureManager(KRContext &context);
    virtual ~KRTextureManager();
    
    void selectTexture(int iTextureUnit, KRTexture *pTexture, float lod_coverage, KRTexture::texture_usage_t textureUsage);
    
    KRTexture *loadTexture(const char *szName, const char *szExtension, KRDataBlock *data);
    KRTexture *getTextureCube(const char *szName);
    KRTexture *getTexture(const std::string &name);
    
    long getMemUsed();
    long getMemActive();
    
    long getMemoryTransferedThisFrame();
    void addMemoryTransferredThisFrame(long memoryTransferred);
    
    void memoryChanged(long memoryDelta);
    
    void startFrame(float deltaTime);
    void endFrame(float deltaTime);
    
    unordered_map<std::string, KRTexture *> &getTextures();
    
    void compress(bool premultiply_alpha = false);
    
    std::set<KRTexture *> &getActiveTextures();
    
    void _setActiveTexture(int i);
    void _setWrapModeS(GLuint i, GLuint wrap_mode);
    void _setWrapModeT(GLuint i, GLuint wrap_mode);
    void _setMaxAnisotropy(int i, float max_anisotropy);
    
    void _clearGLState();
    void setMaxAnisotropy(float max_anisotropy);
    
    void doStreaming(long &memoryRemaining, long &memoryRemainingThisFrame);
    void primeTexture(KRTexture *texture);
    
private:
    int m_iActiveTexture;
    
    long m_memoryTransferredThisFrame;
    
    unordered_map<std::string, KRTexture *> m_textures;
    
    KRTexture *m_boundTextures[KRENGINE_MAX_TEXTURE_UNITS];
    GLuint m_wrapModeS[KRENGINE_MAX_TEXTURE_UNITS];
    GLuint m_wrapModeT[KRENGINE_MAX_TEXTURE_UNITS];
    float m_maxAnisotropy[KRENGINE_MAX_TEXTURE_UNITS];
    
    
    std::set<KRTexture *> m_activeTextures;
    
    std::vector<std::pair<float, KRTexture *> > m_activeTextures_streamer;
    std::vector<std::pair<float, KRTexture *> > m_activeTextures_streamer_copy;
    bool m_streamerComplete;
    
    std::atomic<long> m_textureMemUsed;
    
    void balanceTextureMemory(long &memoryRemaining, long &memoryRemainingThisFrame);
    
    std::mutex m_streamerFenceMutex;
};

#endif
