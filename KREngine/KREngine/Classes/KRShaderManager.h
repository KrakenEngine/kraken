//
//  KRShaderManager.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-08-11.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <stdint.h>
#import <vector>
#import <map>
#import <string>
#import "KRCamera.h"

using std::map;
using std::vector;

#include "KRShader.h"

#ifndef KRSHADERMANAGER_H
#define KRSHADERMANAGER_H

#define KRENGINE_MAX_SHADER_HANDLES 100

class KRShaderManager {
public:
    KRShaderManager(const GLchar *szVertShaderSource, const GLchar *szFragShaderSource);
    ~KRShaderManager();
    
    KRShader *getShader(KRCamera *pCamera, bool bDiffuseMap, bool bNormalMap, bool bSpecMap, int iShadowQuality);
    
private:
    std::map<std::string, KRShader *> m_shaders;
    
    GLchar *m_szFragShaderSource;
    GLchar *m_szVertShaderSource;
    
    KRShader *m_pShader;
};

#endif
