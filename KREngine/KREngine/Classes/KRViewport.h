//
//  KRViewport.h
//  KREngine
//
//  Created by Kearwood Gilbert on 2012-10-25.
//  Copyright (c) 2012 Kearwood Software. All rights reserved.
//

#ifndef KRENGINE_KRVIEWPORT_H
#define KRENGINE_KRVIEWPORT_H

#include "KRVector2.h"
#include "KRMat4.h"

class KRViewport {
public:
    KRViewport();
    KRViewport(const KRVector2 &size, const KRMat4 &matView, const KRMat4 &matProjection);
    ~KRViewport();
    
    const KRVector2 &getSize() const;
    const KRMat4 &getViewMatrix() const;
    const KRMat4 &getProjectionMatrix() const;
    void setSize(const KRVector2 &size);
    void setViewMatrix(const KRMat4 &matView);
    void setProjectionMatrix(const KRMat4 &matProjection);
    
    // Overload assignment operator
    KRViewport& operator=(const KRViewport &v);
    
private:
    KRVector2 m_size;
    KRMat4 m_matView;
    KRMat4 m_matProjection;
};

#endif
