//
//  KRResource.h
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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
#include "KRContextObject.h"
#include "KRDataBlock.h"

#ifndef KRRESOURCE_H
#define KRRESOURCE_H
class KRBundle;
class KRScene;
class KRMesh;
class KRResource : public KRContextObject
{
public:
    std::string getName();
    virtual std::string getExtension() = 0;
    virtual bool save(const std::string& path);
    virtual bool save(KRDataBlock &data) = 0;

    KrResult moveToBundle(KRBundle* bundle);
    
    static std::string GetFileExtension(const std::string& name);
    static std::string GetFileBase(const std::string& name);
    static std::string GetFilePath(const std::string& name);
    
    virtual ~KRResource();
    
    static KRMesh* LoadObj(KRContext &context, const std::string& path);
#if !TARGET_OS_IPHONE
//    static KRScene* LoadFbx(KRContext &context, const std::string& path); TODO, FINDME, HACK! - Uncomment
    static KRScene* LoadBlenderScene(KRContext &context, const std::string& path);
#endif
   
protected:
    KRResource(KRContext &context, std::string name);
   
   
private:
    std::string m_name;
    

};

#endif
