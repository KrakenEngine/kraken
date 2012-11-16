//
//  KRPointLight.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRPointLight_h
#define KREngine_KRPointLight_h

#import "KRLight.h"
#import "KRMat4.h"

class KRPointLight : public KRLight {
    
public:
    
    KRPointLight(KRScene &scene, std::string name);
    virtual ~KRPointLight();
    
    virtual std::string getElementName();
    virtual KRAABB getBounds();
    
#if TARGET_OS_IPHONE
    
    virtual void render(KRCamera *pCamera, std::vector<KRLight *> &lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
#endif
    
private:
    void generateMesh();
    
    GLfloat *m_sphereVertices;
    int m_cVertices;
};

#endif
