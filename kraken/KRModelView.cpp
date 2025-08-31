//
//  KRModelView.cpp
//  Kraken Engine
//
//  Copyright 2025 Kearwood Gilbert. All rights reserved.
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

  m_matModelView = matModel * viewport->getViewMatrix();
}

KRModelView::~KRModelView()
{

}

bool KRModelView::getShaderValue(ShaderValue value, Vector3* output) const
{

    switch (value) {
      case ShaderValue::camerapos_model_space:
      {
        // Transform location of camera to object space for calculation of specular halfVec
        *output = Matrix4::Dot(m_matModelInverse, m_viewport->getCameraPosition());
        return true;
      }
      case ShaderValue::view_space_model_origin:
      {
        // Origin point of model space is the light source position.  No perspective, so no w divide required
        *output = Matrix4::Dot(m_matModelView, Vector3::Zero());
        return true;
      }
    }
    return false;
}
bool KRModelView::getShaderValue(ShaderValue value, Matrix4* output) const
{
  switch (value) {
    case ShaderValue::model_matrix:
    {
      *output = m_matModel;
      return true;
    }
    case ShaderValue::model_view:
    {
      *output = m_matModelView;
      return true;
    }
    case ShaderValue::model_view_inverse_transpose:
    {
      Matrix4 matModelViewInverseTranspose = m_matModelView;
      matModelViewInverseTranspose.transpose();
      matModelViewInverseTranspose.invert();
      *output = matModelViewInverseTranspose;
      return true;
    }
    case ShaderValue::model_inverse_transpose:
    {
      Matrix4 matModelInverseTranspose = m_matModel;
      matModelInverseTranspose.transpose();
      matModelInverseTranspose.invert();
      *output = matModelInverseTranspose;
      return true;
    }
    case ShaderValue::invmvp_no_translate:
    {
      Matrix4 matInvMVPNoTranslate = m_matModelView;
      // Remove the translation
      matInvMVPNoTranslate.getPointer()[3] = 0;
      matInvMVPNoTranslate.getPointer()[7] = 0;
      matInvMVPNoTranslate.getPointer()[11] = 0;
      matInvMVPNoTranslate.getPointer()[12] = 0;
      matInvMVPNoTranslate.getPointer()[13] = 0;
      matInvMVPNoTranslate.getPointer()[14] = 0;
      matInvMVPNoTranslate.getPointer()[15] = 1.0;
      matInvMVPNoTranslate = matInvMVPNoTranslate * m_viewport->getProjectionMatrix();
      matInvMVPNoTranslate.invert();
      *output = matInvMVPNoTranslate;
      return true;
    }
    case ShaderValue::mvp:
    {
      Matrix4 mvpMatrix = m_matModel * m_viewport->getViewProjectionMatrix();
      *output = mvpMatrix;
      return true;
    }
    case ShaderValue::invmvp:
    {
      Matrix4 mvpMatrix = m_matModel * m_viewport->getViewProjectionMatrix();
      Matrix4 invMVP = Matrix4::Invert(mvpMatrix);
      *output = invMVP;
      return true;
    }
  }

  return false;
}

