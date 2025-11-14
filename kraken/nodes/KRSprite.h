//
//  KRSprite.h
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

#pragma once

#include "resources/KRResource.h"
#include "KRNode.h"
#include "resources/texture/KRTexture.h"
#include "resources/texture/KRTextureBinding.h"

class KRSprite : public KRNode
{
public:
  static void InitNodeInfo(KrNodeInfo* nodeInfo);

  KRSprite(KRScene& scene, std::string name);

  virtual ~KRSprite();
  virtual std::string getElementName() override;
  virtual tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent) override;
  virtual void loadXML(tinyxml2::XMLElement* e) override;

  void setSpriteTexture(std::string sprite_texture);
  void setSpriteAlpha(float alpha);
  float getSpriteAlpha() const;

  virtual void preStream(const KRViewport& viewport) override;
  virtual void render(RenderInfo& ri) override;

  virtual hydra::AABB getBounds() override;

protected:

  bool getShaderValue(ShaderValue value, float* output) const override;

  KRTextureBinding m_spriteTexture;
  float m_spriteAlpha;
};
