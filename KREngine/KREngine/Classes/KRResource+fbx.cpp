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
#include "KRModel.h"
#include "KRMaterial.h"
#include "KRLight.h"
#include "KRPointLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRNode.h"
#include "KRScene.h"

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(pSdkManager->GetIOSettings()))
#endif

void InitializeSdkObjects(KFbxSdkManager*& pSdkManager, KFbxScene*& pScene);
void DestroySdkObjects(KFbxSdkManager* pSdkManager);
bool LoadScene(KFbxSdkManager* pSdkManager, KFbxDocument* pScene, const char* pFilename);
void LoadNode(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
void LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode);
void LoadLight(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode);


std::vector<KRResource *> KRResource::LoadFbx(KRContext &context, const std::string& path)
{
    std::vector<KRResource *> resources;
    KRScene *pScene = new KRScene(context, KRResource::GetFileBase(path));
    resources.push_back(pScene);
    
    
    
    KFbxSdkManager* lSdkManager = NULL;
    KFbxScene* pFbxScene = NULL;
    bool lResult;
    KFbxGeometryConverter *pGeometryConverter = NULL;
    
    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, pFbxScene);
    
    // Initialize Geometry Converter
    pGeometryConverter = new KFbxGeometryConverter(lSdkManager);
    
    // Load the scene.
    lResult = LoadScene(lSdkManager, pFbxScene, path.c_str());
    
    
    // ----====---- Walk Through Scene ----====----
    
    int i;
    KFbxNode* pNode = pFbxScene->GetRootNode();
    
    if(pNode)
    {
        for(i = 0; i < pNode->GetChildCount(); i++)
        {
            LoadNode(pScene->getRootNode(), resources, pGeometryConverter, pNode->GetChild(i));
        }
    }
    
    DestroySdkObjects(lSdkManager);
    
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

void LoadNode(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    KFbxVector4 lTmpVector;
    
    
    fbxDouble3 local_rotation = pNode->LclRotation.Get(); // pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
    fbxDouble3 local_translation = pNode->LclTranslation.Get(); // pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
    fbxDouble3 local_scale = pNode->LclScaling.Get(); // pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);
    
    printf("        Translation: %f %f %f\n", local_translation[0], local_translation[1], local_translation[2]);
    printf("        Rotation:    %f %f %f\n", local_rotation[0], local_rotation[1], local_rotation[2]);
    printf("        Scaling:     %f %f %f\n", local_scale[0], local_scale[1], local_scale[2]);
    
    KFbxNodeAttribute::EAttributeType attribute_type = (pNode->GetNodeAttribute()->GetAttributeType());
    switch(attribute_type) {
        case KFbxNodeAttribute::eMESH:
            LoadMesh(parent_node, resources, pGeometryConverter, pNode);
            break;
        case KFbxNodeAttribute::eLIGHT:
            LoadLight(parent_node, resources, pNode);
            break;
    }
    
    
    // Load child nodes
    for(int i = 0; i < pNode->GetChildCount(); i++)
    {
        LoadNode(parent_node, resources, pGeometryConverter, pNode->GetChild(i));
    }
}

void LoadMesh(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxGeometryConverter *pGeometryConverter, KFbxNode* pNode) {
    std::string light_map = pNode->GetName();
    light_map.append("_lightmap");
    
    KRInstance *new_instance = new KRInstance(parent_node->getScene(), pNode->GetName(), pNode->GetName(), light_map, 0.0f);
    fbxDouble3 local_rotation = pNode->LclRotation.Get(); // pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
    fbxDouble3 local_translation = pNode->LclTranslation.Get(); // pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
    fbxDouble3 local_scale = pNode->LclScaling.Get(); // pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);
    /*
    fbxDouble3 local_rotation = pNode->GetGeometricRotation(KFbxNode::eDESTINATION_SET);
    fbxDouble3 local_translation = pNode->GetGeometricTranslation(KFbxNode::eDESTINATION_SET);
    fbxDouble3 local_scale = pNode->GetGeometricScaling(KFbxNode::eDESTINATION_SET);
     */
    new_instance->setLocalRotation(KRVector3(local_rotation[0], local_rotation[1], local_rotation[2]));
    new_instance->setLocalTranslation(KRVector3(local_translation[0], local_translation[1], local_translation[2]));
    new_instance->setLocalScale(KRVector3(local_scale[0], local_scale[1], local_scale[2]));
    parent_node->addChild(new_instance);
    
    
    printf("Mesh: %s\n", pNode->GetName());
    KFbxMesh* pSourceMesh = (KFbxMesh*) pNode->GetNodeAttribute();
    KFbxMesh* pMesh = pGeometryConverter->TriangulateMesh(pSourceMesh);
    
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
                    KFbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
                    if(leMat) {
                        if (leMat->GetReferenceMode() == KFbxGeometryElement::eINDEX || leMat->GetReferenceMode() == KFbxGeometryElement::eINDEX_TO_DIRECT) {
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
                            KFbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
                            
                            if(leTangent->GetMappingMode() == KFbxGeometryElement::eBY_POLYGON_VERTEX) {
                                switch (leTangent->GetReferenceMode()) {
                                    case KFbxGeometryElement::eDIRECT:
                                        new_tangent = leTangent->GetDirectArray().GetAt(lControlPointIndex);
                                        break;
                                    case KFbxGeometryElement::eINDEX_TO_DIRECT:
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
            
            KFbxPropertyDouble3 lKFbxDouble3;
            KFbxPropertyDouble1 lKFbxDouble1;
            
            if (pMaterial->GetClassId().Is(KFbxSurfacePhong::ClassId)) {
                // We found a Phong material.
                
                // Ambient Color
                lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Ambient;
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
                
                // Reflection color
                lKFbxDouble3 =((KFbxSurfacePhong *) pMaterial)->Reflection;
                
                // Reflection factor
                lKFbxDouble1 =((KFbxSurfacePhong *) pMaterial)->ReflectionFactor;
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
                KFbxTexture* pTexture = KFbxCast <KFbxTexture> (pProperty.GetSrcObject(KFbxTexture::ClassId,0));
                assert(!pTexture->GetSwapUV());
                assert(pTexture->GetCroppingTop() == 0);
                assert(pTexture->GetCroppingLeft() == 0);
                assert(pTexture->GetCroppingRight() == 0);
                assert(pTexture->GetCroppingBottom() == 0);
                assert(pTexture->GetWrapModeU() == KFbxTexture::eREPEAT);
                assert(pTexture->GetWrapModeV() == KFbxTexture::eREPEAT);
                assert(pTexture->GetRotationU() == 0.0f);
                assert(pTexture->GetRotationV() == 0.0f);
                assert(pTexture->GetRotationW() == 0.0f);
                
                KFbxFileTexture *pFileTexture = KFbxCast<KFbxFileTexture>(pTexture);
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
                KFbxTexture* pTexture = KFbxCast <KFbxTexture> (pProperty.GetSrcObject(KFbxTexture::ClassId,0));
                KFbxFileTexture *pFileTexture = KFbxCast<KFbxFileTexture>(pTexture);
                if(pFileTexture) {
                    new_material->setSpecularMap(KRResource::GetFileBase(pFileTexture->GetFileName()), KRVector2(pTexture->GetScaleU(), pTexture->GetScaleV()),  KRVector2(pTexture->GetTranslationU(), pTexture->GetTranslationV()));
                }
            }
            
            // Normal Map Texture
            pProperty = pMaterial->FindProperty(KFbxSurfaceMaterial::sNormalMap);
            if(pProperty.GetSrcObjectCount(KFbxLayeredTexture::ClassId) > 0) {
                printf("Warning! Layered textures not supported.\n");
            }
            
            texture_count = pProperty.GetSrcObjectCount(KFbxTexture::ClassId);
            if(texture_count > 1) {
                printf("Error! Multiple normal map textures not supported.\n");
            } else if(texture_count == 1) {
                KFbxTexture* pTexture = KFbxCast <KFbxTexture> (pProperty.GetSrcObject(KFbxTexture::ClassId,0));
                KFbxFileTexture *pFileTexture = KFbxCast<KFbxFileTexture>(pTexture);
                if(pFileTexture) {
                    new_material->setNormalMap(KRResource::GetFileBase(pFileTexture->GetFileName()), KRVector2(pTexture->GetScaleU(), pTexture->GetScaleV()),  KRVector2(pTexture->GetTranslationU(), pTexture->GetTranslationV()));
                }
            }
            
            bool bFound = false;
            vector<KRResource *>::iterator resource_itr = resources.begin();
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

}

void LoadLight(KRNode *parent_node, std::vector<KRResource *> &resources, KFbxNode* pNode) {
    const GLfloat PI = 3.14159265;
    const GLfloat d2r = PI * 2 / 360;
    
    KFbxLight* pLight = (KFbxLight*) pNode->GetNodeAttribute();
    const char *szName = pNode->GetName();
    
    fbxDouble3 light_color = pLight->Color.Get();
    fbxDouble1 light_intensity = pLight->Intensity.Get();
    fbxDouble1 light_hotspot = pLight->HotSpot.Get(); // light inner cone angle (in degrees). Also know as the HotSpot
    fbxDouble1 light_coneangle = pLight->ConeAngle.Get(); // light outer cone angle (in degrees). Also known as the Falloff
    KFbxLight::EDecayType light_decaytype = pLight->DecayType.Get(); // decay type
    fbxDouble1 light_decaystart = pLight->DecayStart.Get(); // decay start distance
    
    
//    KFbxLight::eNONE         - does not attenuate with distance
//    KFbxLight::eLINEAR       - attenuation of 1/d
//    KFbxLight::eQUADRATIC    - attenuation of 1/d^2
//    KFbxLight::eCUBIC        - attenuation of 
    
    KFbxVector4 v4; // Default translation values
    v4 = pNode->LclTranslation.Get();
    
    //KFbxVector4 light_translation = pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
    //KFbxVector4 light_rotation = pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
    //KFbxVector4 light_scaling = pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);
    
    
    //KRVector3 translation = KRVector3(light_translation[0], light_translation[1], light_translation[2]);
    
    KRLight *new_light = NULL;
    
    switch(pLight->LightType.Get()) {
        case KFbxLight::ePOINT:
        {
            KRPointLight *l = new KRPointLight(parent_node->getScene(), szName);
            new_light = l;
            
        }
            break;
        case KFbxLight::eDIRECTIONAL:
        {
            KRDirectionalLight *l = new KRDirectionalLight(parent_node->getScene(), szName);
            new_light = l;
        }
            break;
        case KFbxLight::eSPOT:
        {
            KRSpotLight *l = new KRSpotLight(parent_node->getScene(), szName);
            l->setInnerAngle(light_hotspot * d2r);
            l->setOuterAngle(light_coneangle * d2r);
            new_light = l;
        }
            break;
        case KFbxLight::eAREA:
            // Not supported yet
            break;
    }
    
    if(new_light) {
        fbxDouble3 local_rotation = pNode->LclRotation.Get(); // pNode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
        fbxDouble3 local_translation = pNode->LclTranslation.Get(); // pNode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
        fbxDouble3 local_scale = pNode->LclScaling.Get(); // pNode->GetGeometricScaling(KFbxNode::eSOURCE_SET);
        new_light->setLocalRotation(KRVector3(local_rotation[0], local_rotation[1], local_rotation[2]));
        new_light->setLocalTranslation(KRVector3(local_translation[0], local_translation[1], local_translation[2]));
        new_light->setLocalScale(KRVector3(local_scale[0], local_scale[1], local_scale[2]));
        new_light->setColor(KRVector3(light_color[0], light_color[1], light_color[2]));
        new_light->setIntensity(light_intensity);
        new_light->setDecayStart(light_decaystart);
        
        parent_node->addChild(new_light);
    }
    
}

