//
//  KRInstance.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-09-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#import "KRModel.h"
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRBoundingVolume.h"
#import "KRInstance.h"
#import "KRCamera.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#ifndef KRINSTANCE_H
#define KRINSTANCE_H

class KRBoundingVolume;

class KRInstance {
    
public:
    KRInstance(KRModel *pModel, const KRMat4 modelMatrix);
    ~KRInstance();
    void render(KRCamera *pCamera, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &viewMatrix,  Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers);
    
    KRBoundingVolume getExtents();
    
    KRMat4 &getModelMatrix();
    KRModel *getModel();
    
private:
    KRModel *m_pModel;
    KRMat4 m_modelMatrix;
};


#endif
