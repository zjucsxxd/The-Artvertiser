/*
 *  FProfiler.cpp
 *  F
 *
 *  Created by damian on 25/5/08.
 *  Copyright 2008 frey damian@frey.co.nz. All rights reserved.
 *
 */

#include "FProfiler.h"

#include <algorithm>

#include "FThread.h"

FProfiler::FProfileContexts FProfiler::contexts;
FSemaphore FProfiler::lock;

FProfileContext* FProfiler::GetContext()
{
	FThreadContext thread_context;
	lock.Wait();
	// try to get current thread context
	for ( FProfileContexts::const_iterator i = contexts.begin();
		  i!= contexts.end();
		  ++i )
	{
		if ( thread_context == ( *i )->thread_context )
		{
			lock.Signal();
			return *i;
		}
	}

	// no context found for this thread: must create a new one
	FProfileContext* context = new FProfileContext();
	// add it to the vector
	contexts.push_back( context );
	// fill in details
	context->thread_context.Set();
	context->toplevel = new FProfileSection();
	context->current = context->toplevel;

	// return
	lock.Signal();
	return context;
}

void FProfiler::Clear()
{
    // get lock
    lock.Wait();
    // delete everything
    for ( int i=0; i<contexts.size(); i++ )
    {
        delete contexts[i];
    }
    contexts.clear();

    // done
    lock.Signal();

}

void FProfiler::SectionPush(const std::string &name)
{
	FProfileContext* context = GetContext();
	assert( context->current );

	// try to grab the section out of the db
	// we store by name so that we can accumulate results over multiple frames
	FProfileSection* s = context->current->children[name];
	if ( s == NULL )
	{
		s = new FProfileSection();
		s->parent = context->current;
		context->current->children[name] = s;
	}

	// shift current to us
	context->current = s;

	// store start time
	context->current->start_time.SetNow();
//	QueryPerformanceCounter(&s->start_time);

}


void FProfiler::SectionPop()
{
	FTime stop_time;
	stop_time.SetNow();

	// grab the section
	FProfileContext* context = GetContext();
	FProfileSection* s = context->current;

    // check we're not popping up too far
	if ( context->current->parent == NULL )
        return;

	// get time for this run
/*	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	double time = 1000*(double)(stop_time.QuadPart - s->start_time.QuadPart)/(double)freq.QuadPart;
	*/

	stop_time -= s->start_time;
	double time = stop_time.ToMillis();

	// work out the new avg time and increment the call count
	double total_time = time + s->avg_time * s->call_count++;
	s->avg_time = total_time/(double)s->call_count;

	// shift current up
	context->current = context->current->parent;
}

void FProfiler::Display( /*FProfiler::SORT_BY sort*/ )
{
	printf("---------------------------------------------------------------------------------------\n" );
    // re-use formatting from individual lines
    printf( "%-50s  %10s  %10s  %6s\n", "name                            values in ms -> ", "total ", "average ", "count" );
    printf("---------------------------------------------------------------------------------------\n" );
	lock.Wait();
	for ( FProfileContexts::iterator i = contexts.begin();
		  i != contexts.end();
		  ++i )
	{
		printf("Thread %x\n", &((*i)->thread_context) );
		(*i)->toplevel->Display("");
	}
	lock.Signal();
	printf("---------------------------------------------------------------------------------------\n" );
}


FProfileSection::FProfileSection()
{
	parent = NULL;
	avg_time = 0;
	call_count = 0;
}

FProfileSection::~FProfileSection()
{
    for ( FProfileSections::iterator i = children.begin();
        i != children.end();
        ++i )
    {
        delete (*i).second;
    }
    children.clear();
}

bool less_than_comparator( FProfileSection* a, FProfileSection* b )
{
	return a->avg_time*a->call_count < b->avg_time*b->call_count;
}

void FProfileSection::Display( const std::string& prefix )
{
	for ( FProfileSections::iterator i = children.begin();
		  i!= children.end();
		  ++i )
	{
		std::string name = prefix + (*i).first;
		printf( "%-50s  %10.2f  %10.5f  %6d\n", name.c_str(),
				  (*i).second->avg_time * (*i).second->call_count,
				  (*i).second->avg_time, (*i).second->call_count );
		(*i).second->Display( prefix + "| " );
	}
}
