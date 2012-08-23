//
//  KRSkyBox.h
//  KREngine
//
//  Created by Michael Ilich on 2012-08-23.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef __KREngine__KRSkyBox__
#define __KREngine__KRSkyBox__

#include <iostream>
#import "KRMat4.h"
#import "KRNode.h"

class KRSkyBox : public KRNode {
    
public:
    
    KRSkyBox(KRContext &context, std::string name);
    virtual ~KRSkyBox();
    
    virtual std::string getElementName();
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRNode::RenderPass renderPass);
#endif
    
private:
    KRMat4 m_modelMatrix;
};


#endif