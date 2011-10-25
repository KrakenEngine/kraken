//
//  KRTexture.h
//  gldemo
//
//  Created by Kearwood Gilbert on 10-10-23.
//  Copyright (c) 2010 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <stdint.h>
#import <list>
#import <string>

using std::list;

#ifndef KRTEXTURE_H
#define KRTEXTURE_H

class KRTexture {
public:
    KRTexture();
    ~KRTexture();
    
    bool loadFromFile(const char *szFile);
    bool createGLTexture();
    GLuint getName();
    
private:
    
    GLuint    m_iName;
    uint32_t  m_iWidth;
    uint32_t  m_iHeight;
    GLenum    m_internalFormat;
    bool      m_bHasAlpha;
    
    struct    dataBlockStruct {
        void *start;
        uint32_t length;
    };
    
    std::list<dataBlockStruct> m_blocks;
    
    int m_fdFile;
    int m_fileSize;
    void *m_pFile;
};

#endif
