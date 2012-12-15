//
//  KRHitInfo.cpp
//  KREngine
//
//  Copyright 2012 Kearwood Gilbert. All rights reserved.
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

#include "KRHitInfo.h"

KRHitInfo::KRHitInfo()
{
    m_position = KRVector3::Zero();
    m_normal = KRVector3::Zero();
    m_node = NULL;
}

KRHitInfo::KRHitInfo(const KRVector3 &position, const KRVector3 &normal, KRNode *node)
{
    m_position = position;
    m_normal = normal;
    m_node = node;
}

KRHitInfo::KRHitInfo(const KRVector3 &position, const KRVector3 &normal)
{
    m_position = position;
    m_normal = normal;
    m_node = NULL;
}

KRHitInfo::~KRHitInfo()
{
    
}

bool KRHitInfo::didHit() const
{
    return m_normal == KRVector3::Zero();
}

KRVector3 KRHitInfo::getPosition() const
{
    return m_position;
}

KRVector3 KRHitInfo::getNormal() const
{
    return m_normal;
}

KRNode *KRHitInfo::getNode() const
{
    return m_node;
}

KRHitInfo& KRHitInfo::operator =(const KRHitInfo& b)
{
    m_position = b.m_position;
    m_normal = b.m_normal;
    m_node = b.m_node;
    return *this;
}
