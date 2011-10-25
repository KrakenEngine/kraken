//
//  KRModelManager.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-12-31.
//  Copyright 2010 Kearwood Software. All rights reserved.
//

#ifndef KRMODELMANAGER_H
#define KRMODELMANAGER_H

#include "KRModel.h"

#include <map>
#import <string>
using std::map;

class KRModelManager {
public:
    KRModelManager(KRMaterialManager *pMaterialManager);
    ~KRModelManager();
    
    KRModel *loadModel(const char *szName, const char *szPath);
    KRModel *getModel(const char *szName);
    KRModel *getFirstModel();
    
private:
    std::map<std::string, KRModel *> m_models;
    KRMaterialManager *m_pMaterialManager;
};

#endif
