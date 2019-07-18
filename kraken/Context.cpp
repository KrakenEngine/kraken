#include "public/context.h"

#include "KRContext.h"

namespace kraken {

Context* sContext = nullptr;

class Context::impl
{
public:
  impl();
  ~impl();
  bool loadResource(const char* szPath);

private:
  KRContext mContext;
};

/* static */
Context* Context::Get()
{
  if (!sContext) {
    sContext = new Context();
  }
  return sContext;
}

Context::Context()
{
  mImpl = new impl();
}

Context::~Context()
{
  delete mImpl;
}

bool Context::loadResource(const char* szPath)
{
  return mImpl->loadResource(szPath);
}

bool Context::impl::loadResource(const char* szPath)
{
  mContext.loadResource(szPath);
  // TODO: update KRContext::loadResource to return success/fail boolean
  return true;
}

Context::impl::impl()
{

}

Context::impl::~impl()
{

}

}; // namespace kraken
