//
//  BufferObj.h
//  ImplProject
//
//  Created by Ryan Bartley on 2/12/14.
//
//

#pragma once

#include <OpenCL/OpenCL.h>
#include "MemoryObj.h"

//namespace cinder { namespace gl {
//		using BufferObjRef = std::shared_ptr<class BufferObj>;
//}}

namespace cl {
	
using BufferObjRef = std::shared_ptr<class BufferObj>;
using BufferDestructorCallback = void(*)( cl_mem, void *);

class BufferObj : public std::enable_shared_from_this<BufferObj>, public MemoryObj {
public:
	
	static BufferObjRef create( cl_mem_flags flags, size_t size, void *data );
	
	class SubBuffer : public std::enable_shared_from_this<SubBuffer>, public MemoryObj {
	public:
		using SubBufferRef = std::shared_ptr<SubBuffer>;
		
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		static SubBufferRef create( const BufferObjRef &buffer, cl_mem_flags flags, size_t origin, size_t size );
		
		~SubBuffer(){}
		
		// NON COPYABLE
		SubBuffer( const SubBuffer &other ) = delete;
		SubBuffer& operator=( const SubBuffer &other ) = delete;
		SubBuffer( SubBuffer&& other ) = delete;
		SubBuffer& operator=( SubBuffer&& other ) = delete;
		
	private:
		SubBuffer( const BufferObjRef &buffer, cl_mem_flags flags, const cl_buffer_region *bufferCreateInfo );
		
		size_t			mOrigin;
		size_t			mSize;
		BufferObjRef	mParent;
	};
	
	~BufferObj() {}
	
	// NON COPYABLE
	BufferObj( const BufferObj &other ) = delete;
	BufferObj& operator=( const BufferObj &other ) = delete;
	BufferObj( BufferObj&& other ) = delete;
	BufferObj& operator=( BufferObj&& other ) = delete;
	
	size_t			getSize() { return mSize; }
	
	// TODO: Need to decide if these are good options
	void partitionBuffer( size_t origin, size_t size );
	void partitionBuffer( size_t divisor );
	
private:
	BufferObj( cl_mem_flags flags, size_t size, void *data );
	// TODO: Make a directX buffer object for shared context
	BufferObj( cl_mem_flags flags, size_t size );
	
	using SubBufferRef = std::shared_ptr<SubBuffer>;
	
	size_t						mSize;
	std::vector<SubBufferRef>	mSubBuffers;
	
	friend BufferObjRef createBufferFromGl( const ci::gl::BufferObjRef&, cl_mem_flags );
};
	
using SubBufferRef = std::shared_ptr<BufferObj::SubBuffer>;
	
} // namespace cl
