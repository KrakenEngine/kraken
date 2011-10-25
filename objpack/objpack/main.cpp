//
//  main.cpp
//  objpack
//
//  Created by Kearwood Gilbert on 11-04-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KROBJPacker.h"

int main (int argc, const char * argv[])
{

    if(argc < 2) {
        std::cout << "You must pass an .obj file as a parameter.  An .obj.pack file will be written for each .obj file.\n";
    } else {
        KROBJPacker p;
        for(int i=1; i < argc; i++) {
            std::cout << "Packing " << argv[i] << " ...\n";
            p.pack(argv[i]);
        }
        std::cout << "Done.\n";
    }
    
    return 0;
}

