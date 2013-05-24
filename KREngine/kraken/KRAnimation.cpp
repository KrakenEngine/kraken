//
//  KRAnimation.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KRAnimation.h"
#include "KRAnimationManager.h"
#include "KRContext.h"
#include "KRNode.h"
#include "KRAnimationCurve.h"

#include "tinyxml2.h"

KRAnimation::KRAnimation(KRContext &context, std::string name) : KRResource(context, name)
{
    m_auto_play = false;
    m_loop = false;
    m_playing = false;
    m_local_time = 0.0f;
    m_duration = 0.0f;
}
KRAnimation::~KRAnimation()
{
    for(unordered_map<std::string, KRAnimationLayer *>::iterator itr = m_layers.begin(); itr != m_layers.end(); ++itr){
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

bool KRAnimation::save(KRDataBlock &data) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement *animation_node =  doc.NewElement( "animation" );
    doc.InsertEndChild(animation_node);
    animation_node->SetAttribute("loop", m_loop ? "true" : "false");
    animation_node->SetAttribute("auto_play", m_auto_play ? "true" : "false");
    animation_node->SetAttribute("duration", m_duration);
    
    for(unordered_map<std::string, KRAnimationLayer *>::iterator itr = m_layers.begin(); itr != m_layers.end(); ++itr){
        (*itr).second->saveXML(animation_node);
    }
    
    tinyxml2::XMLPrinter p;
    doc.Print(&p);
    data.append((void *)p.CStr(), strlen(p.CStr())+1);
    
    return true;
}

KRAnimation *KRAnimation::Load(KRContext &context, const std::string &name, KRDataBlock *data)
{
    data->append((void *)"\0", 1); // Ensure data is null terminated, to read as a string safely
    tinyxml2::XMLDocument doc;
    doc.Parse((char *)data->getStart());
    KRAnimation *new_animation = new KRAnimation(context, name);
    
    tinyxml2::XMLElement *animation_node = doc.RootElement();
    
    if(animation_node->QueryFloatAttribute("duration", &new_animation->m_duration) != tinyxml2::XML_SUCCESS) {
        new_animation->m_duration = 0.0f; // Default value
    }
    
    if(animation_node->QueryBoolAttribute("loop", &new_animation->m_loop) != tinyxml2::XML_SUCCESS) {
        new_animation->m_loop = false; // Default value
    }
    
    if(animation_node->QueryBoolAttribute("auto_play", &new_animation->m_auto_play) != tinyxml2::XML_SUCCESS) {
        new_animation->m_auto_play = false; // Default value
    }

    for(tinyxml2::XMLElement *child_element=animation_node->FirstChildElement(); child_element != NULL; child_element = child_element->NextSiblingElement()) {
        if(strcmp(child_element->Name(), "layer") == 0) {
            KRAnimationLayer *new_layer = new KRAnimationLayer(context);
            new_layer->loadXML(child_element);
            new_animation->m_layers[new_layer->getName()] = new_layer;
        }
    }
    
    if(new_animation->m_auto_play) {
        new_animation->m_playing = true;
    }
    
//    KRNode *n = KRNode::LoadXML(*new_scene, scene_element->FirstChildElement());
    
    delete data;
    return new_animation;
}


unordered_map<std::string, KRAnimationLayer *> &KRAnimation::getLayers()
{
    return m_layers;
}

KRAnimationLayer *KRAnimation::getLayer(const char *szName)
{
    return m_layers[szName];
}

void KRAnimation::update(float deltaTime)
{    
    if(m_playing) {
        m_local_time += deltaTime;
    }
    if(m_loop) {
        while(m_local_time > m_duration) {
            m_local_time -= m_duration;
        }
    } else if(m_local_time > m_duration) {
        m_local_time = m_duration;
        m_playing = false;
        getContext().getAnimationManager()->updateActiveAnimations(this);
    }
    
    for(unordered_map<std::string, KRAnimationLayer *>::iterator layer_itr = m_layers.begin(); layer_itr != m_layers.end(); layer_itr++) {
        KRAnimationLayer *layer = (*layer_itr).second;
        for(std::vector<KRAnimationAttribute *>::iterator attribute_itr = layer->getAttributes().begin(); attribute_itr != layer->getAttributes().end(); attribute_itr++) {
            KRAnimationAttribute *attribute = *attribute_itr;
            
            
            // TODO - Currently only a single layer supported per animation -- need to either implement combining of multiple layers or ask the FBX sdk to bake all layers into one
            KRAnimationCurve *curve = attribute->getCurve();
            KRNode *target = attribute->getTarget();
            KRNode::node_attribute_type attribute_type = attribute->getTargetAttribute();
            
            if(curve != NULL && target != NULL) {
                int frame_number = (int)(m_local_time * curve->getFrameRate());
                if(frame_number < curve->getFrameStart()) {
                    target->SetAttribute(attribute_type, curve->getValue(0));
                } else if(frame_number - curve->getFrameStart() >= curve->getFrameCount()) {
                    target->SetAttribute(attribute_type, curve->getValue(curve->getFrameCount() - 1));
                } else {
                    target->SetAttribute(attribute_type, curve->getValue(frame_number - curve->getFrameStart()));
                }
            }
            
        }
    }
}

void KRAnimation::Play()
{
    m_local_time = 0.0f;
    m_playing = true;
    getContext().getAnimationManager()->updateActiveAnimations(this);
}
void KRAnimation::Stop()
{
    m_playing = false;
    getContext().getAnimationManager()->updateActiveAnimations(this);
}

float KRAnimation::getTime()
{
    return m_local_time;
}

void KRAnimation::setTime(float time)
{
    m_local_time = time;
}

float KRAnimation::getDuration()
{
    return m_duration;
}

void KRAnimation::setDuration(float duration)
{
    m_duration = duration;
}

bool KRAnimation::isPlaying()
{
    return m_playing;
}

bool KRAnimation::getAutoPlay() const
{
    return m_auto_play;
}

void KRAnimation::setAutoPlay(bool auto_play)
{
    m_auto_play = auto_play;
}

bool KRAnimation::getLooping() const
{
    return m_loop;
}

void KRAnimation::setLooping(bool looping)
{
    m_loop = looping;
}
