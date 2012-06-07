//
//  KRDataBlock.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRDataBlock_h
#define KREngine_KRDataBlock_h

class KRDataBlock {
public:
    KRDataBlock();
    ~KRDataBlock();
    
    void append(void *data, int size);
    
    void *getData();
    int getSize();
private:
    void *m_data;
    int m_data_size;
};

#endif
