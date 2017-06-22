//
//  KREngine.h
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

#ifndef _KRDSP_H
#define _KRDSP_H

#include "KREngine-common.h"

namespace KRDSP {

#ifdef __APPLE__
#define KRDSP_APPLE_VDSP
#include <Accelerate/Accelerate.h>
#else
  // Slow, but portable fallback implementation
#define KRDSP_SLOW
#endif

#if defined(KRDSP_APPLE_VDSP)

  // Apple vDSP
  typedef DSPSplitComplex SplitComplex;
  struct FFTWorkspace {
    FFTSetup setup;

    void create(size_t length);
    void destroy();
    FFTWorkspace();
    ~FFTWorkspace();
  };

#elif defined(KRDSP_SLOW)

  typedef struct {
    float *realp;
    float *imagp;
  } SplitComplex;

  struct FFTWorkspace {
    float *sin_table;
    float *cos_table;

    void create(size_t length);
    void destroy();
    FFTWorkspace();
    ~FFTWorkspace();
  };

#else
#error Not Implemented
#endif

void FFTForward(const FFTWorkspace &workspace, SplitComplex *src, size_t count);
void FFTInverse(const FFTWorkspace &workspace, SplitComplex *src, size_t count);
void Int16ToFloat(const short *src, size_t srcStride, float *dest, size_t destStride, size_t count);
void Scale(float *buffer, float scale, size_t count);
void ScaleCopy(const float *src, float scale, float *dest, size_t count);
void ScaleCopy(const SplitComplex *src, float scale, SplitComplex *dest, size_t count);
void ScaleRamp(float *buffer, float scaleStart, float scaleStep, size_t count);
void Accumulate(float *buffer, size_t bufferStride, const float *buffer2, size_t buffer2Stride, size_t count);
void Accumulate(SplitComplex *buffer, const SplitComplex *buffer2, size_t count);
void Multiply(const SplitComplex *a, const SplitComplex *b, SplitComplex *c, size_t count);

} // namespace KRDSP

#endif // _KRDSP_H
