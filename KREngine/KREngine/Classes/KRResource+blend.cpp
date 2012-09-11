//
//  KRResource+blend.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-08.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "KRResource.h"
#include "KRScene.h"
#include "KRResource+blend.h"


std::vector<KRResource *> KRResource::LoadBlenderScene(KRContext &context, const std::string& path) {
    std::vector<KRResource *> resources;
    KRScene *pScene = new KRScene(context, KRResource::GetFileBase(path));
    resources.push_back(pScene);
    
    KRDataBlock data;
    
    if(data.load(path)) {
        KRBlendFile blend_file = KRBlendFile(pFile);
    }
    
    return resources;
}


KRBlendFile::KRBlendFile(const void *pFile) {
    unsigned char *scan = (unsigned char *)pFile;
    readHeader(scan);
    std::string block_code = "";
    while(block_code != "ENDB") {
        Block b = Block(this, scan);
        block_code = b.getCode();
        m_blocks.push_back(b);
        
        printf("Loaded block: %s - %i bytes\n", b.getCode().c_str(), b.getDataSize());
    }
}

void KRBlendFile::readHeader(unsigned char *&scan) {
    if(strncmp((char *)scan, "BLENDER", 7) != 0) {
        // TODO throw exception 
    }
    scan += 7;
    if(scan[0] == '_' && scan[1] == 'v') {
        // 32-bit, little-endian
        m_file_type = KRBLEND_LITTLEENDIAN_32BIT;
    } else if(scan[0] == '_' && scan[1] == 'V') {
        // 32-bit, bit-endian
        m_file_type = KRBLEND_BIGENDIAN_32BIT;
    } else if(scan[0] == '-' && scan[1] == 'v') {
        // 64-bit, little-endian
        m_file_type = KRBLEND_LITTLEENDIAN_64BIT;
    } else if(scan[0] == '-' && scan[1] == 'V') {
        // 64-bit, big-endian
        m_file_type = KRBLEND_BIGENDIAN_64BIT;
    } else {
        // TODO - throw exception
    }
    scan += 5; // Skip and ignore version
}

__int32_t KRBlendFile::readInt(unsigned char *&scan) {
    __int32_t ret = 0;
    // read a 32-bit integer and increment scan
    
    switch (m_file_type) {
        case KRBLEND_BIGENDIAN_32BIT:
        case KRBLEND_BIGENDIAN_64BIT:
            ret = (__int32_t)scan[3] + (__int32_t)scan[2] * 0x100 + (__int32_t)scan[1] * 0x10000 + (__int32_t)scan[0] * 0x1000000;
            break;
        case KRBLEND_LITTLEENDIAN_32BIT:
        case KRBLEND_LITTLEENDIAN_64BIT:
            ret = (__int32_t)scan[0] + (__int32_t)scan[1] * 0x100 + (__int32_t)scan[2] * 0x10000 + (__int32_t)scan[3] * 0x1000000;
            break;
    }
    
    scan += 4;
    return ret;
}

__int64_t KRBlendFile::readPointer(unsigned char *&scan) {
    __int64_t ret = 0;
    // read a 32-bit integer and increment scan
    switch (m_file_type) {
        case KRBLEND_BIGENDIAN_32BIT:
            ret = scan[3] + scan[2] * 0x100 + scan[1] * 0x10000 + scan[0] * 0x1000000;
            scan += 4;
            break;
        case KRBLEND_LITTLEENDIAN_32BIT:
            ret = scan[0] + scan[1] * 0x100 + scan[2] * 0x10000 + scan[3] * 0x1000000;
            scan += 4;
            break;
            
        case KRBLEND_BIGENDIAN_64BIT:
            ret = scan[7] + scan[6] * 0x100 + scan[5] * 0x10000 + scan[4] * 0x1000000 + scan[3] * 0x100000000 + scan[2] * 0x10000000000  + scan[1] * 0x1000000000000 + scan[0] * 0x100000000000000;
            scan += 8;
            break;
        case KRBLEND_LITTLEENDIAN_64BIT:
            ret = scan[0] + scan[1] * 0x100 + scan[2] * 0x10000 + scan[3] * 0x1000000 + scan[4] * 0x100000000 + scan[5] * 0x10000000000  + scan[6] * 0x1000000000000 + scan[7] * 0x100000000000000;
            scan += 8;
            break;
    }
    
    
    return ret;
}

KRBlendFile::~KRBlendFile() {
    
}


KRBlendFile::Block::Block(KRBlendFile *blendFile, unsigned char *&scan) {
    scan += (__int64_t)scan % 4; // Scan forward until the next 4-byte boundary
    char szBlock[5];
    szBlock[0] = *scan++;
    szBlock[1] = *scan++;
    szBlock[2] = *scan++;
    szBlock[3] = *scan++;
    szBlock[4] = '\0';
    m_code = szBlock;
    m_dataSize = blendFile->readInt(scan);
    m_prev_pointer = blendFile->readPointer(scan);
    m_sdna_index = blendFile->readInt(scan);
    m_structure_count = blendFile->readInt(scan);
    m_data = scan;
    scan += m_dataSize;
}
KRBlendFile::Block::~Block() {
    
}

std::string KRBlendFile::Block::getCode() {
    return m_code;
}

int KRBlendFile::Block::getDataSize() {
    return m_dataSize;
}

