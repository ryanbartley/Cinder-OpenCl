/*
 Copyright (c) 2016, The Cinder Project, All rights reserved.
 
 This code is intended for use with the Cinder C++ library: http://libcinder.org
 
 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"
#include "cinder/ImageIo.h"

namespace cinder {
namespace gl {
using Texture2dRef = std::shared_ptr<class Texture2d>;
}
}

namespace cinder { namespace ocl {
	
using namespace cl;
using ImageFormats = std::vector<ImageFormat>;

cl_context_properties* getDefaultSharedGraphicsContextProperties( const cl::Platform &platform = cl::Platform() );
	
cl_float2	toCl( const vec2 &val );
cl_float3	toCl( const vec3 &val );
cl_float4	toCl( const vec3 &val );
cl_float4	toCl( const vec4 &val );
cl_float16	toCl( const mat4 &val );
cl_int2		toCl( const ivec2 &val );
cl_int3		toCl( const ivec3 &val );
cl_int4		toCl( const ivec4 &val );
cl_uint2	toCl( const uvec2 &val );
cl_uint3	toCl( const uvec3 &val );
cl_uint4	toCl( const uvec4 &val );

std::string			errorToString( cl_int error );
std::string			constantToString( cl_int constant );
Program				createProgram( Context &context, const DataSourceRef &dataSource, bool build = true );
	
ImageFormats&		getSupportedImageFormats( cl::Context context, cl_mem_object_type type );
void				printSupportedImageFormats( const cl::Context &context, cl_mem_object_type type );
void				convertChannelOrder( cl_channel_order channel,
										 ImageIo::ColorModel *colorModel,
										 ImageIo::ChannelOrder *channelOrder );
void				convertChannelDataType( cl_channel_type type, ImageIo::DataType *dataType );
cl::ImageFormat		getImageFormat( ImageIo::ChannelOrder channelOrder, ImageIo::DataType dataType );
Image2D				createImage2D( const ImageSourceRef &imageSource,
								   cl::Context context,
								   cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR );
cl::ImageGL			createImageGL( const gl::Texture2dRef &texture, cl::Context context, cl_mem_flags flags );
cl::Buffer			createBuffer( const ImageSourceRef &imageSource,
								  cl::Context context,
								  cl_mem_flags flags = CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR );
ImageSourceRef		createSource( Image2D image );
	
} // namespace ocl	
	
std::ostream& operator<<( std::ostream &lhs, const ocl::Platform &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Device &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Program &rhs );
std::ostream& operator<<( std::ostream &lhs, const ocl::Kernel &rhs );
	
namespace ocl {
	
namespace detail {
	
template<typename CL_TYPE, typename GLM_TYPE>
inline CL_TYPE toCl( const GLM_TYPE &val )
{
	CL_TYPE clRet;
	memcpy( &clRet, &val, sizeof( GLM_TYPE ) );
	return clRet;
}

template<typename GLM_TYPE, typename CL_TYPE>
inline GLM_TYPE toGl( const CL_TYPE &val )
{
	GLM_TYPE glRet;
	memcpy( &glRet, &val, sizeof( GLM_TYPE ) );
	return glRet;
}
	
} // namespace detail
	
inline cl_float2	toCl( const vec2 &val ) { return detail::toCl<cl_float2>( val ); }
inline cl_float3	toCl( const vec3 &val ) { return detail::toCl<cl_float3>( val ); }
inline cl_float4	toCl( const vec4 &val ) { return detail::toCl<cl_float4>( val ); }
inline cl_float16	toCl( const mat4 &val ) { return detail::toCl<cl_float16>( val ); }
inline cl_int2		toCl( const ivec2 &val ) { return detail::toCl<cl_int2>( val ); }
inline cl_int3		toCl( const ivec3 &val ) { return detail::toCl<cl_int3>( val ); }
inline cl_int4		toCl( const ivec4 &val ) { return detail::toCl<cl_int4>( val ); }
inline cl_uint2		toCl( const uvec2 &val ) { return detail::toCl<cl_uint2>( val ); }
inline cl_uint3		toCl( const uvec3 &val ) { return detail::toCl<cl_uint3>( val ); }
inline cl_uint4		toCl( const uvec4 &val ) { return detail::toCl<cl_uint4>( val ); }
	
}
	
} // namespace cinder


