//
//  KRResource.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

#include "KREngine-common.h"
#include "KRResourceBinding.h"
#include "KRContext.h"

KRResourceBinding::KRResourceBinding(std::string& name)
  : m_name(name)
  , m_resource(nullptr)
{
}

KRResourceBinding::KRResourceBinding()
  : m_resource(nullptr)
{
}

KRResourceBinding::~KRResourceBinding()
{
  m_resource = nullptr;
  m_name.clear();
}

KRResource* KRResourceBinding::get()
{
  return m_resource;
}

void KRResourceBinding::set(KRResource* resource)
{
  if (resource == nullptr) {
    m_resource = nullptr;
    m_name.clear();
    return;
  }

  m_resource = resource;
  m_name = resource->getName();
}


bool KRResourceBinding::isSet() const
{
  return m_name.size() > 0;
}

const std::string& KRResourceBinding::getName() const
{
  return m_name;
}

void KRResourceBinding::setName(const std::string& name)
{
  if (m_name == name) {
    return;
  }
  m_name = name;
  m_resource = nullptr;
}

void KRResourceBinding::clear()
{
  set(nullptr);
}

bool KRResourceBinding::isLoaded() const
{
  return m_resource != nullptr;
}