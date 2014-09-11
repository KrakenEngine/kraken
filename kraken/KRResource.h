//
//  KRResource.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"

#ifndef KRRESOURCE_H
#define KRRESOURCE_H

class KRResource : public KRContextObject
{
public:
    std::string getName();
    virtual std::string getExtension() = 0;
    virtual bool save(const std::string& path);
    virtual bool save(KRDataBlock &data) = 0;
    
    static std::string GetFileExtension(const std::string& name);
    static std::string GetFileBase(const std::string& name);
    static std::string GetFilePath(const std::string& name);
    
    virtual ~KRResource();
    
    
    static std::vector<KRResource *> LoadObj(KRContext &context, const std::string& path);
#if !TARGET_OS_IPHONE
    static void LoadFbx(KRContext &context, const std::string& path);
    static std::vector<KRResource *> LoadBlenderScene(KRContext &context, const std::string& path);
#endif
   
protected:
    KRResource(KRContext &context, std::string name);
   
   
private:
    std::string m_name;
    

};

#endif
