#include "public/kraken.h"

#include "KRContext.h"

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
  sContext->loadResource(pLoadResourceInfo->pResourcePath);
  return KR_SUCCESS;
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
  return KR_ERROR_NOT_IMPLEMENTED;
}

KrResult KrMapResource(const KrMapResourceInfo* pMapResourceInfo)
{
  return KR_ERROR_NOT_IMPLEMENTED;
}

KrResult KrUnmapResource(const KrUnmapResourceInfo* pUnmapResourceInfo)
{
  return KR_ERROR_NOT_IMPLEMENTED;
}

KrResult KrCreateBundle(const KrCreateBundleInfo* pCreateBundleInfo)
{
  return KR_ERROR_NOT_IMPLEMENTED;
}

KrResult KrMoveToBundle(const KrMoveToBundleInfo* pMoveToBundleInfo)
{
  return KR_ERROR_NOT_IMPLEMENTED;
}
