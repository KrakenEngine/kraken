#include "public/context.h"

#include "KRContext.h"

namespace kraken {

Context* sContext = nullptr;

class Context::impl
{
public:
  impl();
  ~impl();

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

Context::impl::impl()
{

}

Context::impl::~impl()
{

}

}; // namespace kraken
