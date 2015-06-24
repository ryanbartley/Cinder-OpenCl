//
//  Cinder-OpenCL.h
//  HelloWorld
//
//  Created by ryan bartley on 6/12/15.
//
//

#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"
#include "cinder/ImageIo.h"

namespace cl {
}

namespace cinder {
	
std::ostream& operator<<( std::ostream &lhs, const cl::Platform &rhs );
std::ostream& operator<<( std::ostream &lhs, const cl::Device &rhs );
cl_context_properties* getDefaultSharedGraphicsContextProperties( cl::Platform platform = cl::Platform() );

void convertChannelOrder( cl_channel_order channel, ImageIo::ColorModel *colorModel, ImageIo::ChannelOrder *channelOrder );
void convertChannelDataType( cl_channel_type type, ImageIo::DataType *dataType );

std::string errorToString( cl_int error );
std::string constantToString( cl_int constant );

class ImageTargetCl : public ImageTarget {
  public:
	static std::shared_ptr<ImageTargetCl> create( cl::Image2D image ) { return std::shared_ptr<ImageTargetCl>( new ImageTargetCl( image ) ); }
	
	virtual bool hasAlpha() const { return true; }
	virtual void* getRowPointer( int32_t row ) { return nullptr; }
		//		virtual bool hasAlpha() const { return mMat->channels() == 4; }
//		virtual void*	getRowPointer( int32_t row ) { return reinterpret_cast<void*>( reinterpret_cast<uint8_t*>(mMat->data) + row * mMat->step ); }
		
  protected:
	ImageTargetCl( cl::Image2D image );
	
	cl::Image2D		mImage;
};
	
class ImageSourceCl : public ImageSource {
  public:
	ImageSourceCl( const cl::CommandQueue &commandQueue, const cl::Image2D &image )
	: ImageSource()
	{
		mWidth = image.getImageInfo<CL_IMAGE_WIDTH>();
		mHeight = image.getImageInfo<CL_IMAGE_HEIGHT>();
		auto imageFormat = image.getImageInfo<CL_IMAGE_FORMAT>();
		
		// Convert the color model and channel order
		ColorModel colorModel; ChannelOrder channelOrder;
		convertChannelOrder( imageFormat.image_channel_order, &colorModel, &channelOrder );
		setColorModel( colorModel );
		setChannelOrder( channelOrder );
		
		// Convert the data type
		DataType dataType;
		convertChannelDataType( imageFormat.image_channel_data_type, &dataType );
		setDataType( dataType );
		
		mRowBytes = image.getImageInfo<CL_IMAGE_ROW_PITCH>();
		cl::size_t<3> offset;
		cl::size_t<3> region;
		region[0] = mWidth;
		region[0] = mHeight;
		mData = new uint8_t[mWidth*mHeight*dataTypeBytes(mDataType)*channelOrderNumChannels(mChannelOrder)];
		commandQueue.enqueueReadImage( image, true, offset, region, mRowBytes, 0, const_cast<uint8_t*>(mData) );
	}
	
	void load( ImageTargetRef target ) override {
		// get a pointer to the ImageSource function appropriate for handling our data configuration
		ImageSource::RowFunc func = setupRowFunc( target );
		
		const uint8_t *data = mData;
		for( int32_t row = 0; row < mHeight; ++row ) {
			((*this).*func)( target, row, data );
			data += mRowBytes;
		}
	}
	
  private:
	
	const uint8_t* mData;
	uint32_t mRowBytes;
};

} // namespace cinder
