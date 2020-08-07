#include "public/kraken.h"

#include "KRContext.h"
#include "KRBundle.h"
#include "hydra.h"
#include "KRLODSet.h"
#include "KRLODGroup.h"
#include "KRPointLight.h"
#include "KRDirectionalLight.h"
#include "KRSpotLight.h"
#include "KRSprite.h"
#include "KRModel.h"
#include "KRCollider.h"
#include "KRBone.h"
#include "KRLocator.h"
#include "KRAudioSource.h"
#include "KRAmbientZone.h"
#include "KRReverbZone.h"

using namespace kraken;

namespace {

KRContext* sContext = nullptr;

}; // anonysmous namespace

KrResult KrInitialize(const KrInitializeInfo* pInitializeInfo)
{
  if (!sContext) {
    sContext = new KRContext(pInitializeInfo);
  }
  return KR_SUCCESS;
}

KrResult KrShutdown()
{
  if (sContext) {
    delete sContext;
    sContext = nullptr;
  }
  return KR_SUCCESS;
}

KrResult KrCreateWindowSurface(const KrCreateWindowSurfaceInfo* pCreateWindowSurfaceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->createWindowSurface(pCreateWindowSurfaceInfo);
}

KrResult KrDeleteWindowSurface(const KrDeleteWindowSurfaceInfo* pDeleteWindowSurfaceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->deleteWindowSurface(pDeleteWindowSurfaceInfo);
}

KrResult KrLoadResource(const KrLoadResourceInfo* pLoadResourceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->loadResource(pLoadResourceInfo);
}

KrResult KrUnloadResource(const KrUnloadResourceInfo* pUnloadResourceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->unloadResource(pUnloadResourceInfo);
}

KrResult KrSaveResource(const KrSaveResourceInfo* pSaveResourceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->saveResource(pSaveResourceInfo);
}

KrResult KrMapResource(const KrMapResourceInfo* pMapResourceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->mapResource(pMapResourceInfo);
}

KrResult KrUnmapResource(const KrUnmapResourceInfo* pUnmapResourceInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->unmapResource(pUnmapResourceInfo);
}

KrResult KrCreateBundle(const KrCreateBundleInfo* pCreateBundleInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->createBundle(pCreateBundleInfo);
}

KrResult KrMoveToBundle(const KrMoveToBundleInfo* pMoveToBundleInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->moveToBundle(pMoveToBundleInfo);
}

KrResult KrCompileAllShaders(const KrCompileAllShadersInfo* pCompileAllShadersInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->compileAllShaders(pCompileAllShadersInfo);
}

KrResult KrCreateScene(const KrCreateSceneInfo* pCreateSceneInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->createScene(pCreateSceneInfo);
}

KrResult KrFindNodeByName(const KrFindNodeByNameInfo* pFindNodeByNameInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->findNodeByName(pFindNodeByNameInfo);
}

KrResult KrFindAdjacentNodes(const KrFindAdjacentNodesInfo* pFindAdjacentNodesInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->findAdjacentNodes(pFindAdjacentNodesInfo);
}

KrResult KrSetNodeLocalTransform(const KrSetNodeLocalTransformInfo* pSetNodeLocalTransform)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->setNodeLocalTransform(pSetNodeLocalTransform);
}

KrResult KrSetNodeWorldTransform(const KrSetNodeWorldTransformInfo* pSetNodeWorldTransform)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->setNodeWorldTransform(pSetNodeWorldTransform);
}

KrResult KrDeleteNode(const KrDeleteNodeInfo* pDeleteNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->deleteNode(pDeleteNodeInfo);
}

KrResult KrDeleteNodeChildren(const KrDeleteNodeChildrenInfo* pDeleteNodeChildrenInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->deleteNodeChildren(pDeleteNodeChildrenInfo);
}

KrResult KrAppendBeforeNode(const KrAppendBeforeNodeInfo* pAppendBeforeNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->appendBeforeNode(pAppendBeforeNodeInfo);
}

KrResult KrAppendAfterNode(const KrAppendAfterNodeInfo* pAppendAfterNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->appendAfterNode(pAppendAfterNodeInfo);
}

KrResult KrAppendFirstChildNode(const KrAppendFirstChildNodeInfo* pAppendFirstChildNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->appendFirstChildNode(pAppendFirstChildNodeInfo);
}

KrResult KrAppendLastChildNode(const KrAppendLastChildNodeInfo* pAppendLastChildNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->appendLastChildNode(pAppendLastChildNodeInfo);
}

KrResult KrUpdateNode(const KrUpdateNodeInfo* pUpdateNodeInfo)
{
  if (!sContext) {
    return KR_ERROR_NOT_INITIALIZED;
  }
  return sContext->updateNode(pUpdateNodeInfo);
}

KrResult KrInitNodeInfo(KrNodeInfo* pNodeInfo, KrStructureType nodeType)
{
  pNodeInfo->sType = nodeType;
  switch (nodeType) {
  case KR_STRUCTURE_TYPE_NODE:
    KRNode::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_CAMERA:
    KRCamera::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_LOD_SET:
    KRLODSet::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_LOD_GROUP:
    KRLODGroup::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_POINT_LIGHT:
    KRPointLight::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_DIRECTIONAL_LIGHT:
    KRDirectionalLight::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_SPOT_LIGHT:
    KRSpotLight::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_SPRITE:
    KRSprite::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_MODEL:
    KRModel::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_COLLIDER:
    KRCollider::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_BONE:
    KRBone::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_LOCATOR:
    KRLocator::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_AUDIO_SOURCE:
    KRAudioSource::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_AMBIENT_ZONE:
    KRAmbientZone::InitNodeInfo(pNodeInfo);
    break;
  case KR_STRUCTURE_TYPE_NODE_REVERB_ZONE:
    KRReverbZone::InitNodeInfo(pNodeInfo);
    break;
  default:
    return KR_ERROR_INCORRECT_TYPE;
  }
  return KR_SUCCESS;
}
