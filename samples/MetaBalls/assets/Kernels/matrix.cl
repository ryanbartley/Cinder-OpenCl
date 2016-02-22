/*All source code is licensed under BSD
 
 Copyright (c) 2010, Daniel Holden All rights reserved.
 
 Corange is licensed under a basic BSD license.
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
 Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

typedef struct {
  float4 r0;
  float4 r1;
  float4 r2;
  float4 r3;
} mat4;

mat4 mat4_new(float xx, float xy, float xz, float xw,
              float yx, float yy, float yz, float yw,
              float zx, float zy, float zz, float zw,
              float wx, float wy, float wz, float ww) {
  mat4 m;
  m.r0 = (float4)(xx, xy, xz, xw);
  m.r1 = (float4)(yx, yy, yz, zw);
  m.r2 = (float4)(zx, zy, zz, zw);
  m.r3 = (float4)(wx, wy, wz, ww);
  return m;
}

mat4 mat4_id() {
  return mat4_new(1,0,0,0,
                  0,1,0,0,
                  0,0,1,0,
                  0,0,0,1);
}

float4 mat4_mul_f4(mat4 m, float4 v) {
  float4 ret;
  ret.x = dot(v, m.r0);
  ret.y = dot(v, m.r1);
  ret.z = dot(v, m.r2);
  ret.w = dot(v, m.r3);
  return ret;
}

float3 mat4_mul_f3(mat4 m, float3 v) {
  float4 v2 = (float4)(v, 1);
  
  float4 ret;
  ret.x = dot(v2, m.r0);
  ret.y = dot(v2, m.r1);
  ret.z = dot(v2, m.r2);
  ret.w = dot(v2, m.r3);
  
  return ret.xyz / ret.w;
}