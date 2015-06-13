//
//  Image.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/18/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "MemoryObj.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"

namespace cl {

// need to make this an abstract class to forgoe needing a virtual destructor
class ImageBase : public MemoryObj {
public:
	
	struct Format {
		
	};
	
	virtual ~ImageBase();
	
	cl_channel_order getChannelOrder() const { return mChannelOrder; }
	cl_channel_type  getChannelDataType() const { return mChannelType; }
	
protected:
	//TODO: Figure out how to get context
	ImageBase();
	
	cl_channel_order	mChannelOrder;
	cl_channel_type		mChannelType;
};

typedef std::shared_ptr<class Image2d> Image2dRef;
	
class Image2d : public ImageBase {
public:
	
	struct Format : public ImageBase::Format {
		
	};
	
	~Image2d();
	
	Image2dRef create( void *data, cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch );
	
private:
	Image2d( void *data, cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch );
	Image2d( size_t width, size_t height, const Format &format = Format() );
	Image2d( const ci::gl::Texture2dRef &texture, cl_mem_flags flags, GLint mipLevelForUse = 0 );
	
	cl_int mWidth, mHeight;
	
	
};

class Image3d : public ImageBase {
public:
	
	~Image3d();
	
private:
	
};
	
} // namespace cl
