//
//  KRResource+fbx.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <assert.h>
#include <vector.h>

#include <fbxsdk.h>


#include "KRResource.h"
#include "KRModel.h"
#include "KRMaterial.h"
#include "KRLight.h"
#include "KRPointLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRNode.h"
#include "KRScene.h"
#include "KRQuaternion.h"
#include "KRBone.h"

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif

void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene);
void DestroySdkObjects(KFbxSdkManager* pSdkManager);
bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename);
KRAnimation *LoadAnimation(KRContext &context, FbxAnimStack* pAnimStack);
KRAnimationCurve *LoadAnimationCurve(KRContext &context, FbxAnimCurve* pAnimCurve);
KRAnimationLayer *LoadAnimationLayer(KRContext &context, FbxAnimLayer *pAnimLayer);
void LoadNode(KFbxScene* pFbxScene, KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
//void BakeNode(KFbxNode* pNode);
KRNode *LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
KRNode *LoadLight(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);
KRNode *LoadSkeleton(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);
std::string GetFbxObjectName(FbxObject *obj);

const float KRAKEN_FBX_ANIMATION_FRAMERATE = 30.0f; // FINDME - This should be configurable


std::string GetFbxObjectName(FbxObject *obj)
{
    // Object names from FBX files are now concatenated with the FBX numerical ID to ensure that they are unique
    // TODO - This should be updated to only add a prefix or suffix if needed to make the name unique
    std::stringstream st;
    st << "fbx_";
    st << obj->GetUniqueID();
    if(strlen(obj->GetName()) != 0) {
        st << "_";
        st << obj->GetName();
    }
    return st.str();
}

std::vector<KRResource *> KRResource::LoadFbx(KRContext &context, const std::string& path)
{
    
    std::vector<KRResource *> resources;
    KRScene *pScene = new KRScene(context, KRResource::GetFileBase(path));
    resources.push_back(pScene);
    
    KFbxSdkManager* lSdkManager = NULL;
    KFbxScene* pFbxScene = NULL;
    bool lResult;
    FbxGeometryConverter *pGeometryConverter = NULL;
    
    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, pFbxScene);
    
    // Initialize Geometry Converter
    pGeometryConverter = new FbxGeometryConverter(lSdkManager);
    
    // Load the scene.
    lResult = LoadScene(lSdkManager, pFbxScene, path.c_str());
    
    // ----====---- Bake pivots into transforms, as Kraken doesn't support them directly ----====----
    
    printf("Baking pivots...\n");
    KFbxNode* pNode = pFbxScene->GetRootNode();
    if(pNode) {
        pNode->ResetPivotSetAndConvertAnimation();
    }
    
    // ----====---- Import Animation Layers ----====----
    
    int animation_count = pFbxScene->GetSrcObjectCount<FbxAnimStack>();
    for(int i = 0; i < animation_count; i++) {
        //        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(pFbxScene->GetSrcObject(FBX_TYPE(FbxAnimStack), i));
        KRAnimation *new_animation = LoadAnimation(context, pFbxScene->GetSrcObject<FbxAnimStack>(i));
        context.getAnimationManager()->addAnimation(new_animation);
        resources.push_back(new_animation);
    }
    
    // ----====---- Import Animation Curves ----====----
    int curve_count = pFbxScene->GetSrcObjectCount<FbxAnimCurve>();
    for(int i=0; i < curve_count; i++) {
        KRAnimationCurve *new_curve = LoadAnimationCurve(context, pFbxScene->GetSrcObject<FbxAnimCurve>(i));
        context.getAnimationCurveManager()->addAnimationCurve(new_curve);
        resources.push_back(new_curve);
    }
    
    // ----====---- Import Scene Graph Nodes ----====----
    if(pNode)
    {
        for(int i = 0; i < pNode->GetChildCount(); i++)
        {
            LoadNode(pFbxScene, pScene->getRootNode(), resources, pGeometryConverter, pNode->GetChild(i));
        }
    }
    

    
    DestroySdkObjects(lSdkManager);
    
    
    // FINDME - HACK - This logic removes the animations and animation curves from their manager objects so they don't get dealloced twice.  In the future, we should keep all objects in their manager objects while importing and just return a KRContext containing all the managers.
    for(std::vector<KRResource *>::iterator resource_itr=resources.begin(); resource_itr != resources.end(); resource_itr++) {
        KRAnimation *animation = dynamic_cast<KRAnimation *>(*resource_itr);
        KRAnimationCurve *animation_curve = dynamic_cast<KRAnimationCurve *>(*resource_itr);
        if(animation) {
            context.getAnimationManager()->getAnimations().erase(animation->getName());
        }
        if(animation_curve) {
            context.getAnimationCurveManager()->getAnimationCurves().erase(animation_curve->getName());
        }
    }
    
    return resources;
}



void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene)
{
    // The first thing to do is to create the FBX SDK manager which is the 
    // object allocator for almost all the classes in the SDK.
    pSdkManager = KFbxSdkManager::Create();
    
    if (!pSdkManager)
    {
        printf("Unable to create the FBX SDK manager\n");
        exit(0);
    }
    
	// create an IOSettings object
	KFbxIOSettings * ios = KFbxIOSettings::Create(pSdkManager, IOSROOT );
	pSdkManager->SetIOSettings(ios);
    
	// Load plugins from the executable directory
	KString lPath = FbxGetApplicationDirectory();
#if defined(KARCH_ENV_WIN)
	KString lExtension = "dll";
#elif defined(KARCH_ENV_MACOSX)
	KString lExtension = "dylib";
#elif defined(KARCH_ENV_LINUX)
	KString lExtension = "so";
#endif
	pSdkManager->LoadPluginsDirectory(lPath.Buffer(), lExtension.Buffer());
    
    // Create the entity that will hold the scene.
    pScene = KFbxScene::Create(pSdkManager,"");
}

void DestroySdkObjects(KFbxSdkManager* pSdkManager)
{
    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
    if (pSdkManager) pSdkManager->Destroy();
    pSdkManager = NULL;
}


bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename)
{
    int lFileMajor, lFileMinor, lFileRevision;
    int lSDKMajor,  lSDKMinor,  lSDKRevision;
    //int lFileFormat = -1;
    int lAnimStackCount;
    bool lStatus;
    char lPassword[1024];
    
    // Get the file version number generate by the FBX SDK.
    KFbxSdkManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);
    
    // Create an importer.
    KFbxImporter* lImporter = KFbxImporter::Create(pSdkManager,"");
    
    // Initialize the importer by providing a filename.
    const bool lImportStatus = lImporter->Initialize(pFilename, -1, pSdkManager->GetIOSettings());
    lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
    
    if( !lImportStatus )
    {
        printf("Call to KFbxImporter::Initialize() failed.\n");
        printf("Error returned: %s\n\n", lImporter->GetLastErrorString());
        
        if (lImporter->GetLastErrorID() == FbxIOBase::eFileVersionNotSupportedYet ||
            lImporter->GetLastErrorID() == FbxIOBase::eFileVersionNotSupportedAnymore)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }
        
        return false;
    }
    
    printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
    
    if(!lImporter->IsFBX()) {
        printf("ERROR Unrecognized FBX File\n");
        return false;
    }


    printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
    
    // From this point, it is possible to access animation stack information without
    // the expense of loading the entire file.
    
    printf("Animation Stack Information\n");
    
    lAnimStackCount = lImporter->GetAnimStackCount();
    
    printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
    printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
    printf("\n");
    
    
    // Set the import states. By default, the import states are always set to 
    // true. The code below shows how to change these states.
    IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
    IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
    IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
    IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
    IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
    IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
    IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    
    // Import the scene.
    lStatus = lImporter->Import(pScene);
    
    if(lStatus == false && lImporter->GetLastErrorID() == FbxIOBase::ePasswordError)
    {
        printf("Please enter password: ");
        
        lPassword[0] = '\0';
        
        scanf("%s", lPassword);
        KString lString(lPassword);
        
        IOS_REF.SetStringProp(IMP_FBX_PASSWORD,      lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
        
        lStatus = lImporter->Import(pScene);
        
        if(lStatus == false && lImporter->GetLastErrorID() == FbxIOBase::ePasswordError)
        {
            printf("\nPassword is wrong, import aborted.\n");
        }
    }
    
//    // ----====---- Start: Bake pivots into transforms, as Kraken doesn't support them directly ----====----
//    
//    printf("Baking pivots...\n");
//    KFbxNode* pNode = ((KFbxScene*)pScene)->GetRootNode();
////    BakeNode(pNode);
//    
//    for(i = 0; i < lAnimStackCount; i++)
//    {
//        KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);
//        
//        printf("   Animation: \"%s\"\n", lTakeInfo->mName.Buffer());
//        
//        //pNode->ConvertPivotAnimationRecursive(lTakeInfo->mName.Buffer(), KFbxNode::eDestinationPivot, KRAKEN_FBX_ANIMATION_FRAMERATE);
//        pNode->ResetPivotSetAndConvertAnimation();
//        
//    }
////    pNode->ConvertPivotAnimationRecursive(NULL, KFbxNode::eDestinationPivot, KRAKEN_FBX_ANIMATION_FRAMERATE);
////    pNode->UpdatePropertiesFromPivotsAndLimits();
//    
//    // ----====---- End: Bake pivots into transforms, as Kraken doesn't support them directly ----====----

//    // ----====---- Bake pivots into transforms, as Kraken doesn't support them directly ----====----
//    
//    printf("Baking pivots...\n");
//    KFbxNode* pNode = ((KFbxScene*)pScene)->GetRootNode();
//    if(pNode) {
//        pNode->ResetPivotSetAndConvertAnimation();
//    }
    
    // Destroy the importer.
    lImporter->Destroy();
    
    return lStatus;
}

KRAnimation *LoadAnimation(KRContext &context, FbxAnimStack* pAnimStack)
{
    printf("Loading animation: \"%s\"\n", pAnimStack->GetName());
        
    KRAnimation *new_animation = new KRAnimation(context, pAnimStack->GetName());
    int cLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
    new_animation->setDuration(pAnimStack->LocalStop.Get().GetSecondDouble());
    for(int iLayer=0; iLayer < cLayers; iLayer++) {
        new_animation->addLayer(LoadAnimationLayer(context, pAnimStack->GetMember<FbxAnimLayer>(iLayer)));
    }
    return new_animation;
}

KRAnimationCurve *LoadAnimationCurve(KRContext &context, FbxAnimCurve* pAnimCurve)
{
    std::string name = GetFbxObjectName(pAnimCurve);
    printf("Loading animation curve: \"%s\"\n", name.c_str());
    FbxTimeSpan time_span;
    if(!pAnimCurve->GetTimeInterval(time_span)) {
        printf(" ERROR: Failed to get time interval.\n");
        return NULL;
    }
    
    float frame_rate = 30.0f; // FINDME, TODO - This needs to be dynamic
    int frame_start = time_span.GetStart().GetSecondDouble() * frame_rate;
    int frame_count = (time_span.GetStop().GetSecondDouble() * frame_rate) - frame_start;
    
    KRAnimationCurve *new_curve = new KRAnimationCurve(context, name);
    new_curve->setFrameRate(frame_rate);
    new_curve->setFrameStart(frame_start);
    new_curve->setFrameCount(frame_count);
    
    // Resample animation curve
    int last_frame = 0; // Used by FBX sdk for faster keyframe searches
    for(int frame_number=0; frame_number < frame_count; frame_number++) {
        float frame_seconds = (frame_start + frame_number) / frame_rate;
        FbxTime frame_time;
        frame_time.SetSecondDouble(frame_seconds);
        float frame_value = pAnimCurve->Evaluate(frame_time, &last_frame);
        //printf("  Frame %i / %i: %.6f\n", frame_number, frame_count, frame_value);
        new_curve->setValue(frame_number, frame_value);
    }

    return new_curve;
}

KRAnimationLayer *LoadAnimationLayer(KRContext &context, FbxAnimLayer *pAnimLayer)
{
    KRAnimationLayer *new_layer = new KRAnimationLayer(context);
    new_layer->setName(pAnimLayer->GetName());
    new_layer->setWeight(pAnimLayer->Weight.Get() / 100.0f);
    switch(pAnimLayer->BlendMode.Get()) {
        case FbxAnimLayer::eBlendAdditive:
            new_layer->setBlendMode(KRAnimationLayer::KRENGINE_ANIMATION_BLEND_MODE_ADDITIVE);
            break;
        case FbxAnimLayer::eBlendOverride:
            new_layer->setBlendMode(KRAnimationLayer::KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE);
            break;
        case FbxAnimLayer::eBlendOverridePassthrough:
            new_layer->setBlendMode(KRAnimationLayer::KRENGINE_ANIMATION_BLEND_MODE_OVERRIDE_PASSTHROUGH);
            break;
    }
    switch(pAnimLayer->RotationAccumulationMode.Get()) {
        case FbxAnimLayer::eRotationByLayer:
            new_layer->setRotationAccumulationMode(KRAnimationLayer::KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_LAYER);
            break;
        case FbxAnimLayer::eRotationByChannel:
            new_layer->setRotationAccumulationMode(KRAnimationLayer::KRENGINE_ANIMATION_ROTATION_ACCUMULATION_BY_CHANNEL);
            break;
    }
    switch(pAnimLayer->ScaleAccumulationMode.Get()) {
        case FbxAnimLayer::eScaleAdditive:
            new_layer->setScaleAccumulationMode(KRAnimationLayer::KRENGINE_ANIMATION_SCALE_ACCUMULATION_ADDITIVE);
            break;
        case FbxAnimLayer::eScaleMultiply:
            new_layer->setScaleAccumulationMode(KRAnimationLayer::KRENGINE_ANIMATION_SCALE_ACCUMULATION_MULTIPLY);
            break;
    }
    
    return new_layer;
}
//
//void BakeNode(KFbxNode *pNode) {
//    
//    pNode->SetPivotState(KFbxNode::eSourcePivot, KFbxNode::ePivotActive);
//    pNode->SetPivotState(KFbxNode::eDestinationPivot, KFbxNode::ePivotActive);
//    
//    // Pass the current value to the source pivot.
////    *     - Rotation offset (Roff)
////    *     - Rotation pivot (Rp)
////    *     - Pre-rotation (Rpre)
////    *     - Post-rotation (Rpost)
////    *     - Scaling offset (Soff)
////    *     - Scaling pivot (Sp)
////    *     - Geometric translation (Gt)
////    *     - Geometric rotation (Gr)
////    *     - Geometric scaling (Gs)
//    /*
//    pNode->SetPostRotation(KFbxNode::eSourcePivot, pNode->PostRotation.Get());
//    pNode->SetPreRotation(KFbxNode::eSourcePivot, pNode->PreRotation.Get());
//    pNode->SetRotationOffset(KFbxNode::eSourcePivot, pNode->RotationOffset.Get());
//    pNode->SetScalingOffset(KFbxNode::eSourcePivot, pNode->ScalingOffset.Get());
//    pNode->SetRotationPivot(KFbxNode::eSourcePivot, pNode->RotationPivot.Get());
//    pNode->SetScalingPivot(KFbxNode::eSourcePivot, pNode->ScalingPivot.Get());
//    pNode->SetGeometricRotation(KFbxNode::eSourcePivot, pNode->GeometricRotation.Get());
//    pNode->SetGeometricTranslation(KFbxNode::eSourcePivot, pNode->GeometricTranslation.Get());
//    pNode->SetGeometricScaling(KFbxNode::eSourcePivot, pNode->GeometricScaling.Get());
//    pNode->SetRotationOrder(KFbxNode::eSourcePivot, pNode->RotationOrder.Get());
//     */
//    
//    // We want to set all these to 0 and bake them into the transforms.
//    KFbxVector4 lZero(0.0, 0.0, 0.0);
//    KFbxVector4 lOne(1.0, 1.0, 1.0);
//    pNode->SetPostRotation(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetPreRotation(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetRotationOffset(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetScalingOffset(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetRotationPivot(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetScalingPivot(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetGeometricRotation(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetGeometricTranslation(KFbxNode::eDestinationPivot, lZero);
//    pNode->SetGeometricScaling(KFbxNode::eDestinationPivot, lOne);
//    pNode->SetRotationOrder(KFbxNode::eDestinationPivot, eEULER_XYZ);
//    
//    
//    /*
//     FbxVector4 lZero(0,0,0);
//     FbxVector4 lOne(1,1,1);
//     pNode->SetPivotState(FbxNode::eSourcePivot, FbxNode::ePivotActive);
//     pNode->SetPivotState(FbxNode::eDestinationPivot, FbxNode::ePivotActive);
//    
//     EFbxRotationOrder lRotationOrder;
//     pNode->GetRotationOrder(FbxNode::eSourcePivot , lRotationOrder);
//     pNode->SetRotationOrder(FbxNode::eDestinationPivot , lRotationOrder);
//    
//     //For cameras and lights (without targets) let's compensate the postrotation.
//     if( pNode->GetCamera() || pNode->GetLight() )
//         {
//                if( !pNode->GetTarget() )
//                    {
//                            FbxVector4 lRV(90, 0, 0);
//                            if( pNode->GetCamera() )
//                                   lRV.Set(0, 90, 0);
//                    
//                            FbxVector4 prV = pNode->GetPostRotation(FbxNode::eSourcePivot);
//                            FbxAMatrix lSourceR;
//                            FbxAMatrix lR(lZero, lRV, lOne);
//                            FbxVector4 res = prV;
//                    
//                            // Rotation order don't affect post rotation, so just use the default XYZ order
//                            FbxRotationOrder rOrder;
//                            rOrder.V2M(lSourceR, res);
//                    
//                            lR = lSourceR * lR;
//                            rOrder.M2V(res, lR);
//                            prV = res;
//                            pNode->SetPostRotation(FbxNode::eSourcePivot, prV);
//                            pNode->SetRotationActive(true);
//                        }
//            
//                // Point light do not need to be adjusted (since they radiate in all the directions).
//                if( pNode->GetLight() && pNode->GetLight()->LightType.Get() == FbxLight::ePoint )
//                    {
//                            pNode->SetPostRotation(FbxNode::eSourcePivot, FbxVector4(0,0,0,0));
//                        }
//             }
//     // apply Pre rotations only on bones / end of chains
//     if( pNode->GetNodeAttribute() && pNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton
//             || (pNode->GetMarker() && pNode->GetMarker()->GetType() == FbxMarker::eEffectorFK)
//             || (pNode->GetMarker() && pNode->GetMarker()->GetType() == FbxMarker::eEffectorIK) )
//         {
//                if( pNode->GetRotationActive() )
//                    {
//                           pNode->SetPreRotation(FbxNode::eDestinationPivot, pNode->GetPreRotation(FbxNode::eSourcePivot));
//                        }
//            
//                // No pivots on bones
//                pNode->SetRotationPivot(FbxNode::eDestinationPivot, lZero);
//                pNode->SetScalingPivot(FbxNode::eDestinationPivot, lZero);
//                pNode->SetRotationOffset(FbxNode::eDestinationPivot,lZero);
//                pNode->SetScalingOffset(FbxNode::eDestinationPivot, lZero);
//             }
//     else
//         {
//                // any other type: no pre-rotation support but...
//                pNode->SetPreRotation(FbxNode::eDestinationPivot, lZero);
//            
//                // support for rotation and scaling pivots.
//                pNode->SetRotationPivot(FbxNode::eDestinationPivot, pNode->GetRotationPivot(FbxNode::eSourcePivot));
//                pNode->SetScalingPivot(FbxNode::eDestinationPivot, pNode->GetScalingPivot(FbxNode::eSourcePivot));
//                // Rotation and scaling offset are supported
//                pNode->SetRotationOffset(FbxNode::eDestinationPivot, pNode->GetRotationOffset(FbxNode::eSourcePivot));
//                pNode->SetScalingOffset(FbxNode::eDestinationPivot, pNode->GetScalingOffset(FbxNode::eSourcePivot));
//                //
//             // If we don't "support" scaling pivots, we can simply do:
//                // pNode->SetRotationPivot(FbxNode::eDestinationPivot, lZero);
//                // pNode->SetScalingPivot(FbxNode::eDestinationPivot, lZero);
//        }
//    */
//    
//    // Bake child nodes
//    for(int i = 0; i < pNode->GetChildCount(); i++)
//    {
//        BakeNode(pNode->GetChild(i));
//    }
//}

void LoadNode(KFbxScene* pFbxScene, KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    KFbxVector4 lTmpVector;
    pNode->UpdatePropertiesFromPivotsAndLimits();
    
    // Transform = T * Roff * Rp * Rpre * R * Rpost * inverse(Rp) * Soff * Sp * S * inverse(Sp)
    
    // Import animated properties
    int animation_count = pFbxScene->GetSrcObjectCount<FbxAnimStack>();
    for(int i = 0; i < animation_count; i++) {
        //        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(pFbxScene->GetSrcObject(FBX_TYPE(FbxAnimStack), i));
        FbxAnimStack* pAnimStack = pFbxScene->GetSrcObject<FbxAnimStack>(i);
        KRAnimation *pAnimation = parent_node->getContext().getAnimationManager()->getAnimation(pAnimStack->GetName());
        int cLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
        for(int iLayer=0; iLayer < cLayers; iLayer++) {
            FbxAnimLayer *pFbxAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(iLayer);
            KRAnimationLayer *pAnimationLayer = pAnimation->getLayer(pFbxAnimLayer->GetName());
            
            FbxAnimCurve *pAnimCurve = pNode->LclRotation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_X);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclRotation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Y);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclRotation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_ROTATE_Z);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclTranslation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclTranslation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclTranslation.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclScaling.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_X);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclScaling.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Y);
                pAnimationLayer->addAttribute(new_attribute);
            }
            
            pAnimCurve = pNode->LclScaling.GetCurve(pFbxAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
            if(pAnimCurve) {
                KRAnimationAttribute *new_attribute = new KRAnimationAttribute(parent_node->getContext());
                new_attribute->setCurveName(GetFbxObjectName(pAnimCurve));
                new_attribute->setTargetName(GetFbxObjectName(pNode));
                new_attribute->setTargetAttribute(KRNode::KRENGINE_NODE_ATTRIBUTE_SCALE_Z);
                pAnimationLayer->addAttribute(new_attribute);
            }
        }
    }
    
    fbxDouble3 local_rotation = pNode->LclRotation.Get(); // pNode->GetGeometricRotation(KFbxNode::eSourcePivot);
    fbxDouble3 local_translation = pNode->LclTranslation.Get(); // pNode->GetGeometricTranslation(KFbxNode::eSourcePivot);
    fbxDouble3 local_scale = pNode->LclScaling.Get(); // pNode->GetGeometricScaling(KFbxNode::eSourcePivot);
    
    fbxDouble3 post_rotation = pNode->PostRotation.Get();
    fbxDouble3 pre_rotation = pNode->PreRotation.Get();
    fbxDouble3 rotation_offset = pNode->RotationOffset.Get();
    fbxDouble3 scaling_offset = pNode->ScalingOffset.Get();
    fbxDouble3 rotation_pivot = pNode->RotationPivot.Get();
    fbxDouble3 scaling_pivot = pNode->ScalingPivot.Get();
    fbxDouble3 geometric_rotation = pNode->GeometricRotation.Get();
    fbxDouble3 geometric_translation = pNode->GeometricTranslation.Get();
    fbxDouble3 geometric_scaling = pNode->GeometricScaling.Get();
    ERotationOrder rotation_order = pNode->RotationOrder.Get();
    
    
    KFbxVector4 lZero(0.0, 0.0, 0.0);
    KFbxVector4 lOne(1.0, 1.0, 1.0);
    
    assert(post_rotation == lZero);
    assert(pre_rotation == lZero);
    assert(rotation_offset == lZero);
    assert(scaling_offset == lZero);
    assert(rotation_pivot == lZero);
    assert(scaling_pivot == lZero);
    assert(geometric_rotation == lZero);
    assert(geometric_translation == lZero);
    assert(geometric_scaling == lOne);
    assert(rotation_order == eEulerXYZ);

    KRVector3 node_translation = KRVector3(local_translation[0], local_translation[1], local_translation[2]); // T * Roff * Rp
    KRVector3 node_rotation = KRVector3(local_rotation[0], local_rotation[1], local_rotation[2]) / 180.0 * M_PI;
    KRVector3 node_scale = KRVector3(local_scale[0], local_scale[1], local_scale[2]);
    
//    printf("        Local Translation:      %f %f %f\n", local_translation[0], local_translation[1], local_translation[2]);
//    printf("        Local Rotation:         %f %f %f\n", local_rotation[0], local_rotation[1], local_rotation[2]);
//    printf("        Local Scaling:          %f %f %f\n", local_scale[0], local_scale[1], local_scale[2]);
    
    KRNode *new_node = NULL;
    KFbxNodeAttribute::EType attribute_type = (pNode->GetNodeAttribute()->GetAttributeType());
    switch(attribute_type) {
        case KFbxNodeAttribute::eMesh:
            new_node = LoadMesh(parent_node, resources, pGeometryConverter, pNode);
            break;
        case KFbxNodeAttribute::eLight:
            new_node = LoadLight(parent_node, resources, pNode);
            break;
        case KFbxNodeAttribute::eSkeleton:
            new_node = LoadSkeleton(parent_node, resources, pNode);
            break;
        default:
            {
                if(pNode->GetChildCount() > 0) {
                    // Create an empty node, for inheritence of transforms
                    new_node = new KRNode(parent_node->getScene(), GetFbxObjectName(pNode));
                }
            }
            break;
    }
    
    
    if(new_node != NULL) {
        new_node->setLocalRotation(node_rotation);
        new_node->setLocalTranslation(node_translation);
        new_node->setLocalScale(node_scale);
        parent_node->addChild(new_node);
        
        // Load child nodes
        for(int i = 0; i < pNode->GetChildCount(); i++)
        {
            LoadNode(pFbxScene, new_node, resources, pGeometryConverter, pNode->GetChild(i));
        }
    }
}

KRNode *LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    std::string name = GetFbxObjectName(pNode);
    printf("Mesh: %s\n", name.c_str());
    KFbxMesh* pSourceMesh = (KFbxMesh*) pNode->GetNodeAttribute();
    KFbxMesh* pMesh = pGeometryConverter->TriangulateMesh(pSourceMesh);
    
    int skin_count = pMesh->GetDeformerCount(FbxDeformer::eSkin);
    for(int skin_index=0; skin_index<skin_count; skin_index++) {
        FbxSkin *skin = (FbxSkin *)pMesh->GetDeformer(skin_index, FbxDeformer::eSkin);
        skin->GetControlPointBlendWeights()
    }
    
    KFbxVector4* control_points = pMesh->GetControlPoints(); 
    
    int polygon_count = pMesh->GetPolygonCount();
    int uv_count = pMesh->GetElementUVCount();
    int normal_count = pMesh->GetElementNormalCount();
    int tangent_count = pMesh->GetElementTangentCount();
    int elementmaterial_count = pMesh->GetElementMaterialCount();
    int material_count = pNode->GetMaterialCount();
    
        
    printf("  Polygon Count: %i (before triangulation: %i)\n", polygon_count, pSourceMesh->GetPolygonCount());

    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector2> uvb;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_lengths;
    std::vector<int> submesh_starts;
    std::vector<std::string> material_names;
    
    int dest_vertex_id = 0;

    for(int iMaterial=0; iMaterial < material_count; iMaterial++) {
        KFbxSurfaceMaterial *pMaterial = pNode->GetMaterial(iMaterial);
        int source_vertex_id = 0;
        int mat_vertex_count = 0;
        int mat_vertex_start = dest_vertex_id;
        for(int iPolygon = 0; iPolygon < polygon_count; iPolygon++) {
            int lPolygonSize = pMesh->GetPolygonSize(iPolygon);
            if(lPolygonSize != 3) {
                source_vertex_id += lPolygonSize;
                printf("    Warning - Poly with %i vertices found. Expecting only triangles.", lPolygonSize);
            } else {
                // ----====---- Read SubMesh / Material Mapping ----====----
                int iNewMaterial = -1;
                for (int l = 0; l < elementmaterial_count; l++)
                {
                    FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
                    if(leMat) {
                        if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex || leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect) {
                            int new_id = leMat->GetIndexArray().GetAt(iPolygon);
                            if(new_id >= 0) {
                                iNewMaterial = new_id;
                            }
                        }
                    }
                }
                
                if(iMaterial == iNewMaterial) {
                    // ----====---- Read Vertex-level Attributes ----====----
                    for(int iVertex=0; iVertex<3; iVertex++) {
                        // ----====---- Read Vertex Position ----====----
                        int lControlPointIndex = pMesh->GetPolygonVertex(iPolygon, iVertex);
                        KFbxVector4 v = control_points[lControlPointIndex];
                        vertices.push_back(KRVector3(v[0], v[1], v[2]));
                        
                        KRVector2 new_uva = KRVector2(0.0, 0.0);
                        KRVector2 new_uvb = KRVector2(0.0, 0.0);
                        
                        
                        // ----====---- Read UVs ----====----
                        
                        KStringList uvNames;
                        pMesh->GetUVSetNames(uvNames);
                        if(uv_count >= 1) {
                            const char *setName = uvNames[0].Buffer();
                            KFbxVector2 uv;
                            if(pMesh->GetPolygonVertexUV(iPolygon, iVertex, setName, uv)) {
                                new_uva = KRVector2(uv[0], uv[1]);
                            }
                            uva.push_back(new_uva);
                        }
                        
                        if(uv_count >= 2) {
                            const char *setName = uvNames[1].Buffer();
                            KFbxVector2 uv;
                            if(pMesh->GetPolygonVertexUV(iPolygon, iVertex, setName, uv)) {
                                new_uvb = KRVector2(uv[0], uv[1]);
                            }
                            uvb.push_back(new_uvb);
                        }                
                        
                        // ----====---- Read Normals ----====----
                        
                        KFbxVector4 new_normal;
                        if(pMesh->GetPolygonVertexNormal(iPolygon, iVertex, new_normal)) {
                            normals.push_back(KRVector3(new_normal[0], new_normal[1], new_normal[2]));
                        }
                        
                        
                        // ----====---- Read Tangents ----====----
                        for(int l = 0; l < tangent_count; ++l)
                        {
                            KFbxVector4 new_tangent;
                            FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
                            
                            if(leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex) {
                                switch (leTangent->GetReferenceMode()) {
                                    case FbxGeometryElement::eDirect:
                                        new_tangent = leTangent->GetDirectArray().GetAt(lControlPointIndex);
                                        break;
                                    case FbxGeometryElement::eIndexToDirect:
                                    {
                                        int id = leTangent->GetIndexArray().GetAt(lControlPointIndex);
                                        new_tangent = leTangent->GetDirectArray().GetAt(id);
                                    }
                                        break;
                                    default:
                                        break; // other reference modes not shown here!
                                }
                            }
                            if(l == 0) {
                                tangents.push_back(KRVector3(new_tangent[0], new_tangent[1], new_tangent[2]));
                            }
                            
                        }

                        

                        
                        source_vertex_id++;
                        dest_vertex_id++;
                        mat_vertex_count++;
                    }
                }
            }
        }
        
        
        if(mat_vertex_count) {
            // ----====---- Output last material / submesh details ----====----
            submesh_starts.push_back(mat_vertex_start);
            submesh_lengths.push_back(mat_vertex_count);
            material_names.push_back(pMaterial->GetName());
            printf("  %s: %i - %i\n", pMaterial->GetName(), mat_vertex_start, mat_vertex_count + mat_vertex_start - 1);
            
            // ----====---- Output Material File ----====----
            KRMaterial *new_material = new KRMaterial(parent_node->getContext(), pMaterial->GetName());
            
            FbxPropertyT<FbxDouble3> lKFbxDouble3;
            FbxPropertyT<FbxDouble> lKFbxDouble1;
            
            if (pMaterial->GetClassId().Is(KFbxSurfacePhong::ClassId)) {
                // We found a Phong material.
                
                // Ambient Color
                lKFbxDouble3 =((FbxSurfacePhong *) pMaterial)->Ambient;
                new_material->setAmbient(KRVector3(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]));
                
                // Diffuse Color
                lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Diffuse;
                new_material->setDiffuse(KRVector3(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]));
                
                // Specular Color (unique to Phong materials)
                lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Specular;
                new_material->setSpecular(KRVector3(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]));
                
                // Emissive Color
                //lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Emissive;
                
                // Transparency
                lKFbxDouble1 =((KFbxSurfacePhong *) pMaterial)->TransparencyFactor;
                new_material->setTransparency(lKFbxDouble1.Get());
                
                // Shininess
                lKFbxDouble1 =((KFbxSurfacePhong *) pMaterial)->Shininess;
                new_material->setShininess(lKFbxDouble1.Get());
                		
                // Specular Factor
                lKFbxDouble1 =((KFbxSurfacePhong *) pMaterial)->SpecularFactor;
                double specular_factor = lKFbxDouble1.Get();
                
                // Reflection factor
                lKFbxDouble1 =((KFbxSurfacePhong *) pMaterial)->ReflectionFactor;
                
                // Reflection color
                lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Reflection;
                
                // We modulate Relection color by reflection factor, as we only have one "reflection color" variable in Kraken
                new_material->setReflection(KRVector3(lKFbxDouble3.Get()[0] * lKFbxDouble1.Get(), lKFbxDouble3.Get()[1] * lKFbxDouble1.Get(), lKFbxDouble3.Get()[2] * lKFbxDouble1.Get()));
                
            } else if(pMaterial->GetClassId().Is(KFbxSurfaceLambert::ClassId) ) {
                // We found a Lambert material.
                
                // Ambient Color
                lKFbxDouble3=((KFbxSurfaceLambert *)pMaterial)->Ambient;
                new_material->setAmbient(KRVector3(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]));
                
                // Diffuse Color
                lKFbxDouble3 =((KFbxSurfaceLambert *)pMaterial)->Diffuse;
                new_material->setDiffuse(KRVector3(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]));
                
                // Emissive
                //lKFbxDouble3 =((KFbxSurfaceLambert *)pMaterial)->Emissive;
                
                // Opacity
                lKFbxDouble1 =((KFbxSurfaceLambert *)pMaterial)->TransparencyFactor;
                new_material->setTransparency(lKFbxDouble1.Get());
            } else {
                printf("Error! Unable to convert material: %s", pMaterial->GetName());
            }
            
            
            
            KFbxProperty pProperty;
            
            // Diffuse Map Texture
            pProperty = pMaterial->FindProperty(KFbxSurfaceMaterial::sDiffuse);
            if(pProperty.GetSrcObjectCount(KFbxLayeredTexture::ClassId) > 0) {
                printf("Warning! Layered textures not supported.\n");
            }
            
            int texture_count = pProperty.GetSrcObjectCount(KFbxTexture::ClassId);
            if(texture_count > 1) {
                printf("Error! Multiple diffuse textures not supported.\n");
            } else if(texture_count == 1) {
                KFbxTexture* pTexture = FbxCast <KFbxTexture> (pProperty.GetSrcObject(KFbxTexture::ClassId,0));
                assert(!pTexture->GetSwapUV());
                assert(pTexture->GetCroppingTop() == 0);
                assert(pTexture->GetCroppingLeft() == 0);
                assert(pTexture->GetCroppingRight() == 0);
                assert(pTexture->GetCroppingBottom() == 0);
                assert(pTexture->GetWrapModeU() == KFbxTexture::eRepeat);
                assert(pTexture->GetWrapModeV() == KFbxTexture::eRepeat);
                assert(pTexture->GetRotationU() == 0.0f);
                assert(pTexture->GetRotationV() == 0.0f);
                assert(pTexture->GetRotationW() == 0.0f);
                
                KFbxFileTexture *pFileTexture = FbxCast<KFbxFileTexture>(pTexture);
                if(pFileTexture) {
                    new_material->setDiffuseMap(KRResource::GetFileBase(pFileTexture->GetFileName()), KRVector2(pTexture->GetScaleU(), pTexture->GetScaleV()),  KRVector2(pTexture->GetTranslationU(), pTexture->GetTranslationV()));
                }
            }

            
            // Specular Map Texture
            pProperty = pMaterial->FindProperty(KFbxSurfaceMaterial::sSpecular);
            if(pProperty.GetSrcObjectCount(KFbxLayeredTexture::ClassId) > 0) {
                printf("Warning! Layered textures not supported.\n");
            }
            texture_count = pProperty.GetSrcObjectCount(KFbxTexture::ClassId);
            if(texture_count > 1) {
                printf("Error! Multiple specular textures not supported.\n");
            } else if(texture_count == 1) {
                KFbxTexture* pTexture = FbxCast <KFbxTexture> (pProperty.GetSrcObject(KFbxTexture::ClassId,0));
                KFbxFileTexture *pFileTexture = FbxCast<KFbxFileTexture>(pTexture);
                if(pFileTexture) {
                    new_material->setSpecularMap(KRResource::GetFileBase(pFileTexture->GetFileName()), KRVector2(pTexture->GetScaleU(), pTexture->GetScaleV()),  KRVector2(pTexture->GetTranslationU(), pTexture->GetTranslationV()));
                }
            }
            
            // Normal Map Texture
            pProperty = pMaterial->FindProperty(KFbxSurfaceMaterial::sNormalMap);
            if(pProperty.GetSrcObjectCount(KFbxLayeredTexture::ClassId) > 0) {
                printf("Warning! Layered textures not supported.\n");
            }
            
            
            texture_count = pProperty.GetSrcObjectCount<FbxTexture>();
            if(texture_count > 1) {
                printf("Error! Multiple normal map textures not supported.\n");
            } else if(texture_count == 1) {
                KFbxTexture* pTexture = pProperty.GetSrcObject<KFbxTexture>(0);
                KFbxFileTexture *pFileTexture = FbxCast<KFbxFileTexture>(pTexture);
                if(pFileTexture) {
                    new_material->setNormalMap(KRResource::GetFileBase(pFileTexture->GetFileName()), KRVector2(pTexture->GetScaleU(), pTexture->GetScaleV()),  KRVector2(pTexture->GetTranslationU(), pTexture->GetTranslationV()));
                }
            }
            
            bool bFound = false;
            for(vector<KRResource *>::iterator resource_itr = resources.begin(); resource_itr != resources.end(); resource_itr++) {
                KRResource *pResource = (*resource_itr);
                if(pResource->getName() == new_material->getName() && pResource->getExtension() == new_material->getExtension()) {
                    bFound = true;
                }
            }
            if(bFound) {
                delete new_material;
            } else {
                resources.push_back(new_material);
            }
        }
    }
    

    
    
    // ----====---- Generate Output Mesh Object ----====----

    KRModel *new_mesh = new KRModel(parent_node->getContext(), pNode->GetName());
    new_mesh->LoadData(vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names);
    resources.push_back(new_mesh);
    
    if(new_mesh->getLODCoverage() == 100) {
        // If this is the full detail model, add an instance of it to the scene file
        std::string light_map = pNode->GetName();
        light_map.append("_lightmap");
        
        KRInstance *new_instance = new KRInstance(parent_node->getScene(), name, pNode->GetName(), light_map, 0.0f, true, false);
        return new_instance;
    } else {
        return NULL;
    }
    
}

KRNode *LoadSkeleton(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode) {
    std::string name = GetFbxObjectName(pNode);
    KRBone *new_bone = new KRBone(parent_node->getScene(), name.c_str());
    return new_bone;
}

KRNode *LoadLight(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode) {
    const GLfloat PI = 3.14159265;
    const GLfloat d2r = PI * 2 / 360;
    
    FbxLight* pLight = (FbxLight*) pNode->GetNodeAttribute();
    const char *szName = pNode->GetName();
    
    FbxDouble3 light_color = pLight->Color.Get();
    FbxDouble light_intensity = pLight->Intensity.Get();
    FbxDouble light_hotspot = pLight->InnerAngle.Get(); // light inner cone angle (in degrees). Also know as the HotSpot
    FbxDouble light_coneangle = pLight->OuterAngle.Get(); // light outer cone angle (in degrees). Also known as the Falloff
    KFbxLight::EDecayType light_decaytype = pLight->DecayType.Get(); // decay type
    FbxDouble light_decaystart = pLight->DecayStart.Get(); // decay start distance
    
    
//    KFbxLight::eNONE         - does not attenuate with distance
//    KFbxLight::eLINEAR       - attenuation of 1/d
//    KFbxLight::eQUADRATIC    - attenuation of 1/d^2
//    KFbxLight::eCUBIC        - attenuation of
    
    KRLight *new_light = NULL;
    
    switch(pLight->LightType.Get()) {
        case KFbxLight::ePoint:
        {
            KRPointLight *l = new KRPointLight(parent_node->getScene(), szName);
            new_light = l;
            
        }
            break;
        case KFbxLight::eDirectional:
        {
            KRDirectionalLight *l = new KRDirectionalLight(parent_node->getScene(), szName);
            new_light = l;
        }
            break;
        case KFbxLight::eSpot:
        {
            KRSpotLight *l = new KRSpotLight(parent_node->getScene(), szName);
            l->setInnerAngle(light_hotspot * d2r);
            l->setOuterAngle(light_coneangle * d2r);
            new_light = l;
        }
            break;
        case KFbxLight::eVolume:
        case KFbxLight::eArea:
            // Not supported yet
            break;
    }
    
    if(new_light) {
        new_light->setColor(KRVector3(light_color[0], light_color[1], light_color[2]));
        new_light->setIntensity(light_intensity);
        new_light->setDecayStart(light_decaystart);
    }
    return new_light;
    
}
