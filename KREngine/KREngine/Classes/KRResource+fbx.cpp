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
    int uv_count = pMesh->GetElementUVCount();
    int normal_count = pMesh->GetElementNormalCount();
    int tangent_count = pMesh->GetElementTangentCount();
    int material_count = pMesh->GetElementMaterialCount();
    
        
    printf("  Polygon Count: %i (before triangulation: %i)\n", polygon_count, pSourceMesh->GetPolygonCount());

    std::vector<KRVector3> vertices;
    std::vector<KRVector2> uva;
    std::vector<KRVector2> uvb;
    std::vector<KRVector3> normals;
    std::vector<KRVector3> tangents;
    std::vector<int> submesh_lengths;
    std::vector<int> submesh_starts;
    std::vector<std::string> material_names;
    
    
    int source_vertex_id = 0;
    int dest_vertex_id = 0;
    KFbxGeometryElementMaterial* pMaterial = NULL;
    int iMaterial = -1;
    int mat_vertex_count = 0;
    int mat_vertex_start = 0;

    for(int iPolygon = 0; iPolygon < polygon_count; iPolygon++) {
        int lPolygonSize = pMesh->GetPolygonSize(iPolygon);
        if(lPolygonSize != 3) {
            source_vertex_id += lPolygonSize;
            printf("    Warning - Poly with %i vertices found. Expecting only triangles.", lPolygonSize);
        } else {
            // ----====---- Read SubMesh / Material Mapping ----====----
            int iPrevMat = iMaterial;
            KFbxGeometryElementMaterial *pPrevMat = pMaterial;
            iMaterial = -1;
            pMaterial = NULL;
            for (int l = 0; l < material_count; l++)
            {
                KFbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
                if(leMat) {
                    if (leMat->GetReferenceMode() == KFbxGeometryElement::eINDEX || leMat->GetReferenceMode() == KFbxGeometryElement::eINDEX_TO_DIRECT) {
                        int new_id = leMat->GetIndexArray().GetAt(iPolygon);
                        if(new_id >= 0) {
                            iMaterial = new_id;
                            pMaterial = leMat;
                        }
                    }
                }
            }
            
            if(iMaterial != iPrevMat) {
                if(pPrevMat && mat_vertex_count) {
                    submesh_starts.push_back(mat_vertex_start);
                    submesh_lengths.push_back(mat_vertex_count);
                    material_names.push_back(pNode->GetMaterial(iMaterial)->GetName());
                    printf("  Material \"%s\" from %i to %i\n", pNode->GetMaterial(iMaterial)->GetName(), mat_vertex_start, mat_vertex_count + mat_vertex_start - 1);
                }
                
                mat_vertex_count = 0;
                mat_vertex_start = dest_vertex_id;
            }

            
            
            // ----====---- Read Vertex-level Attributes ----====----
            for(int iVertex=0; iVertex<3; iVertex++) {
                // ----====---- Read Vertex Position ----====----
                int lControlPointIndex = pMesh->GetPolygonVertex(iPolygon, iVertex);
                KFbxVector4 v = control_points[lControlPointIndex];
                vertices.push_back(KRVector3(v[0], v[1], v[2]));
                
                
                // ----====---- Read UVs ----====----
                for (int l = 0; l < uv_count; ++l)
                {
                    KFbxVector2 uv;
                    KFbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
                    
                    switch (leUV->GetMappingMode()) {
                        case KFbxGeometryElement::eBY_CONTROL_POINT:
                            switch (leUV->GetReferenceMode()) {
                            case KFbxGeometryElement::eDIRECT:
                                uv = leUV->GetDirectArray().GetAt(lControlPointIndex);
                                break;
                            case KFbxGeometryElement::eINDEX_TO_DIRECT:
                            {
                                int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
                                uv = leUV->GetDirectArray().GetAt(id);
                            }
                                break;
                            default:
                                break; // other reference modes not shown here!
                        }
                            break;
                            
                        case KFbxGeometryElement::eBY_POLYGON_VERTEX:
                        {
                            int lTextureUVIndex = pMesh->GetTextureUVIndex(iPolygon, iVertex);
                            switch (leUV->GetReferenceMode())
                            {
                                case KFbxGeometryElement::eDIRECT:
                                case KFbxGeometryElement::eINDEX_TO_DIRECT:
                                {
                                    uv = leUV->GetDirectArray().GetAt(lTextureUVIndex);
                                }
                                    break;
                                default:
                                    break; // other reference modes not shown here!
                            }
                        }
                            break;
                            
                        case KFbxGeometryElement::eBY_POLYGON: // doesn't make much sense for UVs
                        case KFbxGeometryElement::eALL_SAME:   // doesn't make much sense for UVs
                        case KFbxGeometryElement::eNONE:       // doesn't make much sense for UVs
                            break;
                    }
                    
                    if(l == 0) {
                        uva.push_back(KRVector2(uv[0], uv[1]));
                    } else if(l == 1) {
                        uvb.push_back(KRVector2(uv[0], uv[1]));
                    }
                }
                
                // ----====---- Read Normals ----====----
                for(int l = 0; l < normal_count; ++l)
                {
                    KFbxVector4 new_normal;
                    KFbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
                    
                    if(leNormal->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX) {
                        switch (leNormal->GetReferenceMode())
                        {
                            case KFbxGeometryElement::eDIRECT:
                                new_normal = leNormal->GetDirectArray().GetAt(source_vertex_id);
                                break;
                            case KFbxGeometryElement::eINDEX_TO_DIRECT:
                            {
                                int id = leNormal->GetIndexArray().GetAt(source_vertex_id);
                                new_normal = leNormal->GetDirectArray().GetAt(id);
                            }
                                break;
                            default:
                                break; // other reference modes not shown here!
                        }
                    }
                    if(l == 0) {
                        normals.push_back(KRVector3(new_normal[0], new_normal[1], new_normal[2]));
                    }
                    
                }
                
                // ----====---- Read Tangents ----====----
                for(int l = 0; l < tangent_count; ++l)
                {
                    KFbxVector4 new_tangent;
                    KFbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
                    
                    if(leTangent->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX) {
                        switch (leTangent->GetReferenceMode()) {
                            case KFbxGeometryElement::eDIRECT:
                                new_tangent = leTangent->GetDirectArray().GetAt(source_vertex_id);
                                break;
                            case KFbxGeometryElement::eINDEX_TO_DIRECT:
                            {
                                int id = leTangent->GetIndexArray().GetAt(source_vertex_id);
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
    
    // ----====---- Complete last material / submesh ----====----
    if(iMaterial >= 0 && mat_vertex_count) {
        submesh_starts.push_back(mat_vertex_start);
        submesh_lengths.push_back(mat_vertex_count);
        material_names.push_back(pNode->GetMaterial(iMaterial)->GetName());
        printf("  Material \"%s\" from %i to %i\n", pNode->GetMaterial(iMaterial)->GetName(), mat_vertex_start, mat_vertex_count + mat_vertex_start - 1);
    }
    
    
    // ----====---- Generate Output Mesh Object ----====----

    KRMesh *new_mesh = new KRMesh(pNode->GetName());
    new_mesh->LoadData(vertices, uva, normals, tangents, submesh_starts, submesh_lengths, material_names);
    resources.push_back(new_mesh);
}

