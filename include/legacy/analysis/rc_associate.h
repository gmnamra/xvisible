/*
 *
 *$Header $
 *$Id $
 *$Log$
 *Revision 1.1  2005/09/09 16:46:45  arman
 *SVD and correspondence declar
 *
 *Revision 1.1  2005/08/16 22:01:31  arman
 *SVD association
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#ifndef __RC_ASSOCIATE_H
#define __RC_ASSOCIATE_H


#include <rc_macro.h>
#include <rc_polygongroup.h>

void rfAssociate (vector<rc2Dvector>& cells, vector<rc2Dvector>& regions, 
		  vector<int32>& labels, vector<float>& scores);

#endif /* __RC_ASSOCIATE_H */
