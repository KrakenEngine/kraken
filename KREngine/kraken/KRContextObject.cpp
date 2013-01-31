//
//  KRContextObject.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-16.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRContextObject.h"

KRContextObject::KRContextObject(KRContext &context)
{
    m_pContext = &context;
}

KRContextObject::~KRContextObject()
{
    
}

KRContext &KRContextObject::getContext() const
{
    return *m_pContext;
}