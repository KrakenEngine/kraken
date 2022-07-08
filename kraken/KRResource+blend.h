//
//  KRResource+blend.h
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

#pragma once

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
