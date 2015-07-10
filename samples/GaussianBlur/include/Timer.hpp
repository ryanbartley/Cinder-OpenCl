//
//  Timer.hpp
//  Cube
//
//  Created by Ryan Bartley on 1/29/15.
//
//

#pragma once

#include <chrono>
#include <ostream>

class Timer {
	using high_resolution_clock = std::chrono::high_resolution_clock;
	using milliseconds = std::chrono::milliseconds;
public:
	explicit Timer(bool run = false)
	: mStart( high_resolution_clock::now() )
	{
		if (run)
			start();
	}
	
	void start()
	{
		mStart = high_resolution_clock::now();
	}
	
	milliseconds getElapsed() const
	{
		return std::chrono::duration_cast<milliseconds>(high_resolution_clock::now() - mStart);
	}
    
    inline double getSeconds() const
    {
        return getElapsed().count() / 1000.0;
    }
    
	
	template <typename T, typename Traits>
	friend std::basic_ostream<T, Traits>& operator<<(std::basic_ostream<T, Traits>& out, const Timer& timer)
	{
		return out << timer.getElapsed().count();
	}
	
private:
	high_resolution_clock::time_point mStart;
};