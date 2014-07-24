//
//  Image.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#include "Image.h"
#include "Platform.h"

namespace cinder { namespace cl {
	
ImageBase::ImageBase()
: MemoryObj( Context::context() )
{
	
}
	
Image2dRef Image2d::create( void *data, cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch )
{
	return Image2dRef( new Image2d(  data, flags, image_format, width, height, row_pitch ) );
}


Image2d::Image2d( void *data, cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch )
	: mWidth( width ), mHeight( height )
{
	cl_int errNum;
	
	mChannelOrder = image_format->image_channel_order;
	mChannelType = image_format->image_channel_data_type;
	
	mFlags = flags;
	
	mId = clCreateImage2D( mContext->getId(), mFlags, image_format, mWidth, mHeight, row_pitch, data, &errNum );
	
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << Platform::getClErrorString(errNum) << std::endl;
	}
}
	
Image2d::Image2d( const gl::Texture2dRef &texture, cl_mem_flags flags, GLint mipLevelForUse )
{
	cl_int errNum;
	
	mFlags = flags;
	mWidth = texture->getWidth();
	mHeight = texture->getHeight();
	
	mId = clCreateFromGLTexture2D( mContext->getId(), flags, texture->getTarget(), mipLevelForUse, texture->getId(), &errNum );
	if( errNum != CL_SUCCESS ) {
		std::cout << "ERROR: " << __FUNCTION__ << " " << Platform::getClErrorString(errNum) << std::endl;
	}
}

	
}}