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
    float a = 2.0f * M_PI * i / length;
    cos_table[i] = cos(a);
    sin_table[i] = sin(a);
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
  // Radix-2 Decimation in Time FFT Algorithm
  // http://en.dsplib.org/content/fft_dec_in_time.html

  // Only power-of-two sizes supported
  assert((count & (count - 1)) == 0);

  int levels = 0;
  while (1 << levels <= count) {
    levels++;
  }

  for (size_t i = 0; i < count; i++) {
    size_t j = 0;
    for (int k = 0; k < levels; k++) {
      j <<= 1;
      j |= ((i >> k) & 1);
    }
    if (j > i) {
      float temp = src->realp[i];
      src->realp[i] = src->realp[j];
      src->realp[j] = temp;
      temp = src->imagp[i];
      src->imagp[i] = src->imagp[j];
      src->imagp[j] = temp;
    }
  }

  for (size_t size = 2; size <= count; size *= 2) {
    size_t halfsize = size / 2;
    size_t step = count / size;
    for (size_t i = 0; i < count; i += size) {
      for (size_t j = i, k = 0; j < i + halfsize; j++, k += step) {
        float temp_real = src->realp[j + halfsize] * workspace.cos_table[k];
        temp_real += src->imagp[j + halfsize] * workspace.sin_table[k];
        float temp_imag = -src->realp[j + halfsize] * workspace.sin_table[k];
        temp_imag += src->imagp[j + halfsize] * workspace.cos_table[k];
        src->realp[j + halfsize] = src->realp[j] - temp_real;
        src->imagp[j + halfsize] = src->imagp[j] - temp_imag;
        src->realp[j] += temp_real;
        src->imagp[j] += temp_imag;
      }
    }
  }
}

void FFTInverse(const FFTWorkspace &workspace, SplitComplex *src, size_t count)
{
  SplitComplex swapped;
  swapped.imagp = src->realp;
  swapped.realp = src->imagp;
  FFTForward(workspace, &swapped, count);
}

void Int16ToFloat(const short *src, size_t srcStride, float *dest, size_t destStride, size_t count)
{
  const short *r = src;
  float *w = dest;
  while (w < dest + destStride * count) {
    *w = (float)*r;
    r += srcStride;
    w += destStride;
  }
}

void Scale(float *buffer, float scale, size_t count)
{
  float *w = buffer;
  while (w < buffer + count) {
    *w *= scale;
    w++;
  }
}

void ScaleCopy(const float *src, float scale, float *dest, size_t count)
{
  const float *r = src;
  float *w = dest;
  while (w < dest + count) {
    *w = *r * scale;
    w++;
    r++;
  }
}

void ScaleCopy(const SplitComplex *src, float scale, SplitComplex *dest, size_t count)
{
  ScaleCopy(src->realp, scale, dest->realp, count);
  ScaleCopy(src->imagp, scale, dest->imagp, count);
}

void ScaleRamp(float *buffer, float scaleStart, float scaleStep, size_t count)
{
  float *w = buffer;
  float s = scaleStart;
  while (w < buffer + count) {
    *w *= s;
    w++;
    s += scaleStep;
  }
}

void Accumulate(float *buffer, size_t bufferStride, const float *buffer2, size_t buffer2Stride, size_t count)
{
  float *w = buffer;
  const float *r = buffer2;
  while (w < buffer + bufferStride * count) {
    *w *= *r;
    w += bufferStride;
    r += buffer2Stride;
  }
}

void Accumulate(SplitComplex *buffer, const SplitComplex *buffer2, size_t count)
{
  for (size_t i = 0; i < count; i++) {
    buffer->imagp[i] += buffer2->imagp[i];
    buffer->realp[i] += buffer2->realp[i];
  }
}

void Multiply(const SplitComplex *a, const SplitComplex *b, SplitComplex *c, size_t count)
{
  for (size_t i = 0; i < count; i++) {
    c->realp[i] = a->realp[i] * b->realp[i] - a->imagp[i] * b->imagp[i];
    c->imagp[i] = a->realp[i] * b->imagp[i] + a->imagp[i] * b->realp[i];
  }
}

} // namespace KRDSP

#endif // KRDSP_SLOW
