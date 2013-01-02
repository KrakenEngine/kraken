//
//  KRUnknown.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-02.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#include "KRUnknown.h"

KRUnknown::KRUnknown(KRContext &context, std::string name, std::string extension) : KRResource(context, name)
{
    m_pData = new KRDataBlock();
    m_extension = extension;
}

KRUnknown::KRUnknown(KRContext &context, std::string name, std::string extension, KRDataBlock *data) : KRResource(context, name)
{
    m_pData = data;
    m_extension = extension;
}

KRUnknown::~KRUnknown()
{
    delete m_pData;
}

std::string KRUnknown::getExtension()
{
    return m_extension;
}

bool KRUnknown::save(KRDataBlock &data)
{
    data.append(*m_pData);
    return true;
}

KRDataBlock *KRUnknown::getData()
{
    return m_pData;
}
