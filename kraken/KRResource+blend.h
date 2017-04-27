//
//  KRResource+blend.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-08.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRRESOURCE_BLEND_H
#define KRRESOURCE_BLEND_H

class KRBlendFile {
public:
    KRBlendFile(const void *pFile);
    ~KRBlendFile();
    
    class Block {
    public:
        Block(KRBlendFile *blendFile, unsigned char *&scan);
        ~Block();
        
        std::string getCode();
        int getDataSize();
    private:
        std::string m_code;
        __int32_t m_dataSize;
        __int32_t m_sdna_index;
        __int32_t m_structure_count;
        __int64_t m_prev_pointer;
        unsigned char *m_data;
    };
    
private:
    enum file_type {
        KRBLEND_LITTLEENDIAN_32BIT,
        KRBLEND_LITTLEENDIAN_64BIT,
        KRBLEND_BIGENDIAN_32BIT,
        KRBLEND_BIGENDIAN_64BIT
    } m_file_type;
    void readHeader(unsigned char *&scan);
    
    __int32_t readInt(unsigned char *&scan);
    __int64_t readPointer(unsigned char *&scan);
    
    std::vector<Block> m_blocks;
};



#endif
