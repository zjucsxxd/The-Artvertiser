/*
 *  FTime.cpp
 *  F
 *
 *  Created by damian on 21/03/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "FTime.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef OSX
#else
#include <time.h>
#endif

#include <assert.h>

double FTime::Update()
{
	#ifdef OSX
	uint64_t now = mach_absolute_time();
	uint64_t diff = now - time;

	static double conversion = 0.0;
	if ( conversion == 0.0 )
	{
		mach_timebase_info_data_t info;
		kern_return_t err = mach_timebase_info( &info );

		if( err == 0 )
			conversion = 1e-9*(double)info.numer / (double) info.denom;
	}

	time = now;
	last_update_time = (float)(conversion * (double) diff);

	#else
	timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	// calculate diff
	double diff = now.tv_sec - time.tv_sec;
	diff += 1e-9*double(now.tv_nsec - time.tv_nsec);

	time = now;
	last_update_time = diff;

	#endif
	return last_update_time;
}

void FTime::SetSeconds( double seconds )
{
    #ifdef OSX
    assert( false && "implement me" );
    #else
    // truncate
    time.tv_sec = (long long)seconds;
    // nanoseconds
    time.tv_nsec = (seconds - (long long)seconds)*1e9;
    #endif

}


double FTime::ToMillis() const
{
	#ifdef OSX
	static double conversion = 0.0;
	if ( conversion == 0.0 )
	{
		mach_timebase_info_data_t info;
		kern_return_t err = mach_timebase_info( &info );

		if( err == 0 )
			conversion = 1e-6*(double)info.numer / (double) info.denom;
	}

	return (conversion * (double) time);

	#else

	double result = 1e3*double(time.tv_sec) + 1e-6*double(time.tv_nsec);
	return result;

	#endif

}
