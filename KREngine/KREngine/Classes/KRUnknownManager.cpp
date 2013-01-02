//
//  FileManager.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-02.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRUnknownManager.h"

KRUnknownManager::KRUnknownManager(KRContext &context) : KRContextObject(context)
{
    
}

KRUnknownManager::~KRUnknownManager()
{
    for(map<std::string, map<std::string, KRUnknown *> >::iterator extension_itr = m_unknowns.begin(); extension_itr != m_unknowns.end(); extension_itr++) {
        for(map<std::string, KRUnknown *>::iterator name_itr=(*extension_itr).second.begin(); name_itr != (*extension_itr).second.end(); name_itr++) {
            delete (*name_itr).second;
        }
    }
}

void KRUnknownManager::add(KRUnknown *unknown)
{
    std::string lower_name = unknown->getName();
    std::string lower_extension = unknown->getExtension();
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    map<std::string, map<std::string, KRUnknown *> >::iterator extension_itr = m_unknowns.find(lower_extension);
    if(extension_itr == m_unknowns.end()) {
        m_unknowns[lower_name] = map<std::string, KRUnknown *>();
        extension_itr = m_unknowns.find(lower_extension);
    }
    
    map<std::string, KRUnknown *>::iterator name_itr = (*extension_itr).second.find(lower_name);
    if(name_itr != (*extension_itr).second.end()) {
        delete (*name_itr).second;
        (*name_itr).second = unknown;
    } else {
        (*extension_itr).second[lower_extension] = unknown;
    }
}

KRUnknown *KRUnknownManager::load(const std::string &name, const std::string &extension, KRDataBlock *data)
{
    KRUnknown *unknown = new KRUnknown(getContext(), name, extension, data);
    if(unknown) add(unknown);
    return unknown;
}

KRUnknown *KRUnknownManager::get(const std::string &name, const std::string &extension)
{
    std::string lower_name = name;
    std::string lower_extension = extension;
    
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    
    return m_unknowns[lower_extension][lower_name];
}


const map<std::string, KRUnknown *> &KRUnknownManager::get(const std::string &extension)
{
    std::string lower_extension = extension;
    std::transform(lower_extension.begin(), lower_extension.end(), lower_extension.begin(), ::tolower);
    return m_unknowns[lower_extension];
}

