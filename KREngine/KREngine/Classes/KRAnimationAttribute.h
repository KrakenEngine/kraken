//
//  KRAnimationAttribute.h
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

#ifndef KRANIMATIONATTRIBUTE_H
#define KRANIMATIONATTRIBUTE_H

#import "KRContextObject.h"
#import "KREngine-common.h"
#import "tinyxml2.h"
#import "KRNode.h"

class KRAnimationAttribute : public KRContextObject {
public:
    KRAnimationAttribute(KRContext &context);
    ~KRAnimationAttribute();
    
    tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);
    void loadXML(tinyxml2::XMLElement *e);
    
    std::string getCurveName() const;
    void setCurveName(const std::string &curve_name);
    
    std::string getTargetName() const;
    void setTargetName(const std::string &target_name);
    
    std::string getTargetAttributeName() const;
    void setTargetAttributeName(const std::string &target_attribute_name);
    
private:
    std::string m_target_name;
    std::string m_curve_name;
    KRNode::node_attribute_type m_node_attribute;
    std::string m_target_attribute_name;
};

#endif
