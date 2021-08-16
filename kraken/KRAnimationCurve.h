//
//  KRAnimationCurve.h
//  Kraken Engine
//
//  Copyright 2021 Kearwood Gilbert. All rights reserved.
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

#ifndef KRANIMATIONCURVE_H
#define KRANIMATIONCURVE_H

#include "KREngine-common.h"
#include "KRContextObject.h"
#include "KRDataBlock.h"
#include "KRResource.h"

class KRAnimationCurve : public KRResource {
    
public:
    KRAnimationCurve(KRContext &context, const std::string &name);
    virtual ~KRAnimationCurve();
    
    virtual std::string getExtension();
    virtual bool save(const std::string& path);
    virtual bool save(KRDataBlock &data);
    virtual bool load(KRDataBlock *data);
    
    float getFrameRate();
    void setFrameRate(float frame_rate);
    int getFrameStart();
    void setFrameStart(int frame_number);
    int getFrameCount();
    void setFrameCount(int frame_count);
    float getValue(float local_time);
    float getValue(int frame_number);
    void setValue(int frame_number, float value);
    
    
    static KRAnimationCurve *Load(KRContext &context, const std::string &name, KRDataBlock *data);
    
    bool valueChanges(float start_time, float duration);
    bool valueChanges(int start_frame, int frame_count);
    
    KRAnimationCurve *split(const std::string &name, float start_time, float duration);
    KRAnimationCurve *split(const std::string &name, int start_frame, int frame_count);
    
    void _lockData();
    void _unlockData();
    
private:
    KRDataBlock *m_pData;
    
    typedef struct {
        char szTag[16];
        float frame_rate;
        int32_t frame_start;
        int32_t frame_count;
    } animation_curve_header;

};


#endif
