//
//  KRAnimation.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRANIMATION_H
#define KRANIMATION_H

#import "KREngine-common.h"
#import "KRContextObject.h"
#import "KRDataBlock.h"
#import "KRResource.h"
#import "KRAnimationLayer.h"
#import <map>

class KRAnimation : public KRResource {
    
public:
    KRAnimation(KRContext &context, std::string name);
    virtual ~KRAnimation();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    
    static KRAnimation *Load(KRContext &context, const std::string &name, KRDataBlock *data);
    
    void addLayer(KRAnimationLayer *layer);
    
private:
    std::map<std::string, KRAnimationLayer *> m_layers;
};



#endif
