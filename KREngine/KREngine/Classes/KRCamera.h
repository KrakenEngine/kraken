//
//  KRSettings.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-10-02.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#ifndef KRCAMERA_H
#define KRCAMERA_H

#import "KRMat4.h"

class KRCamera {
public:
    KRCamera();
    ~KRCamera();
    
    KRMat4 getProjectionMatrix();
    
    bool bEnablePerPixel;
    bool bEnableDiffuseMap;
    bool bEnableNormalMap;
    bool bEnableSpecMap;
    bool bDebugPSSM;
    bool bDebugSuperShiny;
    bool bShowShadowBuffer;
    bool bEnableAmbient;
    bool bEnableDiffuse;
    bool bEnableSpecular;
    double dSunR;
    double dSunG;
    double dSunB;
    double dAmbientR;
    double dAmbientG;
    double dAmbientB;
    double perspective_fov;
    double perspective_nearz;
    double perspective_farz;
    double perspective_aspect;
    
    int dof_quality;
    double dof_depth;
    double dof_falloff;
    bool bEnableFlash;
    double flash_intensity;
    double flash_depth;
    double flash_falloff;
    
    bool bEnableVignette;
    double vignette_radius;
    double vignette_falloff;
};

#endif
