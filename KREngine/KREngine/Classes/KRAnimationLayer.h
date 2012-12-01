//
//  KRAnimationLayer.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRANIMATIONLAYER_H
#define KRANIMATIONLAYER_H

#import "KRContextObject.h"
#import "KREngine-common.h"
#import "tinyxml2.h"

class KRAnimationLayer : public KRContextObject {
public:
    KRAnimationLayer(KRContext &context);
    ~KRAnimationLayer();
    
    tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    void loadXML(tinyxml2::XMLElement *e);
    
    std::string getName() const;
    void setName(const std::string &name);
    
    float getWeight() const;
    void setWeight(float weight);
    
    typedef enum {
        KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE,
        KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE,
        KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE_PASSTHROUGH
    } blend_mode_t;
    
    blend_mode_t getBlendMode() const;
    void setBlendMode(const blend_mode_t &blend_mode);
    
    typedef enum {
        KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER,
        KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_CHANNEL
    } rotation_accumulation_mode_t;
    
    rotation_accumulation_mode_t getRotationAccumulationMode() const;
    void setRotationAccumulationMode(const rotation_accumulation_mode_t &rotation_accumulation_mode);
    
    typedef enum {
        KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY,
        KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE
    } scale_accumulation_mode_t;
    
    scale_accumulation_mode_t getScaleAccumulationMode() const;
    void setScaleAccumulationMode(const scale_accumulation_mode_t &scale_accumulation_mode);

    
private:
    std::string m_name;
    float m_weight;
    blend_mode_t m_blend_mode;
    rotation_accumulation_mode_t m_rotation_accumulation_mode;
    scale_accumulation_mode_t m_scale_accumulation_mode;
};

#endif
