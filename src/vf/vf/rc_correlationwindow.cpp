/*
 * $Id: rc_correlationwindow.cpp 4186 2006-02-13 15:58:57Z armanmg $
 *
 * Wrapper cache class for rcWindow
 *
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 */

#include <rc_correlationwindow.h>

// Always instantiate
template class rcCorrelationWindow<uint8>;
template class rcCorrelationWindow<uint16>;
template class rcCorrelationWindow<uint32>;

