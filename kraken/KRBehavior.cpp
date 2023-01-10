//
//  KRBehavior.cpp
//  Kraken Engine
//
//  Copyright 2023 Kearwood Gilbert. All rights reserved.
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

#include "KRBehavior.h"
#include "KRNode.h"

KRBehaviorFactoryFunctionMap m_factoryFunctions;

KRBehavior::KRBehavior()
{
  __node = NULL;
}

KRBehavior::~KRBehavior()
{

}

void KRBehavior::init()
{
  // Note: Subclasses are not expected to call this method
}

KRNode* KRBehavior::getNode() const
{
  return __node;
}

void KRBehavior::__setNode(KRNode* node)
{
  __node = node;
}


KRBehavior* KRBehavior::LoadXML(KRNode* node, tinyxml2::XMLElement* e)
{
  std::map<std::string, std::string> attributes;
  for (const tinyxml2::XMLAttribute* attribute = e->FirstAttribute(); attribute != NULL; attribute = attribute->Next()) {
    attributes[attribute->Name()] = attribute->Value();
  }

  const char* szElementName = e->Attribute("type");
  if (szElementName == NULL) {
    return NULL;
  }
  KRBehaviorFactoryFunctionMap::const_iterator itr = m_factoryFunctions.find(szElementName);
  if (itr == m_factoryFunctions.end()) {
    return NULL;
  }
  return (*itr->second)(attributes);
}

void KRBehavior::RegisterFactoryCTOR(std::string behaviorName, KRBehaviorFactoryFunction fnFactory)
{
  m_factoryFunctions[behaviorName] = fnFactory;
}

void KRBehavior::UnregisterFactoryCTOR(std::string behaviorName)
{
  m_factoryFunctions.erase(behaviorName);
}
