/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.2  2005/10/12 22:04:17  arman
 *incremental
 *
 *Revision 1.1  2005/04/29 21:58:33  arman
 *basic unix based file/directory utils
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcFILEUTILS_H

#include <iostream>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/dir.h>
#include <vector>
#include <deque>
#include <rc_types.h>
#include <rc_pair.h>
#include<cstdio>
#include<list>
#include<iterator>
#include<stdexcept>

extern "C" {
	#include<glob.h>
}

using namespace std;


/*
Sample code for CUJ article,
"Custom Containers & Iterators for STL-Friendly Code"
(March 2005)
by Ethan McCallum


Copyright (c) 2004, Ethan McCallum.

Permission is granted to use this code without restriction as long as this
copyright notice appears in all source files.
*/


/**
\brief an STL-style iterator for the STLGlob class
*/
class rcGlobIterator
	: public std::iterator< std::forward_iterator_tag , const std::string , int >
{

	public:

	/// convenience typedefs for the increment operators
	typedef rcGlobIterator self_type ;
	typedef self_type& self_reference ;

	/// builds default "end" object
	explicit rcGlobIterator() ;

	/// beginning or middle of a glob's range
	explicit rcGlobIterator( char** list ) ;

	/// basic copy ctor
	rcGlobIterator( const rcGlobIterator& other ) ;

	/// basic assignment operator
	self_reference operator=( const rcGlobIterator& other ) ;

	/**
	\brief do-nothing dtor

	NOTE: the dtor does NOT free() the char** passed in the ctor!
	it belongs to the glob_t from the parent rcGlob object
	*/
	~rcGlobIterator() throw() {
	} // dtor

	/// move forward one item, pre-increment
	self_reference operator++() ;

	/// move forward one item, post-increment
	self_type operator++(int) ;

	/// test whether this iterator is the same as another
	bool operator==( const self_reference other ) ;


	/// test whether this iterator is not the same as another
	bool operator!=( const self_reference other ) ;

	/// dereference the iterator: returns the current path as a std::string
	reference operator*() ;

	/// access the underlying object, in this case a std::string
	reference operator->() ;


	protected:
	/* empty */


	private:
	/// the list from the parent rcGlob's underlying glob_t
	char** sequence_ ;

	/// the current item in the list
	std::string current_ ;


} ; // class rcGlobIterator


/**
\brief C++ wrapper around the glob() system call.

(extracted from my rpm_find_latest project, 2004/04)

It works similar to an STL container:
Pass it a list<> of paths and use the iterators to
touch each item, as expanded by the system call.

This class is named "rcGlob" because the name "Glob"
conflicted with an existing class somewhere (but not
within this project).

*/


class rcGlob {

	public:
	/// STL-style iterator for this container
	typedef rcGlobIterator const_iterator ;

	/// type returned by size()
	typedef int size_type ;


	/// prints an error message when there's a problem expanding a pattern
	static int globErrorHandler( const char* error_path , int error_num ) ;

	/// empty ctor -- requires that you call push_back() to add some patterns
	rcGlob() ;

	/// create an object with one pattern.  You can still call push_back() to add more.
	rcGlob( const std::string& inPattern ) throw( std::runtime_error ) ;

	/// basic dtor -- destroys the underlying glob_t
	~rcGlob() throw() ;

	/**
	\brief adds a pattern to the list

	Calling this invalidates any existing iterators.

	\throw std::runtime_error when the pattern doesn't expand (i.e. the underlying glob() call fails)
	*/
	void push_back( const std::string& inPattern ) throw( std::runtime_error ) ;

	/// returns an iterator to the beginning of the collection
	const_iterator begin() const ;

	/// returns an iterator signaling <I>past-the-end</I> value of the collection
	const_iterator end() const ;

	/// returns number of elements in 
	int size() const throw() ;

	protected:
	/* empty */

	private:
	/// holds the expanded paths in a char**
	glob_t glob_ ;

	/// not implemented
	rcGlob( const rcGlob& ) ;

	/// not implemented
	rcGlob& operator=( const rcGlob& ) ;

} ; // class rcGlob


extern int alphasort ();

void RFY_API rfGetDirEntries(const string & dirname, 
		     vector<std::string> & entries, const char *imageformat = "tif");

bool RFY_API rfIsDirectory (const char * inFullPath);
int32 RFY_API rfGetFileSize (const char *inFullPath);

bool RFY_API rfFileExists(const std::string& filename);

std::string RFY_API rfStripPath( const std::string& fileName );
std::string RFY_API rfStripExtension( const std::string& fileName );
std::string RFY_API rfGetExtension( const std::string& fileName );

std::string RFY_API rfMakeTmpFileName(const char *pathFormat, const char* baseName );
std::string RFY_API rfMakeTmpFileName(const char* baseName );

static const char *defaultBase = "zoubin";

std::string rfDumpMatrix (deque<deque<double> >& matrix, const char *pathFormat = 0,
		       const char* baseName = defaultBase);

#define __rcFILEUTILS_H

#endif /* __rcFILEUTILS_H */
