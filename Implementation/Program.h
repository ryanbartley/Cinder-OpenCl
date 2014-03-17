//
//  Program.h
//  ImplProject
//
//  Created by Ryan Bartley on 3/16/14.
//
//

#pragma once

#include <OpenCl/OpenCl.h>

namespace cinder { namespace cl {
	
typedef std::shared_ptr<class Program> ProgramRef;

class Program : public boost::noncopyable, public std::enable_shared_from_this<Program> {
public:
	static ProgramRef create( const DataSourceRef &dataSource );
	
	~Program();
	
private:
	Program( const DataSourceRef &dataSource );
	
	cl_program mId;
};
	
}}