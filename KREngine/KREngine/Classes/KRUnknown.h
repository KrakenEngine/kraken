//
//  KRUnknown.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2013-01-02.
//  Copyright (c) 2013 Kearwood Software. All rights reserved.
//

#ifndef KRUNKNOWN_H
#define KRUNKNOWN_H

#import "KREngine-common.h"
#import "KRContextObject.h"
#import "KRDataBlock.h"
#import "KRResource.h"
#import <map>

class KRUnknown : public KRResource {
    
public:
    KRUnknown(KRContext &context, std::string name, std::string extension);
    KRUnknown(KRContext &context, std::string name, std::string extension, KRDataBlock *data);
    virtual ~KRUnknown();
    
    virtual std::string getExtension();
    
    virtual bool save(KRDataBlock &data);
    
    KRDataBlock *getData();

private:
    
    std::string m_extension;
    KRDataBlock *m_pData;
};

#endif /* defined(KRUNKNOWN_H) */
