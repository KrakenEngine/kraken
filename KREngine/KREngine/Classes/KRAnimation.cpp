//
//  KRAnimation.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-11-30.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KRAnimation.h"
#import "tinyxml2.h"

KRAnimation::KRAnimation(KRContext &context, std::string name) : KRResource(context, name)
{

}
KRAnimation::~KRAnimation()
{
    for(std::map<std::string, KRAnimationLayer *>::iterator itr = m_layers.begin(); itr != m_layers.end(); ++itr){
        delete (*itr).second;
    }
}

std::string KRAnimation::getExtension() {
    return "kranimation";
}

void KRAnimation::addLayer(KRAnimationLayer *layer)
{
    m_layers[layer->getName()] = layer;
}

bool KRAnimation::save(const std::string& path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *animation_node =  doc.NewElement( "animation" );
    doc.InsertEndChild(animation_node);
//    m_pRootNode->saveXML(animation_node);
    
    for(std::map<std::string, KRAnimationLayer *>::iterator itr = m_layers.begin(); itr != m_layers.end(); ++itr){
        (*itr).second->saveXML(animation_node);
    }
    
    doc.SaveFile(path.c_str());
    return true;
}


KRAnimation *KRAnimation::Load(KRContext &context, const std::string &name, KRDataBlock *data)
{
    data->append((void *)"\0", 1); // Ensure data is null terminated, to read as a string safely
    tinyxml2::XMLDocument doc;
    doc.Parse((char *)data->getStart());
    KRAnimation *new_animation = new KRAnimation(context, name);
    
    tinyxml2::XMLElement *animation_node = doc.RootElement();

    for(tinyxml2::XMLElement *child_element=animation_node->FirstChildElement(); child_element != NULL; child_element = child_element->NextSiblingElement()) {
        if(strcmp(child_element->Name(), "layer") == 0) {
            KRAnimationLayer *new_layer = new KRAnimationLayer(context);
            new_animation->m_layers[new_layer->getName()] = new_layer;
        }
    }
    
//    KRNode *n = KRNode::LoadXML(*new_scene, scene_element->FirstChildElement());
    
    delete data;
    return new_animation;
}

