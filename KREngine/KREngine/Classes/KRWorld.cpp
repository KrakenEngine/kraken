//
//  KRWorld.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>

#include "KRWorld.h"
#include "KRDataBlock.h"


KRWorld::KRWorld() {
    
}
KRWorld::~KRWorld() {
    
}

void KRWorld::Load(void *data, int data_size) {
    
}

KRDataBlock *KRWorld::Save() {
    KRDataBlock *block = new KRDataBlock();
    return block;
}
