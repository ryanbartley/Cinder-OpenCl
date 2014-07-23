//
//  Program.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#pragma once

#include <OpenCl/OpenCl.h>

#define BINARY 0

namespace cinder { namespace cl {

class Program;
typedef std::shared_ptr<Program> ProgramRef;
typedef std::shared_ptr<class Platform> PlatformRef;
typedef std::shared_ptr<class Device> DeviceRef;
typedef std::shared_ptr<class BufferObj> BufferObjRef;

class Program : public boost::noncopyable, public std::enable_shared_from_this<Program> {
public:

	class Kernel : public boost::noncopyable, public std::enable_shared_from_this<Kernel> {
	public:
		typedef std::shared_ptr<Kernel> KernelRef;
		
		static KernelRef create( const ProgramRef &program, const std::string &name );
		static KernelRef create( cl_kernel kernel, const std::string &name );
		
		~Kernel();
		
		void setKernelArg( cl_int index, size_t size, void *data );
		void setKernelArg( cl_int index, const BufferObjRef &buffer );
		
		const cl_kernel getId() { return mId; }
		const std::string& getName() { return mName; }
		
	private:
		Kernel( const ProgramRef &program, const std::string &name );
		Kernel( cl_kernel kernel, const std::string &name );
		
		cl_kernel	mId;
		std::string mName;
	};
	
	typedef std::shared_ptr<Kernel> KernelRef;
	
	static ProgramRef create( const DataSourceRef &dataSource, const PlatformRef &platform = nullptr, const std::string *options = nullptr, bool createKernels = false );
	
#if BINARY
	// TODO: Test this
	static ProgramRef createFromBinary( const DataSourceRef &binaryData );
#endif
	
	~Program();
	
	const cl_program getId() { return mId; }
	const std::map<std::string, Program::KernelRef>& getKernelMap() const { return mKernels; }
	std::map<std::string, Program::KernelRef>& getKernelMap() { return mKernels; }
	KernelRef getKernelByName( const std::string &name );
	cl_kernel getKernelIdByName( const std::string &name );
	
	KernelRef createKernel( const std::string &name );
	void setKernel( const KernelRef &kernel );
	
	void setKernelArg( const std::string &name, cl_int index, size_t size, void *data );
	void setKernelArg( const std::string &name, cl_int index, const BufferObjRef &buffer );
	
	bool saveBinaries( const std::map<std::string, DeviceRef> &fileNameDevicePair );
	
private:
	Program( const DataSourceRef &dataSource, const PlatformRef &platform, const std::string *options, bool createKernels );
	
#if BINARY
	// TODO: Test this
	Program( const std::string &binaryFileName );
#endif
	
	static void CL_CALLBACK programBuildCallback( cl_program program, void *userData );
	
	cl_program mId;
	std::map<std::string, Program::KernelRef> mKernels;
};
	
typedef std::shared_ptr<typename Program::Kernel> KernelRef;
	
}}