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
#include <assert.h>
#include <vector.h>

#include <fbxsdk.h>


#include "KRResource.h"
#include "KRMesh.h"

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif

void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene);
void DestroySdkObjects(KFbxSdkManager* pSdkManager);
bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename);
void LoadNode(std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
void LoadMesh(std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);


std::vector<KRResource *> KRResource::LoadFbx(const std::string& path)
{
    std::vector<KRResource *> resources;
    
    
    KFbxSdkManager* lSdkManager = NULL;
    KFbxScene* pScene = NULL;
    bool lResult;
    KFbxGeometryConverter *pGeometryConverter = NULL;
    
    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, pScene);
    
    // Initialize Geometry Converter
    pGeometryConverter = new KFbxGeometryConverter(lSdkManager);
    
    // Load the scene.
    lResult = LoadScene(lSdkManager, pScene, path.c_str());
    
    
    // ----====---- Walk Through Scene ----====----
    
    int i;
    KFbxNode* pNode = pScene->GetRootNode();
    
    if(pNode)
    {
        for(i = 0; i < pNode->GetChildCount(); i++)
        {
            LoadNode(resources, pGeometryConverter, pNode->GetChild(i));
        }
    }
    
    DestroySdkObjects(lSdkManager);
    
    /*
    
    KRMesh *new_mesh = new KRMesh(KRResource::GetFileBase(path));
    
    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_lengths;
    std::vector<int> submesh_starts;
    std::vector<std::string> material_names;
    
    new_mesh->LoadData(vertices, uva, normals, tangents, submesh_starts, submesh_lengths, material_names);
    resources.push_back(new_mesh);
    */
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
	KString lPath = KFbxGetApplicationDirectory();
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
    int i, lAnimStackCount;
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
        
        if (lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
            lImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
        {
            printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
            printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        }
        
        return false;
    }
    
    printf("FBX version number for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
    
    if (lImporter->IsFBX())
    {
        printf("FBX version number for file %s is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
        
        // From this point, it is possible to access animation stack information without
        // the expense of loading the entire file.
        
        printf("Animation Stack Information\n");
        
        lAnimStackCount = lImporter->GetAnimStackCount();
        
        printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
        printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
        printf("\n");
        
        for(i = 0; i < lAnimStackCount; i++)
        {
            KFbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);
            
            printf("    Animation Stack %d\n", i);
            printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
            printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());
            
            // Change the value of the import name if the animation stack should be imported 
            // under a different name.
            printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());
            
            // Set the value of the import state to false if the animation stack should be not
            // be imported. 
            printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
            printf("\n");
        }
        
        // Set the import states. By default, the import states are always set to 
        // true. The code below shows how to change these states.
        IOS_REF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOS_REF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOS_REF.SetBoolProp(IMP_FBX_LINK,            true);
        IOS_REF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOS_REF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOS_REF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
    }
    
    // Import the scene.
    lStatus = lImporter->Import(pScene);
    
    if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
    {
        printf("Please enter password: ");
        
        lPassword[0] = '\0';
        
        scanf("%s", lPassword);
        KString lString(lPassword);
        
        IOS_REF.SetStringProp(IMP_FBX_PASSWORD,      lString);
        IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);
        
        lStatus = lImporter->Import(pScene);
        
        if(lStatus == false && lImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
        {
            printf("\nPassword is wrong, import aborted.\n");
        }
    }
    
    // Destroy the importer.
    lImporter->Destroy();
    
    return lStatus;
}

void LoadNode(std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    KFbxVector4 lTmpVector;
    
    /*
    lTmpVector = pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
    printf("        Translation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
    lTmpVector = pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
    printf("        Rotation:    %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
    lTmpVector = pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);
    printf("        Scaling:     %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
    */
    
    KFbxNodeAttribute::EAttributeType attribute_type = (pNode->GetNodeAttribute()->GetAttributeType());
    switch(attribute_type) {
        case KFbxNodeAttribute::eMESH:
            LoadMesh(resources, pGeometryConverter, pNode);
            break;
    }
    
    
    // Load child nodes
    for(int i = 0; i < pNode->GetChildCount(); i++)
    {
        LoadNode(resources, pGeometryConverter, pNode->GetChild(i));
    }
}

void LoadMesh(std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    printf("Mesh: %s\n", pNode->GetName());
    KFbxMesh* pSourceMesh = (KFbxMesh*) pNode->GetNodeAttribute();
    KFbxMesh* pMesh = pGeometryConverter->TriangulateMesh(pSourceMesh);
    
    KFbxVector4* control_points = pMesh->GetControlPoints(); 
    
    int polygon_count = pMesh->GetPolygonCount();
    
    printf("  Polygon Count: %i (before triangulation: %i)\n", polygon_count, pSourceMesh->GetPolygonCount());

    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_lengths;
    std::vector<int> submesh_starts;
    std::vector<std::string> material_names;
    
    for(int iPolygon = 0; iPolygon < polygon_count; iPolygon++) {
        int lPolygonSize = pMesh->GetPolygonSize(iPolygon);
        if(lPolygonSize != 3) {
            assert(lPolygonSize == 3);
        }
    }
    

    KRMesh *new_mesh = new KRMesh(pNode->GetName());
    new_mesh->LoadData(vertices, uva, normals, tangents, submesh_starts, submesh_lengths, material_names);
    resources.push_back(new_mesh);
}

