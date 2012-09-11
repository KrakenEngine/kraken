//
//  KRBundle.cpp
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

#include "KRBundle.h"
#include "KRContext.h"
#include <assert.h>

typedef struct _tar_header
{
    char file_name[100];
    char file_mode[8];
    char owner_id[8]; // Owner's numeric user ID (OCTAL!)
    char group_id[8]; // Group's numeric user ID (OCTAL!)
    char file_size[12]; // File size in bytes (OCTAL!)
    char mod_time[12]; // Last modification time in numeric Unix time format
    char checksum[8]; // Checksum for header block
    char file_type[1]; // Link indicator (file type)
    char linked_file[100]; // Name of linked file
    
} tar_header_type;

KRBundle::KRBundle(KRContext &context, std::string name, KRDataBlock *pData) : KRContextObject(context)
{
    m_pData = pData;
    
    unsigned char *pFile = (unsigned char *)m_pData->getStart();
    while(pFile < m_pData->getEnd() ) {
        tar_header_type *file_header = (tar_header_type *)pFile;
        size_t file_size = strtol(file_header->file_size, NULL, 8);
        pFile += 512; // Skip past the header to the file contents
        
        if(file_header->file_name[0] != '\0' && file_header->file_name[0] != '.') {
            // We ignore the last two records in the tar file, which are zero'ed out tar_header structures
            KRDataBlock *pFileData = new KRDataBlock();
            if(pFileData->load(pFile, file_size)) {
                context.loadResource(file_header->file_name, pFileData);
            } else {
                delete pFileData;
                assert(false);
            }
        }
        
         // Advance past the end of the file
        if((file_size & 0x01ff) == 0) {
            // file size is a multiple of 512 bytes, we can just add it
            pFile += file_size;
        } else {
            // We would not be on a 512 byte boundary, round up to the next one
            pFile += (file_size + 0x0200) - (file_size & 0x1ff);
        }
        
    }
}

KRBundle::~KRBundle()
{
    delete m_pData;
}
