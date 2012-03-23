//
//  KRResource.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#import <stdint.h>
#import <vector>
#import <set>
#import <list>
#import <string>
#import "KREngine-common.h"

#ifndef KREngine_KRResource_h
#define KREngine_KRResource_h

class KRResource
{
public:
    std::string getName();
    
    static std::string GetFileExtension(const std::string& name);
    static std::string GetFileBase(const std::string& name);
    static std::string GetFilePath(const std::string& name);
    
    static std::vector<KRResource *> Load(const std::string& path);
    
    virtual ~KRResource();
    virtual bool save(const std::string& path) = 0;
protected:
    KRResource(std::string name);
   
   
private:
    std::string m_name;
    
    static std::vector<KRResource *> LoadObj(const std::string& path);
    static std::vector<KRResource *> LoadFbx(const std::string& path);
};

#endif
