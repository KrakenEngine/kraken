//
//  KRSettings.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 11-10-02.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#include <iostream>

#import "KRCamera.h"

KRCamera::KRCamera() {
    double const PI = 3.141592653589793f;
    
    bShowShadowBuffer = false;
    bEnablePerPixel = true;
    bEnableDiffuseMap = true;
    bEnableNormalMap = true;
    bEnableSpecMap = true;
    bDebugPSSM = false;
    bEnableAmbient = true;
    bEnableDiffuse = true;
    bEnableSpecular = true;
    bDebugSuperShiny = false;
    
    dAmbientR = 0.25f;
    dAmbientG = 0.25f;
    dAmbientB = 0.35f;
    
    dSunR = 1.0f;
    dSunG = 1.0f;
    dSunB = 0.70f;
    
    perspective_fov = PI / 8.0;
    perspective_aspect = 1.3333;
    perspective_nearz = 0.25f;
    perspective_farz = 100.0f;
    
    dof_quality = 0;
    dof_depth = 0.05f;
    dof_falloff = 0.05f;
    
    bEnableFlash = false;
    flash_intensity = 1.0f;
    flash_depth = 0.7f;
    flash_falloff = 0.5f;
    
    
    bEnableVignette = false;
    vignette_radius = 0.4f;
    vignette_falloff = 1.0f;

}

KRCamera::~KRCamera() {
    
}

KRMat4 KRCamera::getProjectionMatrix() {
    KRMat4 projectionMatrix;
    projectionMatrix.perspective(perspective_fov, perspective_aspect, perspective_nearz, perspective_farz);
    projectionMatrix.rotate(-90 * 0.0174532925199, Z_AXIS);
    return projectionMatrix;
}