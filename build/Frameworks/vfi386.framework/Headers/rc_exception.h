/*
 *  rc_excption.h
 *  binaworks
 *
 *  Created by arman on 1/8/09.
 *  Copyright 2009 Reify Corporation. All rights reserved.
 *
 */

#ifndef RCEXCEPTION
#define RCEXCEPTION
#include "rc_types.h"
#include <exception>
#include <iostream>
#include <sstream>

using namespace std;
// Basic Exception Processing. Clients can derive and implement methods
class general_exception : public std::exception
{
public:
    typedef std::exception Superclass;

    general_exception( const std::string& msg ) throw() :
	_location (""),  _description( msg ), _file (__FILE__), _line (__LINE__)
	{ }

    general_exception(const char *file="Unknown", unsigned int lineNumber=0,
					  const char *desc="None", const char *loc="Unknown") : _location (loc), _description (desc), _file (file), _line (lineNumber) {}

    general_exception(const std::string& file, unsigned int lineNumber,
					  const std::string& desc="None",
											const std::string& loc="Unknown") : std::exception (), _location (loc), _description (desc), _file (file), _line (lineNumber)
    {
		std::cerr << file << lineNumber << std::endl;
    };

    general_exception( const general_exception &orig ): _location (orig._location),_description (orig._description), _file (orig._file), _line (orig._line)
	{
		_location    = orig._location;

	}

    /** Virtual destructor needed for subclasses. Has to have empty throw(). */
    virtual ~general_exception() throw() {}

    /** Assignment operator. */
    general_exception &operator= ( const general_exception &orig )
	{
        _location    = orig._location;
        _description = orig._description;
        _file = orig._file;
        _line = orig._line;
        return *this;
	}

    /** Equivalence operator. */
    virtual bool operator==( const general_exception &orig )
    {
		if ( _location    == orig._location &&
			_description == orig._description &&
			_file == orig._file &&
			_line == orig._line)
        {
			return true;
        }
		else
        {
			return false;
        }
    }


    /** Methods to get and set the Location and Description fields. The Set
     * methods are overloaded to support both std::string and const char
     * array types. Get methods return const char arrays. */
    virtual void SetLocation(const std::string& s)
    { _location = s;    }
    virtual void SetDescription(const std::string& s)
    { _description = s; }
    virtual void SetLocation(const char * s)
    { _location = s;    }
    virtual void SetDescription (const char *s)
    { _description = s; }
    virtual const char *GetLocation()    const
    { return _location.c_str();    }
    virtual const char *GetDescription() const
    { return _description.c_str(); }

    /** What file did the exception occur in? */
    virtual const char *GetFile()    const
    { return _file.c_str(); }

    /** What line did the exception occur in? */
    virtual unsigned int GetLine() const
    { return _line; }

    /** Provide std::exception::what() implementation. */
    virtual const char* what() const throw()
    { return _description.c_str(); }

private:
    /** Exception data.  Location of the error and description of the error. */
    std::string  _location;
    std::string  _description;
    std::string  _file;
    unsigned int _line;

};

class parabolic_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "invalid input";
    }
};


class io_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "io error ";
    }
};

class intersectingPoints : public std::exception {
public:
    virtual const char* what() const throw() {
		return "non or more than one";
    }
};

class invalid_segment : public std::exception {
public:
    virtual const char* what() const throw() {
		return "Invalid Segment";
    }
};

class invalid_data : public std::exception {
public:
	virtual const char* what() const throw() {
		return "Invalid data ";
	}
};

class infline_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "invalid input";
    }
};

class singular_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "singular";
    }
};

class rectangle_no_overlap_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "no overlap";
    }
};

class rectangle_invalid_size_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "invalid rectangle size";
    }
};

class find_invalid_size_error : public std::exception {
public:
    virtual const char* what() const throw() {
		return "invalid input image size";
    }
};

class divide_by_zero_error : public std::exception {
public:
	virtual const char* what() const throw() {
		return "divide_by_zero ";
	}
};

class not_implemented_error : public general_exception {
public:
	virtual const char* what() const throw() {
		return " not implemented ";
	}
};




/** The exception macro is used to print error information (i.e., usually
 * a condition that results in program failure). Example usage looks like:
 * general_exceptionMacro(<< "this is error info" << this->SomeVariable);
 */

#define rmExceptionMacro(x)                                             \
{                                                                     \
ostringstream message;                                              \
message << "Reify Class Libray: " x;                                \
throw general_exception(__FILE__, __LINE__, message.str().c_str());       \
}


#endif

