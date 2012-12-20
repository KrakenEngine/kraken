//
//  KRResource.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRResource.h"

KRResource::KRResource(KRContext &context, std::string name) : KRContextObject(context) {
    m_name = name;
}
KRResource::~KRResource() {
    
}

std::string KRResource::getName()
{
    return m_name;
}

std::string KRResource::GetFileExtension(const std::string& name)
{
    if(name.find_last_of(".") != std::string::npos) {
        return name.substr(name.find_last_of(".")+1);
    } else {
        return "";
    }
}

std::string KRResource::GetFileBase(const std::string& name)
{
    std::string f = name;
    // Strip off directory
    if(f.find_last_of("/") != std::string::npos) {
        f = f.substr(f.find_last_of("/") + 1);
    }
    
    // Strip off extension
    if(f.find_last_of(".") != std::string::npos) {
        f = f.substr(0, f.find_last_of("."));
    }
       
    return f;
}

std::string KRResource::GetFilePath(const std::string& name)
{
    if(name.find_last_of("/") != std::string::npos) {
        return name.substr(0, name.find_last_of("/"));
    } else {
        return "";
    }
}

std::vector<KRResource *> KRResource::Load(KRContext &context, const std::string& path)
{
    std::vector<KRResource *> resources;
    std::string extension = GetFileExtension(path);
    if(extension.compare("obj") == 0) {
        return LoadObj(context, path);
#if TARGET_OS_MAC
    } else if(extension.compare("fbx") == 0) {
        return LoadFbx(context, path);
    } else if(extension.compare("blend") == 0) {
        return LoadBlenderScene(context, path);
#endif
    }
    
    return resources;
}

bool KRResource::save(const std::string& path)
{
    KRDataBlock data;
    if(save(data)) {
        return data.save(path);
    } else {
        return false;
    }
}