//
//  KRUnknownManager.h
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
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

#pragma once

#include "KREngine-common.h"

#include "resources/KRResourceManager.h"

#include "KRUnknown.h"
#include "KRContextObject.h"
#include "block.h"

class KRUnknownManager : public KRResourceManager
{
public:
  KRUnknownManager(KRContext& context);
  virtual ~KRUnknownManager();

  virtual KRResource* loadResource(const std::string& name, const std::string& extension, mimir::Block* data) override;
  virtual KRResource* getResource(const std::string& name, const std::string& extension) override;

  void add(KRUnknown* unknown);

  KRUnknown* load(const std::string& name, const std::string& extension, mimir::Block* data);
  KRUnknown* get(const std::string& name, const std::string& extension);


  const unordered_map<std::string, KRUnknown*>& get(const std::string& extension);

  unordered_map<std::string, unordered_map<std::string, KRUnknown*> >& getUnknowns();

private:
  unordered_map<std::string, unordered_map<std::string, KRUnknown*> > m_unknowns;
};
