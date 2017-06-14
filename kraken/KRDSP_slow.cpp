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

#include "KRDSP.h"

#ifdef KRDSP_SLOW

#include "KREngine-common.h"

namespace KRDSP {

FFTWorkspace::FFTWorkspace()
{
  sin_table = nullptr;
  cos_table = nullptr;
}

FFTWorkspace::~FFTWorkspace()
{
  destroy();
}

void FFTWorkspace::create(size_t length)
{
  size_t size = (length / 2);
  cos_table = new float[size];
  sin_table = new float[size];
  for (int i = 0; i < size / 2; i++) {
    cos_table[i] = cos(2 * M_PI * i / length);
    sin_table[i] = sin(2 * M_PI * i / length);
  }
}

void FFTWorkspace::destroy()
{
  if (sin_table) {
    delete sin_table;
    sin_table = nullptr;
  }
  if (cos_table) {
    delete cos_table;
    cos_table = nullptr;
  }
}

void FFTForward(const FFTWorkspace &workspace, SplitComplex *src, size_t count)
{
#error TODO - Implement
}

void FFTInverse(const FFTWorkspace &workspace, SplitComplex *src, size_t count)
{
#error TODO - Implement
}

void Int16ToFloat(const short *src, size_t srcStride, float *dest, size_t destStride, size_t count)
{
#error TODO - Implement
}

void Scale(float *buffer, float scale, size_t count)
{
#error TODO - Implement
}

void ScaleCopy(const float *src, float scale, float *dest, size_t count)
{
#error TODO - Implement
}

void ScaleCopy(const SplitComplex *src, float scale, SplitComplex *dest, size_t count)
{
  ScaleCopy(src->realp, scale, dest->realp, count);
  ScaleCopy(src->imagp, scale, dest->imagp, count);
}

void ScaleRamp(float *buffer, float scaleStart, float scaleStep, size_t count)
{
#error TODO - Implement
}

void Accumulate(float *buffer, size_t bufferStride, const float *buffer2, size_t buffer2Stride, size_t count)
{
#error TODO - Implement
}

void Accumulate(SplitComplex *buffer, const SplitComplex *buffer2, size_t count)
{
#error TODO - Implement
}


void Multiply(const SplitComplex *a, const SplitComplex *b, SplitComplex *c, size_t count)
{
#error TODO - Implement
}

} // namespace KRDSP

#endif // KRDSP_SLOW
