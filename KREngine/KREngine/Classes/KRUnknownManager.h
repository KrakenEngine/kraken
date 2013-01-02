//
//  FileManager.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-02.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
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
