/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.6  2005/11/16 18:38:04  arman
 *added BOOST's concatenation macros
 *
 *Revision 1.5  2005/10/27 16:42:39  arman
 *added String Macros
 *
 *Revision 1.4  2005/10/25 21:56:54  arman
 *added Get macro
 *
 *Revision 1.3  2005/08/30 21:08:51  arman
 *Cell Lineage
 *
 *Revision 1.4  2005/08/01 01:47:25  arman
 *cell lineage addition
 *
 *Revision 1.3  2005/07/29 21:41:05  arman
 *cell lineage incremental
 *
 *Revision 1.2  2005/07/01 21:05:40  arman
 *version2.0p
 *
 *Revision 1.3  2005/06/02 22:32:00  arman
 *removed point plot
 *
 *Revision 1.2  2005/06/02 01:00:53  arman
 *added a macro to ease gnuplot usage
 *
 *Revision 1.1  2004/10/06 21:35:32  arman
 *variety of macros for class definitions
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcMACRO_H
#define __rcMACRO_H


// 16 byte Address Alignment and count multiple of 4
#define rmIsAlignedAddr(a)    ( ((long)a & 15L) == 0 )
#define rmIsAlignedCount(n)   ( (n > 0) && ((n & 3L) == 0) )


#define rmBytesInGig 1000000000

#define SetTrait(nspace,trait,type,val)\
 namespace nspace { \
 template <> \
 struct trait < type > { \
 public: \
 static const bool value = val; \
 }; }






#   define rmStaticConstMacro(name,type,value) enum { name = value }

#   define rmGetStaticConstMacro(name) (Self::name)

#define rmWarningMacro(x) \
{\
      cerr << "WARNING: In " __FILE__ ", line " << __LINE__ << "\n" \
             x << "\n\n"; \
}

/** Get built-in type.  Creates member Get"name"() (e.g., GetVisibility());
 * This is the "const" form of the itkGetMacro.  It should be used unless
 * the member can be changed through the "Get" access routine. */
#define rmGetConstMacro(name,type) \
  virtual type Get##name () const \
  { \
    rmWarningMacro("returning " << #name " of " << this->m##name ); \
    return this->m##name; \
  }



/** Set built-in type.  Creates member Set"name"() (e.g., SetVisibility()); */
#define rmSetMacro(name,type) \
  virtual void Set##name (const type _arg) \
  { \
    if (this->m##name != _arg) \
      { \
      this->m##name = _arg; \
      } \
  }

/** Get built-in type.  Creates member Get"name"() (e.g., GetVisibility()); */

#define rmGetMacro(name,type) \
  virtual type Get##name () \
  { \
    return this->m_##name; \
  }


/** Set character string.  Creates member Set"name"()
 * (e.g., SetFilename(char *)). The macro assumes that
 * the class member (name) is declared a type std::string.
 */

#define rmSetStringMacro(name) \
  virtual void Set##name (const char* _arg) \
  { \
    if ( _arg && (_arg == this->m##name) ) { return;} \
    if (_arg) \
      { \
      this->m##name = _arg;\
      } \
     else \
      { \
      this->m##name = ""; \
      } \
  }


/** Get character string.  Creates member Get"name"()
 * (e.g., SetFilename(char *)). The macro assumes that
 * the class member (name) is declared a type std::string.
 */

#define rmGetStringMacro(name) \
  const char* Get##name () const		\
  { \
    return this->m##name.c_str(); \
  }



// Use this instead of incorrectly names rcFoo

#define rmExceptionMacro(x) \
  { \
    ostringstream message; \
    message << "Reify Class Libray: " x; \
    throw general_exception(__FILE__, __LINE__, message.str().c_str()); \
  }


#define rmGplot(a,b,c)\
  {    GnuPlotInterface gpi;			\
    gpi.Plot2DFct ((a), \
		   0.0, (double) ((a).width() -1), (uint32) (a).width(),\
		   0.0, (double) ((a).height() -1), (uint32) (a).height());\
  }





#endif /* __rcMACRO_H */
