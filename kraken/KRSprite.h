//
//  KRSprite.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRSPRITE_H
#define KRSPRITE_H

#include "KRResource.h"
#include "KRNode.h"
#include "KRTexture.h"

class KRSprite : public KRNode {
public:
    static void InitNodeInfo(KrNodeInfo* nodeInfo);

    KRSprite(KRScene &scene, std::string name);
    
    virtual ~KRSprite();
    virtual std::string getElementName();
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    void setSpriteTexture(std::string sprite_texture);
    void setSpriteAlpha(float alpha);
    float getSpriteAlpha() const;
    
    virtual void render(KRCamera *pCamera, std::vector<KRPointLight *> &point_lights, std::vector<KRDirectionalLight *> &directional_lights, std::vector<KRSpotLight *>&spot_lights, const KRViewport &viewport, KRNode::RenderPass renderPass);
    
    virtual AABB getBounds();
    
protected:
    
    std::string m_spriteTexture;
    KRTexture *m_pSpriteTexture;
    float m_spriteAlpha;
};

#endif
