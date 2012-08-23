//
//  KRContextObject.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-08-16.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRCONTEXTOBJECT_H
#define KRCONTEXTOBJECT_H

class KRContext;

class KRContextObject {

public:
    KRContextObject(KRContext &context);
    ~KRContextObject();
    
    KRContext &getContext();
protected:
    KRContext *m_pContext;
};

#endif
