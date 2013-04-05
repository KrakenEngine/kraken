//
//  KRResource+fbx.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-03-22.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include "KREngine-common.h"
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <fbxsdk.h>


#include "KRResource.h"
#include "KRMesh.h"
#include "KRMaterial.h"
#include "KRLight.h"
#include "KRPointLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRNode.h"
#include "KRScene.h"
#include "KRQuaternion.h"
#include "KRBone.h"
#include "KRBundle.h"
#include "KRModel.h"
#include "KRLODGroup.h"
#include "KRCollider.h"

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
void LoadMaterial(KRContext &context, std::vector<KRResource *> &resources, FbxSurfaceMaterial *pMaterial);
void LoadMesh(KRContext &context, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxMesh* pSourceMesh);
KRNode *LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
KRNode *LoadLight(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);
KRNode *LoadSkeleton(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);
KRNode *LoadCamera(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);
void LoadLOD(KRContext &context, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, FbxLODGroup* pSourceLodGroup);
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
    printf("\nLoading animations...\n");
    int animation_count = pFbxScene->GetSrcObjectCount<FbxAnimStack>();
    for(int i = 0; i < animation_count; i++) {
        FbxAnimStack *animation = pFbxScene->GetSrcObject<FbxAnimStack>(i);
        printf("  Animation %i of %i: %s\n", i+1, animation_count, animation->GetName());
        KRAnimation *new_animation = LoadAnimation(context, animation);
        if(new_animation) {
            context.getAnimationManager()->addAnimation(new_animation);
            resources.push_back(new_animation);
        }
    }
    
    // ----====---- Import Animation Curves ----====----
    printf("\nLoading animation curves...\n");
    int curve_count = pFbxScene->GetSrcObjectCount<FbxAnimCurve>();
    for(int i=0; i < curve_count; i++) {
        FbxAnimCurve *curve = pFbxScene->GetSrcObject<FbxAnimCurve>(i);
        printf("  Animation Curve %i of %i: %s\n", i+1, curve_count, curve->GetName());
        KRAnimationCurve *new_curve = LoadAnimationCurve(context, curve);
        if(new_curve) {
            context.getAnimationCurveManager()->addAnimationCurve(new_curve);
            resources.push_back(new_curve);
        }
    }
    
    // ----====---- Import Meshes ----====----
    int mesh_count = pFbxScene->GetSrcObjectCount<FbxMesh>();
    printf("\nLoading meshes...\n");
    for(int i=0; i < mesh_count; i++) {
        FbxMesh *mesh = pFbxScene->GetSrcObject<FbxMesh>(i);
        
        printf("  Mesh %i of %i: %s\n", i+1, mesh_count, mesh->GetNode()->GetName());
        LoadMesh(context, resources, pGeometryConverter, mesh);
    }
    
    // ----====---- Import Materials ----====----
    int material_count = pFbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
    printf("\nLoading materials...\n");
    for(int i=0; i < material_count; i++) {
        FbxSurfaceMaterial *material = pFbxScene->GetSrcObject<FbxSurfaceMaterial>(i);
        printf("  Material %i of %i: %s\n", i+1, material_count, material->GetName());
        LoadMaterial(context, resources, material);
    }
    
    // ----====---- Import Textures ----====----
    int texture_count = pFbxScene->GetSrcObjectCount<FbxFileTexture>();
    
    printf("\nLoading textures...\n");
    for(int i=0; i < texture_count; i++) {
        FbxFileTexture *texture = pFbxScene->GetSrcObject<FbxFileTexture>(i);
        const char *file_name = texture->GetFileName();
        printf("  Texture %i of %i: %s\n", i+1, texture_count, (KRResource::GetFileBase(file_name) + "." + KRResource::GetFileExtension(file_name)).c_str());
        context.loadResource(file_name);
    }
    
    // ----====---- Import Scene Graph Nodes ----====----
    printf("\nLoading scene graph...\n");
    if(pNode)
    {
        for(int i = 0; i < pNode->GetChildCount(); i++)
        {
            LoadNode(pFbxScene, pScene->getRootNode(), resources, pGeometryConverter, pNode->GetChild(i));
        }
    }
    
    for(std::map<std::string, KRTexture *>::iterator texture_itr = context.getTextureManager()->getTextures().begin(); texture_itr != context.getTextureManager()->getTextures().end(); texture_itr++) {
        resources.push_back((*texture_itr).second);
    }
    
    for(std::map<std::string, KRMesh *>::iterator mesh_itr = context.getModelManager()->getModels().begin(); mesh_itr != context.getModelManager()->getModels().end(); mesh_itr++) {
        resources.push_back((*mesh_itr).second);
    }
    
    DestroySdkObjects(lSdkManager);
    
    // Compress textures to PVR format
    context.getTextureManager()->compress(); // TODO, HACK, FINDME - This should be configurable and exposed through the World Builder GUI
    
    std::string base_name = KRResource::GetFileBase(path);
    std::vector<KRResource *> output_resources;
    
//    KRBundle *main_bundle = new KRBundle(context, base_name);
    KRBundle *texture_bundle = new KRBundle(context, base_name + "_textures");
    KRBundle *animation_bundle = new KRBundle(context, base_name + "_animations");
    KRBundle *material_bundle = new KRBundle(context, base_name + "_materials");
    KRBundle *meshes_bundle = new KRBundle(context, base_name + "_meshes");
    
    for(std::vector<KRResource *>::iterator resource_itr=resources.begin(); resource_itr != resources.end(); resource_itr++) {
        KRResource *resource = *resource_itr;
        if(dynamic_cast<KRTexture *>(resource) != NULL) {
            texture_bundle->append(*resource);
        } else if(dynamic_cast<KRAnimation *>(resource) != NULL) {
            animation_bundle->append(*resource);
        } else if(dynamic_cast<KRAnimationCurve *>(resource) != NULL) {
            animation_bundle->append(*resource);
        } else if(dynamic_cast<KRMaterial *>(resource) != NULL) {
            material_bundle->append(*resource);
        } else if(dynamic_cast<KRMesh *>(resource) != NULL) {
            meshes_bundle->append(*resource);
        } else {
            output_resources.push_back(resource);
//            main_bundle->append(*resource);
        }
    }
    
    output_resources.push_back(texture_bundle);
    output_resources.push_back(animation_bundle);
    output_resources.push_back(material_bundle);
    output_resources.push_back(meshes_bundle);
    
//    main_bundle->append(texture_bundle);
//    main_bundle->append(animation_bundle);
//    main_bundle->append(material_bundle);
//    main_bundle->append(meshes_bundle);
//    output_resources.push_back(main_bundle);
    
    return output_resources;
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
    
    
    FbxSystemUnit system_unit = pFbxScene->GetGlobalSettings().GetSystemUnit();
    
    // Transform = T * Roff * Rp * Rpre * R * Rpost * inverse(Rp) * Soff * Sp * S * inverse(Sp)
    
    // Import animated properties
    int animation_count = pFbxScene->GetSrcObjectCount<FbxAnimStack>();
    for(int i = 0; i < animation_count; i++) {
        //        FbxAnimStack* pAnimStack = FbxCast<FbxAnimStack>(pFbxScene->GetSrcObject(FBX_TYPE(FbxAnimStack), i));
        FbxAnimStack* pAnimStack = pFbxScene->GetSrcObject<FbxAnimStack>(i);
        KRAnimation *pAnimation = parent_node->getContext().getAnimationManager()->getAnimation(pAnimStack->GetName());
        if(pAnimation) {
            int cLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
            for(int iLayer=0; iLayer < cLayers; iLayer++) {
                FbxAnimLayer *pFbxAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(iLayer);
//                float weight = pFbxAnimLayer->Weight.Get();
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
    
    KFbxNodeAttribute::EType attribute_type = (pNode->GetNodeAttribute()->GetAttributeType());
    if(attribute_type == KFbxNodeAttribute::eLODGroup) {
        std::string name = GetFbxObjectName(pNode);
        FbxLODGroup *fbx_lod_group = (FbxLODGroup*) pNode->GetNodeAttribute(); // FbxCast<FbxLODGroup>(pNode);
        if(!fbx_lod_group->WorldSpace.Get()) {
            printf("WARNING - LOD Groups only supported with world space distance thresholds.\n");
        }
        float group_min_distance = 0.0f;
        float group_max_distance = 0.0f;
        if(fbx_lod_group->MinMaxDistance.Get()) {
            group_min_distance = fbx_lod_group->MinDistance.Get();
            group_max_distance = fbx_lod_group->MinDistance.Get();
        }
        
        KRVector3 reference_point;
        // Create a lod_group node for each fbx child node
        int child_count = pNode->GetChildCount();
        for(int i = 0; i < child_count; i++)
        {
            float min_distance = 0;
            float max_distance = 0; // 0 for max_distance means infinity
            FbxLODGroup::EDisplayLevel display_level;
            fbx_lod_group->GetDisplayLevel(i, display_level);
            switch(display_level) {
                case FbxLODGroup::eUseLOD:
                    if(i > 0 ) {
                        FbxDistance d;
                        fbx_lod_group->GetThreshold(i - 1, d);
                        min_distance = d.valueAs(system_unit);
                    }
                    if(i < child_count - 1) {
                        FbxDistance d;
                        fbx_lod_group->GetThreshold(i, d);
                        max_distance = d.valueAs(system_unit);
                    }
                    break;
                case FbxLODGroup::eShow:
                    // We leave min_distance and max_distance as 0's, which effectively makes the LOD group always visible
                    break;
                case FbxLODGroup::eHide:
                    min_distance = -1;
                    max_distance = -1;
                    // LOD Groups with -1 for both min_distance and max_distance will never be displayed; import in case that the distance values are to be modified by scripting at runtime
                    break;
            }
            
            if(group_min_distance != 0.0f && min_distance != -1) {
                if(min_distance < group_min_distance) min_distance = group_min_distance;
            }
            if(group_max_distance != 0.0f && max_distance != -1) {
                if(max_distance == 0.0f) {
                    max_distance = group_max_distance;
                } else if(max_distance > group_max_distance) {
                    max_distance = group_max_distance;
                }
            }

            KRLODGroup *new_node = new KRLODGroup(parent_node->getScene(), name + "_lodlevel" + boost::lexical_cast<string>(i + 1));
            new_node->setMinDistance(min_distance);
            new_node->setMaxDistance(max_distance);
            new_node->setLocalRotation(node_rotation);
            new_node->setLocalTranslation(node_translation);
            new_node->setLocalScale(node_scale);
            parent_node->addChild(new_node);
            
            LoadNode(pFbxScene, new_node, resources, pGeometryConverter, pNode->GetChild(i));
            
            if(i == 0) {
                // Calculate reference point using the bounding box center from the highest quality LOD level
                reference_point = new_node->getBounds().center();
            }
            
            new_node->setReferencePoint(new_node->worldToLocal(reference_point));
        }
    } else {
        KRNode *new_node = NULL;
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
            case KFbxNodeAttribute::eCamera:
                new_node = LoadCamera(parent_node, resources, pNode);
                break;
            default:
                {
                    if(pNode->GetChildCount() > 0) {
                        // Create an empty node, for inheritence of transforms
                        std::string name = GetFbxObjectName(pNode);
                        
                        float min_distance = 0.0f;
                        float max_distance = 0.0f;
                        
                        typedef boost::tokenizer<boost::char_separator<char> > char_tokenizer;
                        
                        int step = 0;
                        
                        char_tokenizer name_components(name, boost::char_separator<char>("_"));
                        for(char_tokenizer::iterator itr=name_components.begin(); itr != name_components.end(); itr++) {
                            std::string component = *itr;
                            std::transform(component.begin(), component.end(),
                                           component.begin(), ::tolower);
                            if(component.compare("lod") == 0) {
                                step = 1;
                            } else if(step == 1) {
                                min_distance = boost::lexical_cast<float>(component);
                                step++;
                            } else if(step == 2) {
                                max_distance = boost::lexical_cast<float>(component);
                                step++;
                            }
                        }
                        
                        /*
                         if(min_distance == 0.0f && max_distance == 0.0f) {
                            // Regular node for grouping children together under one transform
                            new_node = new KRNode(parent_node->getScene(), name);
                        } else {
                         */
                            // LOD Enabled group node
                            KRLODGroup *lod_group = new KRLODGroup(parent_node->getScene(), name);
                            lod_group->setMinDistance(min_distance);
                            lod_group->setMaxDistance(max_distance);
                            new_node = lod_group;
                        /*
                        }
                         */
                        
                        
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
}

void LoadLOD(KRContext &context, std::vector<KRResource *> &resources, FbxLODGroup* pSourceLodGroup) {
    
}

void LoadMaterial(KRContext &context, std::vector<KRResource *> &resources, FbxSurfaceMaterial *pMaterial) {
    //printf("  %s: %i - %i\n", pMaterial->GetName(), mat_vertex_start, mat_vertex_count + mat_vertex_start - 1);
    
    // ----====---- Output Material File ----====----
    KRMaterial *new_material = new KRMaterial(context, pMaterial->GetName());
    
    std::string name = pMaterial->GetName();
    if(boost::starts_with(name, "ab_reflect_")) {
        new_material->setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE);
        size_t underscore_pos = name.find('_', 11);
        new_material->setReflectionCube(name.substr(11, underscore_pos - 11));
    } else if(boost::starts_with(name, "reflect_")) {
        size_t underscore_pos = name.find('_', 8);
        new_material->setReflectionCube(name.substr(8, underscore_pos - 8));
    } else if(boost::starts_with(name, "at_")) {
        new_material->setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_TEST);
    } else if(boost::starts_with(name, "ab_")) {
        new_material->setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDONESIDE);
    } else if(boost::starts_with(name, "ab2_")) {
        new_material->setAlphaMode(KRMaterial::KRMATERIAL_ALPHA_MODE_BLENDTWOSIDE);
    }

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

void LoadMesh(KRContext &context, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxMesh* pSourceMesh) {
    KFbxMesh* pMesh = pGeometryConverter->TriangulateMesh(pSourceMesh);
    
    int control_point_count = pMesh->GetControlPointsCount();
    KFbxVector4* control_points = pMesh->GetControlPoints();
    
    struct control_point_weight_info {
        float weights[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX];
        int bone_indexes[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX];
    };
    
    control_point_weight_info *control_point_weights = new control_point_weight_info[control_point_count];
    for(int control_point=0; control_point < control_point_count; control_point++) {
        for(int i=0; i<KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; i++) {
            control_point_weights[control_point].weights[i] = 0.0f;
            control_point_weights[control_point].bone_indexes[i] = 0;
        }
        
    }
    
    std::vector<std::string> bone_names;
    bool too_many_bone_weights = false;
    
    // Collect the top 4 bone weights per vertex ...
    int skin_count = pMesh->GetDeformerCount(FbxDeformer::eSkin);
    int target_bone_index = 0;
    for(int skin_index=0; skin_index<skin_count; skin_index++) {
        FbxSkin *skin = (FbxSkin *)pMesh->GetDeformer(skin_index, FbxDeformer::eSkin);
        int cluster_count = skin->GetClusterCount();
        printf("  Found skin with %i clusters.\n", cluster_count);
        for(int cluster_index=0; cluster_index < cluster_count; cluster_index++) {
            FbxCluster *cluster = skin->GetCluster(cluster_index);
            if(cluster->GetLinkMode() != FbxCluster::eNormalize) {
                printf("    Warning!  link mode not supported.\n");
            }
            std::string bone_name = GetFbxObjectName(cluster->GetLink());
            bone_names.push_back(bone_name);
            
            int cluster_control_point_count = cluster->GetControlPointIndicesCount();
            for(int control_point=0; control_point<cluster_control_point_count; control_point++) {
                control_point_weight_info &weight_info = control_point_weights[cluster->GetControlPointIndices()[control_point]];
                float bone_weight = cluster->GetControlPointWeights()[control_point];
                if(bone_weight > weight_info.weights[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX - 1]) {
                    if(weight_info.weights[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX - 1] != 0.0f) {
                        too_many_bone_weights = true;
                    }
                    weight_info.weights[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX - 1] = bone_weight;
                    weight_info.bone_indexes[KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX - 1] = target_bone_index;
                    for(int bone_index=KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX - 1; bone_index >=0; bone_index--) {
                        if(bone_weight > weight_info.weights[bone_index]) {
                            weight_info.weights[bone_index+1] = weight_info.weights[bone_index];
                            weight_info.bone_indexes[bone_index+1] = weight_info.bone_indexes[bone_index];
                            weight_info.weights[bone_index] = bone_weight;
                            weight_info.bone_indexes[bone_index] = target_bone_index;
                        }
                    }
                } else {
                    too_many_bone_weights = true;
                }
            }
            target_bone_index++;
        }
    }
    
    if(too_many_bone_weights) {
        printf("    WARNING! - Clipped bone weights to limit of %i per vertex (selecting largest weights and re-normalizing).\n", KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX);
    }
    // Normalize bone weights
    if(bone_names.size() > 0) {
        for(int control_point_index=0; control_point_index < control_point_count; control_point_index++) {
            control_point_weight_info &weight_info = control_point_weights[control_point_index];
            float total_weights = 0.0f;
            for(int i=0; i < KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; i++) {
                total_weights += weight_info.weights[i];
            }
            if(total_weights == 0.0f) total_weights = 1.0f; // Prevent any divisions by zero
            for(int i=0; i < KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; i++) {
                weight_info.weights[i] = weight_info.weights[i] / total_weights;
            }
        }
    }
    
    int polygon_count = pMesh->GetPolygonCount();
    int uv_count = pMesh->GetElementUVCount();
    int normal_count = pMesh->GetElementNormalCount();
    int tangent_count = pMesh->GetElementTangentCount();
    int elementmaterial_count = pMesh->GetElementMaterialCount();
    int material_count = pSourceMesh->GetNode()->GetMaterialCount(); // FINDME, TODO - To support instancing, material names should be stored in the instance rather than the mesh
    
    std::vector<std::vector<float> > bone_weights;
    std::vector<std::vector<int> > bone_indexes;
    
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
        KFbxSurfaceMaterial *pMaterial = pSourceMesh->GetNode()->GetMaterial(iMaterial);
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
                        
                        if(bone_names.size() > 0) {
                            control_point_weight_info &weight_info = control_point_weights[lControlPointIndex];
                            std::vector<int> vertex_bone_indexes;
                            std::vector<float> vertex_bone_weights;
                            for(int i=0; i<KRENGINE_MAX_BONE_WEIGHTS_PER_VERTEX; i++) {
                                vertex_bone_indexes.push_back(weight_info.bone_indexes[i]);
                                vertex_bone_weights.push_back(weight_info.weights[i]);
                            }
                            bone_indexes.push_back(vertex_bone_indexes);
                            bone_weights.push_back(vertex_bone_weights);
                        }
                        
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
        }
    }
    
    delete control_point_weights;
    
    KRMesh *new_mesh = new KRMesh(context, pSourceMesh->GetNode()->GetName());
    new_mesh->LoadData(vertices, uva, uvb, normals, tangents, submesh_starts, submesh_lengths, material_names, bone_names, bone_indexes, bone_weights,KRMesh::KRENGINE_MODEL_FORMAT_TRIANGLES);
    
    context.getModelManager()->addModel(new_mesh);
}

KRNode *LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, FbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    std::string name = GetFbxObjectName(pNode);
    
    KFbxMesh* pSourceMesh = (KFbxMesh*) pNode->GetNodeAttribute();

    if(KRMesh::GetLODCoverage(pNode->GetName()) == 100) {
        // If this is the full detail model, add an instance of it to the scene file
        std::string light_map = pNode->GetName();
        light_map.append("_lightmap");
        
        // FINDME, HACK - Until we have a GUI, we're using prefixes to select correct object type
        const char *node_name = pNode->GetName();
        if(strncmp(node_name, "physics_collider_", strlen("physics_collider_")) == 0) {
            return new KRCollider(parent_node->getScene(), GetFbxObjectName(pNode), pSourceMesh->GetNode()->GetName(), KRAKEN_COLLIDER_PHYSICS, 0.0f);
        } else if(strncmp(node_name, "audio_collider_", strlen("audio_collider_")) == 0) {
            return new KRCollider(parent_node->getScene(), GetFbxObjectName(pNode), pSourceMesh->GetNode()->GetName(), KRAKEN_COLLIDER_AUDIO, 1.0f);
        } else if(strncmp(node_name, "collider_", strlen("collider_")) == 0) {
            
            return new KRCollider(parent_node->getScene(), GetFbxObjectName(pNode), pSourceMesh->GetNode()->GetName(), KRAKEN_COLLIDER_PHYSICS | KRAKEN_COLLIDER_AUDIO, 1.0f);
        } else {
            return new KRModel(parent_node->getScene(), GetFbxObjectName(pNode), pSourceMesh->GetNode()->GetName(), light_map, 0.0f, true, false);
        }
    } else {
        return NULL;
    }
    
}

KRNode *LoadSkeleton(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode) {
    std::string name = GetFbxObjectName(pNode);
    KRBone *new_bone = new KRBone(parent_node->getScene(), name.c_str());
    return new_bone;
}

KRNode *LoadCamera(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode) {
    FbxCamera *camera = (FbxCamera *)pNode->GetNodeAttribute();
    const char *szName = pNode->GetName();
    
    KRCamera *new_camera = new KRCamera(parent_node->getScene(), szName);
    return new_camera;
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
