//
//  KRWorld.h
//  KREngine
//
//  Created by Kearwood Gilbert on 12-05-11.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KREngine_KRWorld_h
#define KREngine_KRWorld_h

class KRDataBlock;

class KRWorld {
public:
    KRWorld();
    ~KRWorld();
    
    void Load(void *data, int data_size);
    KRDataBlock *Save();
private:
};

#endif
