/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_mainwindow.h 6058 2008-09-16 05:25:58Z arman $
 *
 *	This file contains main window ui definitions.
 *
 ******************************************************************************/

#ifndef UI_RCMAINWINDOW_H
#define UI_RCMAINWINDOW_H

#include <qwidget.h>
//Added by qt3to4:
#include <QCloseEvent>

class rcStatusBar;

const char* const cAppName = "Standin";

class rcMainWindow : public QWidget
{ 
    Q_OBJECT

public:
    rcMainWindow( QWidget* parent = 0, const char* name = 0 );
    ~rcMainWindow();

public slots:
    void settingChanged();
    
  protected:
    void closeEvent( QCloseEvent* );

  private:
    rcStatusBar*         mStatusBar;
};

#endif // UI_RCMAINWINDOW_H
