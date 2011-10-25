//
//  KRScene.h
//  KREngine
//
//  Created by Kearwood Gilbert on 11-09-29.
//  Copyright 2011 Kearwood Software. All rights reserved.
//

#ifndef KRSCENE_H
#define KRSCENE_H

#import <vector>
#import "KRInstance.h"
#import "KRBoundingVolume.h"
#import "KRMat4.h"
#import "KRModel.h"
#import "KRCamera.h"

class KRBoundingVolume;
class KRInstance;

using std::vector;

class KRScene {
public:
    KRScene();
    ~KRScene();
    KRInstance *addInstance(KRModel *pModel, KRMat4 modelMatrix);
    void render(KRCamera *pCamera, KRBoundingVolume &frustrumVolume, KRMaterialManager *pMaterialManager, bool bRenderShadowMap, KRMat4 &viewMatrix, Vector3 &cameraPosition, Vector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers);
    KRBoundingVolume getExtents();
private:
    vector<KRInstance *> m_instances;
    KRBoundingVolume *m_pExtents;
    
    void calcExtents();
    void clearExtents();
};



#endif
