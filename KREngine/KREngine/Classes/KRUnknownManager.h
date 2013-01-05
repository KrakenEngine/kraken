//
//  FileManager.h
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

#ifndef KRUNKNOWN_MANAGER_H
#define KRUNKNOWN_MANAGER_H

#import "KREngine-common.h"

#include "KRUnknown.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"

#include <map>
#include <set>
#include <string>

using std::map;
using std::set;

class KRUnknownManager : public KRContextObject {
public:
    KRUnknownManager(KRContext &context);
    virtual ~KRUnknownManager();
    
    void add(KRUnknown *unknown);
    
    KRUnknown *load(const std::string &name, const std::string &extension, KRDataBlock *data);
    KRUnknown *get(const std::string &name, const std::string &extension);
    

    const map<std::string, KRUnknown *> &get(const std::string &extension);
    
private:
    map<std::string, map<std::string, KRUnknown *> > m_unknowns;
};

#endif /* defined(KRUNKNOWN_MANAGER_H) */
