//
//  KRBundle.cpp
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

#include "KRBundle.h"
#include "KRContext.h"
#include "KREngine-common.h"

const int KRENGINE_KRBUNDLE_HEADER_SIZE = 512;

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

KRBundle::KRBundle(KRContext &context, std::string name, KRDataBlock *pData) : KRResource(context, name)
{
    m_pData = pData;
    
    __int64_t file_pos = 0;
    while(file_pos < (__int64_t)m_pData->getSize()) {
        tar_header_type file_header;
        m_pData->copy(&file_header, (int)file_pos, sizeof(file_header));
        size_t file_size = strtol(file_header.file_size, NULL, 8);
        file_pos += 512; // Skip past the header to the file contents
        if(file_header.file_name[0] != '\0' && file_header.file_name[0] != '.') {
            // We ignore the last two records in the tar file, which are zero'ed out tar_header structures
            KRDataBlock *pFileData = pData->getSubBlock((int)file_pos, (int)file_size);
            context.loadResource(file_header.file_name, pFileData);
        }
        file_pos += RoundUpSize(file_size);
    }
}

KRBundle::KRBundle(KRContext &context, std::string name) : KRResource(context, name)
{
    // Create an empty krbundle (tar) file, initialized with two zero-ed out file headers, which terminate it.
    m_pData = new KRDataBlock();
    m_pData->expand(KRENGINE_KRBUNDLE_HEADER_SIZE * 2);
    m_pData->lock();
    memset(m_pData->getStart(), 0, m_pData->getSize());
    m_pData->unlock();
}

size_t KRBundle::RoundUpSize(size_t s)
{
    // Get amount of padding needed to increase s to a 512 byte alignment
    if((s & 0x01ff) == 0) {
        // file size is a multiple of 512 bytes, we can just add it
        return s;
    } else {
        // We would not be on a 512 byte boundary, round up to the next one
        return (s + 0x0200) - (s & 0x1ff);
    }

}

KRBundle::~KRBundle()
{
    delete m_pData;
}

std::string KRBundle::getExtension()
{
    return "krbundle";
}

bool KRBundle::save(const std::string& path)
{
    return m_pData->save(path);
}

bool KRBundle::save(KRDataBlock &data) {
    if(m_pData->getSize() > KRENGINE_KRBUNDLE_HEADER_SIZE * 2) {
        // Only output krbundles that contain files
        data.append(*m_pData);
    }
    return true;
}

KRDataBlock* KRBundle::append(KRResource &resource)
{
    // Serialize resource to binary representation
    KRDataBlock resource_data;
    resource.save(resource_data);
    
    std::string file_name = resource.getName() + "." + resource.getExtension();
    
    // Padding is added at the end of file to align next header to a 512 byte boundary.  Padding at the end of the archive includes an additional 1024 bytes -- two zero-ed out file headers that mark the end of the archive
    size_t padding_size = RoundUpSize(resource_data.getSize()) - resource_data.getSize() + KRENGINE_KRBUNDLE_HEADER_SIZE * 2;
    size_t resource_data_start = m_pData->getSize() + KRENGINE_KRBUNDLE_HEADER_SIZE;
    m_pData->expand(KRENGINE_KRBUNDLE_HEADER_SIZE + resource_data.getSize() + padding_size - KRENGINE_KRBUNDLE_HEADER_SIZE * 2); // We will overwrite the existing zero-ed out file headers that marked the end of the archive, so we don't have to include their size here
    
    m_pData->lock();
    
    // Get location of file header
    tar_header_type *file_header = (tar_header_type *)((unsigned char *)m_pData->getEnd() - padding_size - resource_data.getSize() - KRENGINE_KRBUNDLE_HEADER_SIZE);
    
    // Zero out new file header
    memset(file_header, 0, KRENGINE_KRBUNDLE_HEADER_SIZE);
    
    // Copy resource data
    resource_data.lock();
    memcpy((unsigned char *)m_pData->getEnd() - padding_size - resource_data.getSize(), resource_data.getStart(), resource_data.getSize());
    resource_data.unlock();
    
    // Zero out alignment padding and terminating set of file header blocks
    memset((unsigned char *)m_pData->getEnd() - padding_size, 0, padding_size);
    
    // Populate new file header fields
    strncpy(file_header->file_name, file_name.c_str(), 100);
    strcpy(file_header->file_mode, "000644 ");
    strcpy(file_header->owner_id, "000000 ");
    strcpy(file_header->group_id, "000000 ");
    sprintf(file_header->file_size, "%011o", (int)resource_data.getSize());
    file_header->file_size[11] = ' '; // Terminate with space rather than '\0'
    sprintf(file_header->mod_time, "%011o", (int)time(NULL));
    file_header->mod_time[11] = ' '; // Terminate with space rather than '\0'
    
    // Calculate and write checksum for header
    memset(file_header->checksum, ' ', 8); // Must be filled with spaces and no null terminator during checksum calculation
    int check_sum = 0;
    for(int i=0; i < KRENGINE_KRBUNDLE_HEADER_SIZE; i++) {
        unsigned char *byte_ptr = (unsigned char *)file_header;
        check_sum += byte_ptr[i];
    }
    sprintf(file_header->checksum, "%07o", check_sum);
    
    m_pData->unlock();

    KRDataBlock *pFileData = m_pData->getSubBlock((int)resource_data_start, (int)resource_data.getSize());
    return pFileData;
}
