//
//  KRDataBlock.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRDataBlock.h"

KRDataBlock::KRDataBlock() {
    m_data = NULL;
    m_data_size = 0;
}

KRDataBlock::~KRDataBlock() {
    if(m_data) {
        free(m_data);
    }
}

void *KRDataBlock::getData() {
    return m_data;
}

int KRDataBlock::getSize() {
    return m_data_size;
}

void KRDataBlock::append(void *data, int size) {
    if(m_data == NULL) {
        m_data = malloc(size);
        memcpy(m_data, data, size);
        m_data_size = size;
    } else {
        m_data = realloc(m_data, m_data_size + size);
        memcpy((unsigned char *)m_data + size, data, size);
        m_data_size += size;
    }
}