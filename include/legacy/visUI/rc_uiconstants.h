#ifndef UI_RCUICONSTANTS_H
#define UI_RCUICONSTANTS_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>


// Widget text alignment

// Automatically break on word boundaries
const int cUIWordBreakAlignment = (Qt::AlignRight | Qt::AlignCenter | Qt::WordBreak);
// No word breaking, keeps text as single line
const int cUINoBreakAlignment = (Qt::AlignRight | Qt::AlignCenter);

#endif // UI_RCUICONSTANTS_H
