//
//  SourceManager.h
//  KREngine
//
//  Copyright 2019 Kearwood Gilbert. All rights reserved.
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

#ifndef KRSOURCE_MANAGER_H
#define KRSOURCE_MANAGER_H

#include "KREngine-common.h"

#include "KRSource.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"

class KRSourceManager : public KRContextObject {
public:
    KRSourceManager(KRContext &context);
    virtual ~KRSourceManager();
    
    void add(KRSource *source);
    
    KRSource *load(const std::string &name, const std::string &extension, KRDataBlock *data);
    KRSource *get(const std::string &name, const std::string &extension);
    

    const unordered_map<std::string, KRSource *> &get(const std::string &extension);
    
    unordered_map<std::string, unordered_map<std::string, KRSource *> > &getSources();
    
private:
    unordered_map<std::string, unordered_map<std::string, KRSource *> > m_sources;
};

#endif /* defined(KRUNKNOWN_MANAGER_H) */
