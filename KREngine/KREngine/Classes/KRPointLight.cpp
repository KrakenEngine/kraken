//
//  KRPointLight.cpp
//  KREngine
//
//  Created by Kearwood Gilbert on 12-04-05.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#include <iostream>
#include <math.h>

#import "KRPointLight.h"
#import "KRMat4.h"
#import "KRVector3.h"
#import "KRCamera.h"
#import "KRContext.h"
#import "KRBoundingVolume.h"

KRPointLight::KRPointLight(KRScene &scene, std::string name) : KRLight(scene, name)
{
    m_sphereVertices = NULL;
    m_cVertices = 0;
}

KRPointLight::~KRPointLight()
{
    if(m_sphereVertices) {
        delete m_sphereVertices;
        m_cVertices = 0;
    }
}

std::string KRPointLight::getElementName() {
    return "point_light";
}

#if TARGET_OS_IPHONE

void KRPointLight::render(KRCamera *pCamera, KRContext *pContext, KRBoundingVolume &frustrumVolume, KRMat4 &viewMatrix, KRVector3 &cameraPosition, KRVector3 &lightDirection, KRMat4 *pShadowMatrices, GLuint *shadowDepthTextures, int cShadowBuffers, KRNode::RenderPass renderPass) {
    
    KRLight::render(pCamera, pContext, frustrumVolume, viewMatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, cShadowBuffers, renderPass);

    
    if(renderPass == KRNode::RENDER_PASS_DEFERRED_LIGHTS) {
        // Lights are rendered on the second pass of the deferred renderer

        KRMat4 projectionMatrix = pCamera->getProjectionMatrix();
        KRVector3 light_position = getLocalTranslation();
        
        float influence_radius = sqrt((m_intensity / 100.0) / KRLIGHT_MIN_INFLUENCE - 1.0) + m_decayStart;
        
        m_modelMatrix = KRMat4();
        m_modelMatrix.scale(influence_radius);
        m_modelMatrix.translate(light_position.x, light_position.y, light_position.z);
        
        
        KRMat4 mvpmatrix = m_modelMatrix * viewMatrix * projectionMatrix;
        KRMat4 matModelToView = viewMatrix * m_modelMatrix;
        matModelToView.transpose();
        matModelToView.invert();

        
        KRMat4 matModelToView2 = KRMat4() * m_modelMatrix * viewMatrix;
        
        KRMat4 matViewToModel = m_modelMatrix * viewMatrix;
        matViewToModel.invert();
        
        KRVector3 view_space_light_position = KRMat4::Dot(matModelToView2, KRVector3::Zero()); // Origin point of model space is the light source position.  No perspective, so no w divide required
        
        KRBoundingVolume influence_extents = KRBoundingVolume(KRVector3(-1.0), KRVector3(1.0), m_modelMatrix);
        
        KRBoundingVolume frustrumVolumeNoNearClip = KRBoundingVolume(viewMatrix, pCamera->perspective_fov,  pCamera->m_viewportSize.x / pCamera->m_viewportSize.y, 0.0, pCamera->perspective_farz);
        
        if(influence_extents.test_intersect(frustrumVolumeNoNearClip)) {
            // Cull out any lights not within the view frustrum
            
            KRShader *pShader = pContext->getShaderManager()->getShader("light_point", pCamera, false, false, false, 0, false, false, false, false, false, false, false, false, false, renderPass);
            pShader->bind(pCamera, matModelToView, mvpmatrix, cameraPosition, lightDirection, pShadowMatrices, shadowDepthTextures, 0, renderPass);
            glUniform3f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_COLOR],
                        m_color.x,
                        m_color.y,
                        m_color.z
            );
            
            glUniform1f(
                    pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_INTENSITY],
                    m_intensity / 100.0f
            );
            
            glUniform1f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_DECAY_START],
                        getDecayStart()
            );
            
            glUniform1f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_CUTOFF],
                        KRLIGHT_MIN_INFLUENCE
            );
            
            
            glUniform3f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_POSITION],
                        light_position.x,
                        light_position.y,
                        light_position.z
            );
            
            glUniform3f(
                        pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_LIGHT_POSITION_VIEW_SPACE],
                        view_space_light_position.x,
                        view_space_light_position.y,
                        view_space_light_position.z
            );
            
            glUniformMatrix4fv(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_V2M], 1, GL_FALSE, matViewToModel.getPointer());
            glUniformMatrix4fv(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_M2V], 1, GL_FALSE, matModelToView2.getPointer());
            
            
            KRMat4 matInvProjection;
            matInvProjection = pCamera->getProjectionMatrix();
            matInvProjection.invert();
            glUniformMatrix4fv(pShader->m_uniforms[KRShader::KRENGINE_UNIFORM_INVP], 1, GL_FALSE, matInvProjection.getPointer());
            
            
            // Disable z-buffer write
            glDepthMask(GL_FALSE);
            
            // Render sphere of light's influence
            generateMesh();
        
            // Enable z-buffer test
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            glDepthRangef(0.0, 1.0);
            
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(KRShader::KRENGINE_ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, m_sphereVertices);
            glEnableVertexAttribArray(KRShader::KRENGINE_ATTRIB_VERTEX);
            glDrawArrays(GL_TRIANGLES, 0, m_cVertices);
        }
    }
}

void KRPointLight::generateMesh() {
    // Create a triangular facet approximation to a sphere
    // Based on algorithm from Paul Bourke: http://paulbourke.net/miscellaneous/sphere_cylinder/
    
    int iterations = 3;
    int facet_count = pow(4, iterations) * 8;
    
    if(m_cVertices != facet_count * 3) {
        if(m_sphereVertices) {
            free(m_sphereVertices);
            m_sphereVertices = NULL;
        }
        
        m_cVertices = facet_count * 3;

        
        class Facet3 {
        public:
            Facet3() {
                
            }
            ~Facet3() {
                
            }
            KRVector3 p1;
            KRVector3 p2;
            KRVector3 p3;
        };
        
        std::vector<Facet3> f = std::vector<Facet3>(facet_count);
        
        int i,it;
        double a;
        KRVector3 p[6] = {
            KRVector3(0,0,1),
            KRVector3(0,0,-1),
            KRVector3(-1,-1,0),
            KRVector3(1,-1,0),
            KRVector3(1,1,0),
            KRVector3(-1,1,0)
        };

        KRVector3 pa,pb,pc;
        int nt = 0,ntold;
        
        /* Create the level 0 object */
        a = 1 / sqrt(2.0);
        for (i=0;i<6;i++) {
            p[i].x *= a;
            p[i].y *= a;
        }
        f[0].p1 = p[0]; f[0].p2 = p[3]; f[0].p3 = p[4];
        f[1].p1 = p[0]; f[1].p2 = p[4]; f[1].p3 = p[5];
        f[2].p1 = p[0]; f[2].p2 = p[5]; f[2].p3 = p[2];
        f[3].p1 = p[0]; f[3].p2 = p[2]; f[3].p3 = p[3];
        f[4].p1 = p[1]; f[4].p2 = p[4]; f[4].p3 = p[3];
        f[5].p1 = p[1]; f[5].p2 = p[5]; f[5].p3 = p[4];
        f[6].p1 = p[1]; f[6].p2 = p[2]; f[6].p3 = p[5];
        f[7].p1 = p[1]; f[7].p2 = p[3]; f[7].p3 = p[2];
        nt = 8;
        
        /* Bisect each edge and move to the surface of a unit sphere */
        for (it=0;it<iterations;it++) {
            ntold = nt;
            for (i=0;i<ntold;i++) {
                pa.x = (f[i].p1.x + f[i].p2.x) / 2;
                pa.y = (f[i].p1.y + f[i].p2.y) / 2;
                pa.z = (f[i].p1.z + f[i].p2.z) / 2;
                pb.x = (f[i].p2.x + f[i].p3.x) / 2;
                pb.y = (f[i].p2.y + f[i].p3.y) / 2;
                pb.z = (f[i].p2.z + f[i].p3.z) / 2;
                pc.x = (f[i].p3.x + f[i].p1.x) / 2;
                pc.y = (f[i].p3.y + f[i].p1.y) / 2;
                pc.z = (f[i].p3.z + f[i].p1.z) / 2;
                pa.normalize();
                pb.normalize();
                pc.normalize();
                f[nt].p1 = f[i].p1; f[nt].p2 = pa; f[nt].p3 = pc; nt++;
                f[nt].p1 = pa; f[nt].p2 = f[i].p2; f[nt].p3 = pb; nt++;
                f[nt].p1 = pb; f[nt].p2 = f[i].p3; f[nt].p3 = pc; nt++;
                f[i].p1 = pa;
                f[i].p2 = pb;
                f[i].p3 = pc;
            }
        }
        
        m_sphereVertices = (GLfloat *)malloc(sizeof(GLfloat) * m_cVertices * 3);
        GLfloat *pDest = m_sphereVertices;
        for(int facet_index=0; facet_index < facet_count; facet_index++) {
            *pDest++ = f[facet_index].p1.x;
            *pDest++ = f[facet_index].p1.y;
            *pDest++ = f[facet_index].p1.z;
            *pDest++ = f[facet_index].p2.x;
            *pDest++ = f[facet_index].p2.y;
            *pDest++ = f[facet_index].p2.z;
            *pDest++ = f[facet_index].p3.x;
            *pDest++ = f[facet_index].p3.y;
            *pDest++ = f[facet_index].p3.z;
        }
    }
}

#endif