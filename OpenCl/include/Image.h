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

namespace cinder { namespace cl {

// need to make this an abstract class to forgoe needing a virtual destructor
class ImageBase : public MemoryObj {
public:
	virtual ~ImageBase();
private:
	ImageBase();
	
	cl_int width, height;
	
};

typedef std::shared_ptr<class Image2d> Image2dRef;
	
class Image2d : public ImageBase {
public:
	
	~Image2d();
	
private:
	Image2d( cl_mem_flags flags, const cl_image_format *image_format, size_t width, size_t height, size_t row_pitch, void *data );
	Image2d( );
	
};

class Image3d : public ImageBase {
public:
	
	~Image3d();
	
private:
	
};
	
}}
