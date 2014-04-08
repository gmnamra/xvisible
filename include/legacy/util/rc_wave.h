/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/05/01 18:36:31  arman
 *mono channel wave generating functions
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __rcWAVE_H
#define __rcWAVE_H
/* ---
   by Christophe Schlick (10 September 1993)    "Wave Generators for Procedural Techniques in Computer Graphics" 
   in Graphics Gems V (edited by A. Paeth), Academic Press
*/

extern double Rwave (register double t, double s, double Fvar, double Avar); 

extern double Twave (register double t, double s, double Fvar, double Avar); 

extern double Swave (register double t, double s, double Fvar, double Avar); #endif 

#endif /* __rcWAVE_H */
