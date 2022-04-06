//
//  KRNode.h
//  Kraken Engine
//
//  Copyright 2022 Kearwood Gilbert. All rights reserved.
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

#ifndef KRNODE_H
#define KRNODE_H

#include "KRResource.h"
#include "KRViewport.h"
#include "KROctreeNode.h"
#include "KRBehavior.h"

using namespace kraken;

namespace kraken {
class Matrix4;
class AABB;
} // namespace kraken
class KRCamera;
class KRPipelineManager;
class KRMeshManager;
class KRMaterialManager;
class KRTextureManager;
class KRContext;
class KRScene;
class KRSurface;

class KRNode;
class KRPointLight;
class KRSpotLight;
class KRDirectionalLight;
namespace tinyxml2 {
  class XMLNode;
  class XMLAttribute;
}

class KRNode : public KRContextObject
{
public:
    enum RenderPass {
        RENDER_PASS_FORWARD_OPAQUE,
        RENDER_PASS_DEFERRED_GBUFFER,
        RENDER_PASS_DEFERRED_LIGHTS,
        RENDER_PASS_DEFERRED_OPAQUE,
        RENDER_PASS_FORWARD_TRANSPARENT,
        RENDER_PASS_PARTICLE_OCCLUSION,
        RENDER_PASS_ADDITIVE_PARTICLES,
        RENDER_PASS_VOLUMETRIC_EFFECTS_ADDITIVE,
        RENDER_PASS_GENERATE_SHADOWMAPS,
        RENDER_PASS_SHADOWMAP,
        RENDER_PASS_PRESTREAM
    };
    
    enum LodVisibility {
        LOD_VISIBILITY_HIDDEN,
        LOD_VISIBILITY_PRESTREAM,
        LOD_VISIBILITY_VISIBLE
    };

    class RenderInfo {
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
      KRViewport viewport;
      RenderPass renderPass;
    };

    static void InitNodeInfo(KrNodeInfo* nodeInfo);
    
    KRNode(KRScene &scene, std::string name);
    virtual ~KRNode();
    
    virtual tinyxml2::XMLElement *saveXML( tinyxml2::XMLNode *parent);

    static KRNode *LoadXML(KRScene &scene, tinyxml2::XMLElement *e);
    virtual void loadXML(tinyxml2::XMLElement *e);
    
    virtual std::string getElementName();
    const std::string &getName() const;
    
    void addChild(KRNode *child);
    const std::set<KRNode *> &getChildren();
    KRNode *getParent();
    
    void setLocalTranslation(const Vector3 &v, bool set_original = false);
    void setLocalScale(const Vector3 &v, bool set_original = false);
    void setLocalRotation(const Vector3 &v, bool set_original = false);
    
    
    void setRotationOffset(const Vector3 &v, bool set_original = false);
    void setScalingOffset(const Vector3 &v, bool set_original = false);
    void setRotationPivot(const Vector3 &v, bool set_original = false);
    void setScalingPivot(const Vector3 &v, bool set_original = false);
    void setPreRotation(const Vector3 &v, bool set_original = false);
    void setPostRotation(const Vector3 &v, bool set_original = false);
    
    const Vector3 &getRotationOffset();
    const Vector3 &getScalingOffset();
    const Vector3 &getRotationPivot();
    const Vector3 &getScalingPivot();
    const Vector3 &getPreRotation();
    const Vector3 &getPostRotation();
    
    const Vector3 &getInitialRotationOffset();
    const Vector3 &getInitialScalingOffset();
    const Vector3 &getInitialRotationPivot();
    const Vector3 &getInitialScalingPivot();
    const Vector3 &getInitialPreRotation();
    const Vector3 &getInitialPostRotation();
    
    
    const Vector3 &getLocalTranslation();
    const Vector3 &getLocalScale();
    const Vector3 &getLocalRotation();
    
    const Vector3 &getInitialLocalTranslation();
    const Vector3 &getInitialLocalScale();
    const Vector3 &getInitialLocalRotation();
    
    const Vector3 getWorldTranslation();
    const Vector3 getWorldScale();
    const Quaternion getWorldRotation();
    
    const Quaternion getBindPoseWorldRotation();
    const Quaternion getActivePoseWorldRotation();
    
    const Vector3 localToWorld(const Vector3 &local_point);
    const Vector3 worldToLocal(const Vector3 &world_point);
    
    void setWorldTranslation(const Vector3 &v);
    void setWorldScale(const Vector3 &v);
    void setWorldRotation(const Vector3 &v);
    
    virtual AABB getBounds();
    void invalidateBounds() const;
    const Matrix4 &getModelMatrix();
    const Matrix4 &getInverseModelMatrix();
    const Matrix4 &getBindPoseMatrix();
    const Matrix4 &getActivePoseMatrix();
    const Matrix4 &getInverseBindPoseMatrix();
    
    enum node_attribute_type {
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
    
    KRScene &getScene();
    
    virtual void render(const RenderInfo& ri);
    
    virtual void physicsUpdate(float deltaTime);
    virtual bool hasPhysics();
    
    virtual void updateLODVisibility(const KRViewport &viewport);
    LodVisibility getLODVisibility();
    
    void setScaleCompensation(bool scale_compensation);
    bool getScaleCompensation();
    void setAnimationEnabled(node_attribute_type attrib, bool enable);
    bool getAnimationEnabled(node_attribute_type attrib) const;
    
    
    virtual kraken_stream_level getStreamLevel(const KRViewport &viewport);
    
    virtual void setLODVisibility(LodVisibility lod_visibility);
    
protected:
    Vector3 m_localTranslation;
    Vector3 m_localScale;
    Vector3 m_localRotation;
    
    Vector3 m_rotationOffset;
    Vector3 m_scalingOffset;
    Vector3 m_rotationPivot;
    Vector3 m_scalingPivot;
    Vector3 m_preRotation;
    Vector3 m_postRotation;
    
    Vector3 m_initialLocalTranslation;
    Vector3 m_initialLocalScale;
    Vector3 m_initialLocalRotation;
    
    Vector3 m_initialRotationOffset;
    Vector3 m_initialScalingOffset;
    Vector3 m_initialRotationPivot;
    Vector3 m_initialScalingPivot;
    Vector3 m_initialPreRotation;
    Vector3 m_initialPostRotation;
    
    LodVisibility m_lod_visible;
    
    KRNode *m_parentNode;
    std::set<KRNode *> m_childNodes;
    
    bool m_animation_mask[KRENGINE_NODE_ATTRIBUTE_COUNT];
    
private:
    long m_lastRenderFrame;
    void invalidateModelMatrix();
    void invalidateBindPoseMatrix();
    Matrix4 m_modelMatrix;
    Matrix4 m_inverseModelMatrix;
    Matrix4 m_bindPoseMatrix;
    Matrix4 m_activePoseMatrix;
    Matrix4 m_inverseBindPoseMatrix;
    bool m_modelMatrixValid;
    bool m_inverseModelMatrixValid;
    bool m_bindPoseMatrixValid;
    bool m_activePoseMatrixValid;
    bool m_inverseBindPoseMatrixValid;
    
    mutable AABB m_bounds;
    mutable bool m_boundsValid;
    
    std::string m_name;
    
    
    
    KRScene *m_pScene;
    
    std::set<KROctreeNode *> m_octree_nodes;
    bool m_scale_compensation;
    
    std::set<KRBehavior *> m_behaviors;
    
public:
    void addBehavior(KRBehavior *behavior);
    std::set<KRBehavior *> &getBehaviors();
    template <class T> T *getBehavior()
    {
        for(std::set<KRBehavior *>::iterator itr=m_behaviors.begin(); itr != m_behaviors.end(); itr++) {
            T *behavior = dynamic_cast<T *>(*itr);
            if(behavior) {
                return behavior;
            }
        }
        return NULL;
    }
    void removeFromOctreeNodes();
    void addToOctreeNode(KROctreeNode *octree_node);
    void childDeleted(KRNode *child_node);
    
    template <class T> T *find()
    {
        T *match = dynamic_cast<T *>(this);
        if(match) {
            return match;
        }
        
        for(std::set<KRNode *>::const_iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            match = (*itr)->find<T>();
            if(match) {
                return match;
            }
        }
        
        return NULL;
    }
    
    template <class T> T *find(const std::string &name)
    {
        T *match = dynamic_cast<T *>(this);
        if(match) {
            if(name.compare(match->getName()) == 0) {
                return match;
            }
        }
        
        for(std::set<KRNode *>::const_iterator itr=m_childNodes.begin(); itr != m_childNodes.end(); ++itr) {
            match = (*itr)->find<T>(name);
            if(match) {
                return match;
            }
        }
        
        return NULL;
    }
};


#endif
