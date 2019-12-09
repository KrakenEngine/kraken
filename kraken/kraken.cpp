#include "public/kraken.h"

#include "KRContext.h"
#include "KRBundle.h"

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
  return KR_ERROR_NOT_IMPLEMENTED;
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
