//
//  KRModelView.cpp
//  Kraken Engine
//
//  Copyright 2024 Kearwood Gilbert. All rights reserved.
//  
//  Redistribution and use in source and binary forms, with or without modification, are
//  permitted provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of
//  conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright notice, this list
//  of conditions and the following disclaimer in the documentation and/or other materials
//  provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY KEARWOOD GILBERT ''AS IS'' AND ANY EXPRESS OR IMPLIED
//  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KEARWOOD GILBERT OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  The views and conclusions contained in the software and documentation are those of the
//  authors and should not be interpreted as representing official policies, either expressed
//  or implied, of Kearwood Gilbert.
//

#include "KREngine-common.h"

#include "KRModelView.h"
#include "KRViewport.h"

using namespace hydra;

KRModelView::KRModelView(KRViewport* viewport, const hydra::Matrix4& matModel)
  : m_viewport(viewport)
  , m_matModel(matModel)
{
  m_matModelInverse = matModel;
  m_matModelInverse.invert();
}

KRModelView::~KRModelView()
{

}

bool KRModelView::getShaderValue(ShaderValue value, void* buffer, size_t size) const
{
  if (size == sizeof(Vector3)) {
    switch (value) {
      case ShaderValue::camerapos_model_space:
      {
        // Transform location of camera to object space for calculation of specular halfVec
        Vector3 cameraPosObject = Matrix4::Dot(m_matModelInverse, m_viewport->getCameraPosition());
        memcpy(buffer, &cameraPosObject, sizeof(Vector3));
        return true;
      }
    }
  }
  return false;
}

