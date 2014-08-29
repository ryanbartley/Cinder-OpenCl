//
//  Program.cpp
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#include "Program.h"
#include "Context.h"
#include "Platform.h"
#include "Device.h"
#include "BufferObj.h"

namespace cinder { namespace cl {
	
Program::Kernel::Kernel( const ProgramRef &program, const std::string &name  )
: mId( nullptr ), mName( name )
{
	cl_int errNum;
	
	mId = clCreateKernel( program->getId(), name.c_str(), &errNum );
	
	if( errNum != CL_SUCCESS ) {
		std::cerr << "Error: Creating Kernel " << name << " - Error: " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
Program::Kernel::Kernel( cl_kernel kernel, const std::string &name )
: mId( kernel ), mName( name )
{
}
	
Program::Kernel::~Kernel()
{
	clReleaseKernel(mId);
}
	
Program::KernelRef Program::Kernel::create( const ProgramRef &program, const std::string &name )
{
	return Program::KernelRef( new Program::Kernel( program, name ) );
}
	
Program::KernelRef Program::Kernel::create( cl_kernel kernel, const std::string &name )
{
	return Program::KernelRef( new Program::Kernel( kernel, name ) );
}
	
void Program::Kernel::setKernelArg( cl_int index, size_t size, void *data )
{
	cl_int errNum;
	errNum = clSetKernelArg( mId, index, size, data );
	if( errNum != CL_SUCCESS ) {
		std::cerr << "Error: Set Kernel Arg with size and Data " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}
	
void Program::Kernel::setKernelArg( cl_int index, const BufferObjRef &buffer )
{
	cl_int errNum;
	// I have to get the memory object as a pointer
	// This was an enormous problem that I couldn't
	// figure out for a while
	auto memBuff = buffer->getId();
	
	errNum = clSetKernelArg( mId, index, sizeof(cl_mem), &memBuff );
	if( errNum != CL_SUCCESS ) {
		std::cerr << "Error: Set Kernel Arg with BufferObj " << errNum << std::endl;
		exit(EXIT_FAILURE);
	}
}

Program::Program( const DataSourceRef &dataSource, const PlatformRef &platform, const std::string *options, bool createKernels )
{
	cl_int errNum;
	std::string program;
	Buffer buffer( dataSource );
	program.resize( buffer.getDataSize() + 1 );
	memcpy( (void*)program.data(), buffer.getData(), buffer.getDataSize() );
	program[buffer.getDataSize()] = 0;
	auto curContext = Context::context()->getId();
	size_t size = program.size() - 1;
	
	const char * prog[] = {program.c_str()};
	
	mId = clCreateProgramWithSource( curContext, 1, prog, &size, &errNum );
	
	if( mId == NULL ) {
		std::cerr << "Failed to create CL program from source" << std::endl;
		exit(EXIT_FAILURE);
	}
	if( errNum != CL_SUCCESS ) {
		std::cerr << "ERROR: Creating Program from Source - " << errNum << std::endl;
	}
	
	if( platform ) {
		auto deviceIds = platform->getDeviceIds( CL_DEVICE_TYPE_ALL );
		const char* ccOptions = options ? (const char *)options->c_str() : nullptr;
		
		errNum |= clBuildProgram( mId, deviceIds.size(), deviceIds.data(), ccOptions, &Program::programBuildCallback, this );
	}
	else {
		const char* ccOptions = options ? (const char *)options->c_str() : nullptr;
		
		errNum |= clBuildProgram( mId, 0, NULL, ccOptions, &Program::programBuildCallback, this );
	}
	
	if (errNum != CL_SUCCESS)
	{
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo( mId, nullptr, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL );
		std::cerr << "Error in kernel: " << std::endl;
		std::cerr << buildLog;
		clReleaseProgram(mId);
		exit(EXIT_FAILURE);
	}
	
	if( createKernels ) {
		
		cl_uint numKernels;
		errNum = clCreateKernelsInProgram( mId, NULL, NULL, &numKernels );
		
		cl_kernel *kernels = new cl_kernel[numKernels];
		
		errNum = clCreateKernelsInProgram( mId, numKernels, kernels, &numKernels );
		if( errNum != CL_SUCCESS ) {
			std::cerr << "Error: Creating Kernels in Program " << errNum << std::endl;
			exit( EXIT_FAILURE );
		}
		
		for( int i = 0; i < numKernels; ++i ) {
			size_t kerNameSize;
			
			errNum = clGetKernelInfo( kernels[i], CL_KERNEL_FUNCTION_NAME, 0, nullptr, &kerNameSize );
			
			char * name = new char[kerNameSize];
			
			errNum = clGetKernelInfo( kernels[i], CL_KERNEL_FUNCTION_NAME, kerNameSize * sizeof(char), name, 0 );
			
			errNum = clCreateKernelsInProgram( mId, numKernels, kernels, &numKernels );
			if( errNum != CL_SUCCESS ) {
				std::cerr << "Error: Getting Kernel Name in Program " << errNum << std::endl;
				exit( EXIT_FAILURE );
			}
			
			mKernels.insert( std::pair<std::string, Program::KernelRef>( name, Program::Kernel::create( kernels[i], name ) ) );
			
			delete [] name;
		}
		
		delete [] kernels;
	}
}
	
#if BINARY
Program::Program( const std::string &binaryFileName )
{
	// TODO: Don't Use this it's still experimental;
	
	FILE *fp = fopen( binaryFileName.c_str(), "rb");
    if (fp == NULL) {
		std::cerr << "Error: filename Not found Creating Program from Binary " << std::endl;
		exit(EXIT_FAILURE);
    }
	
    // Determine the size of the binary
    size_t binarySize;
    fseek(fp, 0, SEEK_END);
    binarySize = ftell(fp);
    rewind(fp);
	
    // Load binary from disk
    unsigned char *programBinary = new unsigned char[binarySize];
    fread(programBinary, 1, binarySize, fp);
    fclose(fp);
	
    cl_int errNum = 0;
    cl_int binaryStatus;
    mId = clCreateProgramWithBinary( Context::context()->getId(), 1, &device, &binarySize, (const unsigned char**)&programBinary, &binaryStatus, &errNum );
    delete [] programBinary;
    if (errNum != CL_SUCCESS) {
        std::cerr << "Error loading program binary." << std::endl;
		exit(EXIT_FAILURE);
    }
	if (binaryStatus != CL_SUCCESS) {
		std::cerr << "Invalid binary for device" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	errNum = clBuildProgram( mId, 0, NULL, NULL, NULL, NULL );
	
	if (errNum != CL_SUCCESS) {
		// Determine the reason for the error
		char buildLog[16384];
		clGetProgramBuildInfo( mId, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL );
		std::cerr << "Error in program: " << std::endl;
		std::cerr << buildLog << std::endl;
		clReleaseProgram( mId );
		exit(EXIT_FAILURE);
	}
	
}
#endif
	
ProgramRef Program::create( const DataSourceRef &dataSource, const PlatformRef &platform, const std::string *options, bool createKernels )
{
	return ProgramRef( new Program( dataSource, platform, options, createKernels ) );
}

#if BINARY
ProgramRef Program::createFromBinary( const DataSourceRef &dataSource )
{
	return ProgramRef( new Program( dataSource ) );
}
#endif

Program::~Program()
{
	clReleaseProgram(mId);
}
	
void Program::setKernel( const KernelRef &kernel )
{
	auto found = mKernels.find( kernel->getName() );
	if( found == mKernels.end() ) {
		mKernels.insert( std::pair<std::string, KernelRef>(kernel->getName(), kernel) );
	}
	else {
		std::cout << "Already placed Kernel: " << kernel->getName() << std::endl;
	}
}
	
KernelRef Program::createKernel( const std::string &name )
{
	auto found = mKernels.find( name );
	if( found == mKernels.end() ) {
		auto kernel = mKernels.insert( std::pair<std::string, KernelRef>(name, Kernel::create( shared_from_this(), name ) ) );
		return kernel.first->second;
	}
	else {
		std::cout << "Kernel: " << name << " already created." << std::endl;
		return found->second;
	}
}
	
void Program::setKernelArg( const std::string &name, cl_int index, size_t size, void *data )
{
	auto found = mKernels.find( name );
	if( found != mKernels.end() ) {
		found->second->setKernelArg( index, size, data );
	}
	else {
		std::cout << "Error: Couldn't find kernel - " << name << std::endl;
	}
}
	
void Program::setKernelArg( const std::string &name, cl_int index, const BufferObjRef &buffer )
{
	auto found = mKernels.find( name );
	if( found != mKernels.end() ) {
		found->second->setKernelArg( index, buffer );
	}
	else {
		std::cout << "Error: Couldn't find kernel - " << name << " on this program " << std::endl;
	}
}

void Program::programBuildCallback( cl_program program, void *userData )
{
	// TODO: Log errors
	std::cout << "Program Build successful " << program << std::endl;
	auto kernels = ((Program*)(userData))->getKernelMap();
	for( auto kernelFuncIt = kernels.begin(); kernelFuncIt != kernels.end(); ++kernelFuncIt ) {
		std::cerr << "Program Built: " << (*kernelFuncIt).first << std::endl;
	}
}
	
bool Program::saveBinaries( const std::map<std::string, DeviceRef> &fileNameDevicePair )
{
	cl_uint numDevices = 0;
	cl_int errNum;
	// 1 - Query for number of devices attached to program
	errNum = clGetProgramInfo( mId, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &numDevices, NULL );
	
	if (errNum != CL_SUCCESS) {
		std::cerr << "Error querying for number of devices." << std::endl;
		return false;
	}
	
	// 2 - Get all of the Device IDs
	cl_device_id *devices = new cl_device_id[numDevices];
	
	errNum = clGetProgramInfo( mId, CL_PROGRAM_DEVICES, sizeof(cl_device_id) * numDevices, devices, NULL );
	if (errNum != CL_SUCCESS) {
		std::cerr << "Error querying for devices." << std::endl;
		delete [] devices;
		return false;
	}
	
	// 3 - Determine the size of each program binary
	size_t *programBinarySizes = new size_t [numDevices];
	
	errNum = clGetProgramInfo( mId, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * numDevices, programBinarySizes, NULL );
	if (errNum != CL_SUCCESS) {
		std::cerr << "Error querying for program binary sizes." << std::endl;
		delete [] devices;
		delete [] programBinarySizes;
		return false;
	}
	
	unsigned char **programBinaries = new unsigned char*[numDevices];
	
	for ( cl_uint i = 0; i < numDevices; i++ ) {
		programBinaries[i] = new unsigned char[programBinarySizes[i]];
	}
	
	// 4 - Get all of the program binaries
	errNum = clGetProgramInfo( mId, CL_PROGRAM_BINARIES, sizeof(unsigned char*) * numDevices, programBinaries, NULL );
	
	if (errNum != CL_SUCCESS) {
		std::cerr << "Error querying for program binaries" << std::endl;
		delete [] devices;
		delete [] programBinarySizes;
		for (cl_uint i = 0; i < numDevices; i++) {
			delete [] programBinaries[i];
		}
		delete [] programBinaries;
		return false;
	}
	
	// 5 - Finally store the binaries for the device requested
	//     out to disk for future reading.
	for (cl_uint i = 0; i < numDevices; i++) {
		// Store the binary just for the device requested.
		// In a scenario where multiple devices were being used
		// you would save all of the binaries out here.
		for ( auto fileDevIt = fileNameDevicePair.begin(); fileDevIt != fileNameDevicePair.end(); ++fileDevIt ) {
			if ( devices[i] == fileDevIt->second->getId() ) {
				FILE *fp = fopen(fileDevIt->first.c_str(), "wb");
				fwrite( programBinaries[i], 1, programBinarySizes[i], fp );
				fclose(fp);
				break;
			}
		}
	}
	// Cleanup
	delete [] devices;
	delete [] programBinarySizes;
	for (cl_uint i = 0; i < numDevices; i++)
	{
		delete [] programBinaries[i];
	}
	delete [] programBinaries;
	return true;
}
	
Program::KernelRef Program::getKernelByName( const std::string &name )
{
	auto kernelMap = getKernelMap();
	auto found = kernelMap.find( name );
	if( found != kernelMap.end() ) {
		return found->second;
	}
	else {
		return Program::KernelRef();
	}
}
	
cl_kernel Program::getKernelIdByName( const std::string &name )
{
	auto kernelMap = getKernelMap();
	auto found = kernelMap.find( name );
	if( found != kernelMap.end() ) {
		return found->second->getId();
	}
	else {
		throw "ERROR: Program::getKernelIdByName - Couldn't find kernel";
	}
}
	
}}