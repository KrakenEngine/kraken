//
//  KRNode.h
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
#include "resources/KRResourceRequest.h"
#include "resources/KRResourceBinding.h"
#include "KRViewport.h"
#include "KROctreeNode.h"
#include "KRBehavior.h"
#include "KRShaderReflection.h"
#include <type_traits>

using namespace kraken;

namespace hydra {
class Matrix4;
class AABB;
class Vector3;
} // namespace kraken
class KRCamera;
class KRPipelineManager;
class KRMeshManager;
class KRMaterialManager;
class KRTextureManager;
class KRContext;
class KRScene;
class KRSurface;
class KRResourceBinding;

class KRNode;
class KRPointLight;
class KRSpotLight;
class KRDirectionalLight;
class KRRenderPass;
class KRPipeline;
namespace tinyxml2 {
class XMLNode;
class XMLAttribute;
}

template <typename T, class config>
class KRNodeProperty
{
public:
  static constexpr decltype(config::defaultVal) defaultVal = config::defaultVal;
  static constexpr const char* name = config::name;
  KRNodeProperty()
    : val(config::defaultVal)
  {
  }

  KRNodeProperty(T& val)
    : val(val)
  {
  }

  KRNodeProperty& operator=(const T& v)
  {
    val = v;
    return *this;
  }

  operator const T& () const
  {
    return val;
  }

  void save(tinyxml2::XMLElement* element) const
  {
    if constexpr (std::is_same<T, bool>::value) {
      element->SetAttribute(config::name, val ? "true" : "false");
    } else if constexpr (std::is_same<T, hydra::Vector3>::value) {
      kraken::setXMLAttribute(config::name, element, val, config::defaultVal);
    } else if constexpr (std::is_same<T, hydra::AABB>::value) {
      kraken::setXMLAttribute(config::name, element, val, config::defaultVal);
    } else if constexpr (std::is_same<T, std::string>::value) {
      element->SetAttribute(config::name, val.c_str());
    } else if constexpr (std::is_base_of<KRResourceBinding, T>::value) {
      element->SetAttribute(config::name, val.getName().c_str());
    } else {
      element->SetAttribute(config::name, val);
    }
  }

  void load(tinyxml2::XMLElement* element)
  {
    if constexpr (std::is_same<T, int>::value) {
      if (element->QueryIntAttribute(config::name, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, unsigned int>::value) {
        if (element->QueryUnsignedAttribute(config::name, &val) != tinyxml2::XML_SUCCESS) {
          val = config::defaultVal;
        }
    } else if constexpr (std::is_same<T, float>::value) {
      if (element->QueryFloatAttribute(config::name, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, bool>::value) {
      if (element->QueryBoolAttribute(config::name, &val) != tinyxml2::XML_SUCCESS) {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_same<T, hydra::Vector3>::value) {
      val = kraken::getXMLAttribute(config::name, element, config::defaultVal);
    } else if constexpr (std::is_same<T, hydra::AABB>::value) {
      val = kraken::getXMLAttribute(config::name, element, config::defaultVal);
    } else if constexpr (std::is_same<T, std::string>::value) {
      const char* name = element->Attribute(config::name);
      if (name) {
        val = name;
      } else {
        val = config::defaultVal;
      }
    } else if constexpr (std::is_base_of<KRResourceBinding, T>::value) {
        const char* name = element->Attribute(config::name);
        if (name) {
          val.set(name);
        } else {
          val.clear();
        }
    } else {
      static_assert(false, "Typename not implemented.");
    }
  }

  T val;
};

#define KRNODE_PROPERTY(PROP_TYPE, VAR, PROP_DEFAULT, PROP_NAME) \
   struct VAR ## _config { \
    static constexpr decltype(PROP_DEFAULT) defaultVal = PROP_DEFAULT; \
    static constexpr const char* name = PROP_NAME; \
  }; \
  KRNodeProperty<PROP_TYPE, VAR ## _config> VAR;

class KRNode
  : public KRContextObject
  , public KRReflectedObject
{
public:

  enum LodVisibility
  {
    LOD_VISIBILITY_HIDDEN,
    LOD_VISIBILITY_PRESTREAM,
    LOD_VISIBILITY_VISIBLE
  };

  class RenderInfo
  {
  public:
    RenderInfo(VkCommandBuffer& cb)
      : commandBuffer(cb)
    {

    }

    RenderInfo(const RenderInfo&) = delete;
    RenderInfo& operator=(const RenderInfo&) = delete;

    VkCommandBuffer& commandBuffer;
    KRCamera* camera;
    KRSurface* surface;
    std::vector<KRPointLight*> point_lights;
    std::vector<KRDirectionalLight*> directional_lights;
    std::vector<KRSpotLight*> spot_lights;
    KRViewport* viewport;
    KRRenderPass* renderPass;
    KRPipeline* pipeline;

    std::vector<const KRReflectedObject*> reflectedObjects;
  };

  static void InitNodeInfo(KrNodeInfo* nodeInfo);
  virtual KrResult update(const KrNodeInfo* nodeInfo);

  KRNode(KRScene& scene, std::string name);
  virtual ~KRNode();

  virtual tinyxml2::XMLElement* saveXML(tinyxml2::XMLNode* parent);

  static KRNode* LoadXML(KRScene& scene, tinyxml2::XMLElement* e);
  static KrResult createNode(const KrCreateNodeInfo* pCreateNodeInfo, KRScene* scene, KRNode** node);
  virtual void loadXML(tinyxml2::XMLElement* e);

  virtual std::string getElementName();
  const std::string& getName() const;

  void appendChild(KRNode* child);
  void prependChild(KRNode* child);
  void insertBefore(KRNode* child);
  void insertAfter(KRNode* child);
  KRNode* getParent();

  void setLocalTranslation(const hydra::Vector3& v, bool set_original = false);
  void setLocalScale(const hydra::Vector3& v, bool set_original = false);
  void setLocalRotation(const hydra::Vector3& v, bool set_original = false);


  void setRotationOffset(const hydra::Vector3& v, bool set_original = false);
  void setScalingOffset(const hydra::Vector3& v, bool set_original = false);
  void setRotationPivot(const hydra::Vector3& v, bool set_original = false);
  void setScalingPivot(const hydra::Vector3& v, bool set_original = false);
  void setPreRotation(const hydra::Vector3& v, bool set_original = false);
  void setPostRotation(const hydra::Vector3& v, bool set_original = false);

  const hydra::Vector3& getRotationOffset();
  const hydra::Vector3& getScalingOffset();
  const hydra::Vector3& getRotationPivot();
  const hydra::Vector3& getScalingPivot();
  const hydra::Vector3& getPreRotation();
  const hydra::Vector3& getPostRotation();

  const hydra::Vector3& getInitialRotationOffset();
  const hydra::Vector3& getInitialScalingOffset();
  const hydra::Vector3& getInitialRotationPivot();
  const hydra::Vector3& getInitialScalingPivot();
  const hydra::Vector3& getInitialPreRotation();
  const hydra::Vector3& getInitialPostRotation();


  const hydra::Vector3& getLocalTranslation();
  const hydra::Vector3& getLocalScale();
  const hydra::Vector3& getLocalRotation();

  const hydra::Vector3& getInitialLocalTranslation();
  const hydra::Vector3& getInitialLocalScale();
  const hydra::Vector3& getInitialLocalRotation();

  const hydra::Vector3 getWorldTranslation();
  const hydra::Vector3 getWorldScale();
  const hydra::Quaternion getWorldRotation();

  const hydra::Quaternion getBindPoseWorldRotation();
  const hydra::Quaternion getActivePoseWorldRotation();

  const hydra::Vector3 localToWorld(const hydra::Vector3& local_point);
  const hydra::Vector3 worldToLocal(const hydra::Vector3& world_point);

  void setWorldTranslation(const hydra::Vector3& v);
  void setWorldScale(const hydra::Vector3& v);
  void setWorldRotation(const hydra::Vector3& v);

  virtual hydra::AABB getBounds();
  void invalidateBounds() const;
  const hydra::Matrix4& getModelMatrix();
  const hydra::Matrix4& getInverseModelMatrix();
  const hydra::Matrix4& getBindPoseMatrix();
  const hydra::Matrix4& getActivePoseMatrix();
  const hydra::Matrix4& getInverseBindPoseMatrix();

  enum node_attribute_type
  {
    KRENGINE_NODE_ATTRIBUTE_NONE,
    KRENGINE_NODE_ATTRIBUTE_TRANSLATE_X,
    KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Y,
    KRENGINE_NODE_ATTRIBUTE_TRANSLATE_Z,
    KRENGINE_NODE_ATTRIBUTE_SCALE_X,
    KRENGINE_NODE_ATTRIBUTE_SCALE_Y,
    KRENGINE_NODE_ATTRIBUTE_SCALE_Z,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_X,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_Y,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_Z,
    KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_X,
    KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Y,
    KRENGINE_NODE_ATTRIBUTE_PRE_ROTATION_Z,
    KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_X,
    KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Y,
    KRENGINE_NODE_ATTRIBUTE_POST_ROTATION_Z,
    KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_X,
    KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Y,
    KRENGINE_NODE_ATTRIBUTE_ROTATION_PIVOT_Z,
    KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_X,
    KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Y,
    KRENGINE_NODE_ATTRIBUTE_SCALE_PIVOT_Z,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_X,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Y,
    KRENGINE_NODE_ATTRIBUTE_ROTATE_OFFSET_Z,
    KRENGINE_NODE_SCALE_OFFSET_X,
    KRENGINE_NODE_SCALE_OFFSET_Y,
    KRENGINE_NODE_SCALE_OFFSET_Z,
    KRENGINE_NODE_ATTRIBUTE_COUNT
  };

  void SetAttribute(node_attribute_type attrib, float v);

  KRScene& getScene();

  virtual void preStream(const KRViewport& viewport, std::list<KRResourceRequest>& resourceRequests);
  virtual void getResourceBindings(std::list<KRResourceBinding*>& bindings);
  virtual void render(RenderInfo& ri);

  virtual void physicsUpdate(float deltaTime);
  virtual bool hasPhysics();

  virtual void updateLODVisibility(const KRViewport& viewport);
  LodVisibility getLODVisibility();

  void setScaleCompensation(bool scale_compensation);
  bool getScaleCompensation();
  void setAnimationEnabled(node_attribute_type attrib, bool enable);
  bool getAnimationEnabled(node_attribute_type attrib) const;


  virtual kraken_stream_level getStreamLevel(const KRViewport& viewport);

  virtual void setLODVisibility(LodVisibility lod_visibility);

public:
  bool isFirstSibling() const;
  bool isLastSibling() const;
  KRNode* m_parentNode;
  KRNode* m_previousNode;
  KRNode* m_nextNode;
  KRNode* m_firstChildNode;
  KRNode* m_lastChildNode;

protected:
  hydra::Vector3 m_localTranslation;
  hydra::Vector3 m_localScale;
  hydra::Vector3 m_localRotation;

  hydra::Vector3 m_rotationOffset;
  hydra::Vector3 m_scalingOffset;
  hydra::Vector3 m_rotationPivot;
  hydra::Vector3 m_scalingPivot;
  hydra::Vector3 m_preRotation;
  hydra::Vector3 m_postRotation;

  hydra::Vector3 m_initialLocalTranslation;
  hydra::Vector3 m_initialLocalScale;
  hydra::Vector3 m_initialLocalRotation;

  hydra::Vector3 m_initialRotationOffset;
  hydra::Vector3 m_initialScalingOffset;
  hydra::Vector3 m_initialRotationPivot;
  hydra::Vector3 m_initialScalingPivot;
  hydra::Vector3 m_initialPreRotation;
  hydra::Vector3 m_initialPostRotation;

  LodVisibility m_lod_visible;

  bool m_animation_mask[KRENGINE_NODE_ATTRIBUTE_COUNT];

private:
  void makeOrphan();
  long m_lastRenderFrame;
  void invalidateModelMatrix();
  void invalidateBindPoseMatrix();
  hydra::Matrix4 m_modelMatrix;
  hydra::Matrix4 m_inverseModelMatrix;
  hydra::Matrix4 m_bindPoseMatrix;
  hydra::Matrix4 m_activePoseMatrix;
  hydra::Matrix4 m_inverseBindPoseMatrix;
  bool m_modelMatrixValid;
  bool m_inverseModelMatrixValid;
  bool m_bindPoseMatrixValid;
  bool m_activePoseMatrixValid;
  bool m_inverseBindPoseMatrixValid;

  mutable hydra::AABB m_bounds;
  mutable bool m_boundsValid;

  std::string m_name;



  KRScene* m_pScene;

  std::set<KROctreeNode*> m_octree_nodes;
  bool m_scale_compensation;

  std::set<KRBehavior*> m_behaviors;

public:
  void addBehavior(KRBehavior* behavior);
  std::set<KRBehavior*>& getBehaviors();
  template <class T> T* getBehavior()
  {
    for (std::set<KRBehavior*>::iterator itr = m_behaviors.begin(); itr != m_behaviors.end(); itr++) {
      T* behavior = dynamic_cast<T*>(*itr);
      if (behavior) {
        return behavior;
      }
    }
    return NULL;
  }
  void removeFromOctreeNodes();
  void addToOctreeNode(KROctreeNode* octree_node);
  void childRemoved(KRNode* child_node);

  template <class T> T* find()
  {
    T* match = dynamic_cast<T*>(this);
    if (match) {
      return match;
    }

    for (KRNode* child = m_firstChildNode; child != nullptr; child = child->m_nextNode) {
      match = child->find<T>();
      if (match) {
        return match;
      }
    }

    return NULL;
  }

  template <class T> T* find(const std::string& name)
  {
    // TODO - KRScene should maintain a global node-name map for rapid searching
    T* match = dynamic_cast<T*>(this);
    if (match) {
      if (name.compare(match->getName()) == 0) {
        return match;
      }
    }

    for (KRNode* child = m_firstChildNode; child != nullptr; child = child->m_nextNode) {
      match = child->find<T>(name);
      if (match) {
        return match;
      }
    }

    return NULL;
  }
};
