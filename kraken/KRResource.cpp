//
//  KRResource.cpp
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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
#include "KRResource.h"
#include "KRBundle.h"
#include "KRContext.h"

KRResource::KRResource(KRContext &context, std::string name) : KRContextObject(context) {
    m_name = name;
    context.addResource(this, name);
}
KRResource::~KRResource() {
  m_pContext->removeResource(this);
}

std::string KRResource::getName()
{
    return m_name;
}

std::string KRResource::GetFileExtension(const std::string& name)
{
    if(name.find_last_of(".") != std::string::npos) {
        return name.substr(name.find_last_of(".")+1);
    } else {
        return "";
    }
}

std::string KRResource::GetFileBase(const std::string& name)
{
    std::string f = name;

    // Normalize Windows Paths
    std::replace(f.begin(), f.end(), '\\', '/');

    // Strip off directory
    if(f.find_last_of("/") != std::string::npos) {
        f = f.substr(f.find_last_of("/") + 1);
    }
    
    // Strip off extension
    if(f.find_last_of(".") != std::string::npos) {
        f = f.substr(0, f.find_last_of("."));
    }
       
    return f;
}

std::string KRResource::GetFilePath(const std::string& name)
{
    if(name.find_last_of("/") != std::string::npos) {
        return name.substr(0, name.find_last_of("/"));
    } else {
        return "";
    }
}

bool KRResource::save(const std::string& path)
{
    KRDataBlock data;
    if(save(data)) {
        return data.save(path);
    } else {
        return false;
    }
}

KrResult KRResource::moveToBundle(KRBundle* bundle)
{
  KRDataBlock* data = bundle->append(*this);
  if (data == nullptr) {
    return KR_ERROR_UNEXPECTED;
  }
  // TODO(kearwood) - We must re-attach the KRResource to the returned KRDataBlock
  delete data;
  return KR_SUCCESS;
}
