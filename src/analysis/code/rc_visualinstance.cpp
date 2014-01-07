/*
 *
 *$Id $
 *$Log$
 *Revision 1.2  2006/01/10 23:42:59  arman
 *pre-rel2
 *
 *Revision 1.1  2005/08/30 21:02:22  arman
 **** empty log message ***
 *
 *Revision 1.1  2005/07/22 16:37:00  arman
 *instantiation of the template functions in rcVisualFunction
 *
 *
 *
 * Copyright (c) 2002 Reify Corp. All rights reserved.
 */
#include <rc_analysis.h>
#include <rc_kinetoscope.h>
#include <rc_math.h>
#include <iomanip.h>
#include <algorithm>
#include <rc_lsfit.h>
#include <rc_xforms.h>
#include <rc_affinewindow.h>
#include <rc_shape.h>
#include <rc_lsfit.h>

#include <rc_mathmodel.h>
#include <rc_peak.h>
#include <rc_1dcorr.h>
#include <rc_stats.h>
#include <rc_filter1d.h>


template rcIRect rcVisualFunction::roundRectangle<float>(const rcRectangle<float>&, bool) const;
template rcIRect rcVisualFunction::roundRectangle<double>(const rcRectangle<double>&, bool) const;
