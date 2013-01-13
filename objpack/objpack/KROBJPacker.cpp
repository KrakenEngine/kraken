//
//  objpacker.cpp
//  objpack
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

#import <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <vector.h>

#include "KROBJPacker.h"
#import <KREngine_osx/KRVector2.h>
#import <KREngine_osx/KRVector3.h>
#import <KREngine_osx/KRContext.h>

KROBJPacker::KROBJPacker() {
    
}

KROBJPacker::~KROBJPacker() {
    
}

void KROBJPacker::pack(const char *szPath) {
    std::cout << "Reading " << szPath << " ...\n";
    std::string base_path = KRResource::GetFilePath(szPath);
    
    KRContext context = KRContext();
    
    vector<KRResource *> resources = KRResource::Load(context, szPath);
    
    try {    
        vector<KRResource *>::iterator resource_itr = resources.begin();
        for(vector<KRResource *>::iterator resource_itr = resources.begin(); resource_itr != resources.end(); resource_itr++) {
            KRResource *pResource = (*resource_itr);
            std::string out_file_name = base_path;
            out_file_name.append("/output/");
            //out_file_name.append(pResource->GetFileBase(pResource->getName()));
            out_file_name.append(pResource->getName());
            out_file_name.append(".");
            out_file_name.append(pResource->getExtension());
            std::cout << "Writing " << out_file_name << " ... ";
            if(pResource->save(out_file_name)) {
                std::cout << " SUCCESS!\n";
            } else {
                std::cout << " FAIL...\n";
            }
        }
    } catch(...) {
        for(vector<KRResource *>::iterator resource_itr = resources.begin(); resource_itr != resources.end(); resource_itr++) {
            KRResource *pResource = (*resource_itr);
            delete pResource;
        }
        throw;
    }
    for(vector<KRResource *>::iterator resource_itr = resources.begin(); resource_itr != resources.end(); resource_itr++) {
        KRResource *pResource = (*resource_itr);
        delete pResource;
    }
}
