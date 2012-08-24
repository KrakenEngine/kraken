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
#import "KRTexture.h"

class KRSkyBox : public KRNode {
    
public:
    
    KRSkyBox(KRContext &context, std::string name);
    virtual ~KRSkyBox();
    
    virtual std::string getElementName();
    virtual void loadXML(tinyxml2::XMLElement *e);
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRNode::RenderPass renderPass);
#endif
    
private:
    KRMat4 m_modelMatrix;
    
protected:
    std::string m_frontTexture;
    std::string m_backTexture;
    std::string m_topTexture;
    std::string m_bottomTexture;
    std::string m_leftTexture;
    std::string m_rightTexture;

    KRTexture *m_pFrontTexture;
    KRTexture *m_pBackTexture;
    KRTexture *m_pTopTexture;
    KRTexture *m_pBottomTexture;
    KRTexture *m_pLeftTexture;
    KRTexture *m_pRightTexture;
    
};


#endif