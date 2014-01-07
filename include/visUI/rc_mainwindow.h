/******************************************************************************
 *  Copyright (c) 2002-2003 Reify Corp. All rights reserved.
 *
 *	$Id: rc_mainwindow.h 7179 2011-02-05 22:25:05Z arman $
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

class rcMainWindow : public QWidget
{ 
    Q_OBJECT

public:
    rcMainWindow( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel );
    ~rcMainWindow();

public slots:
    void settingChanged();
    
  protected:
    void closeEvent( QCloseEvent* );

  private:
    rcStatusBar*         mStatusBar;
};

#endif // UI_RCMAINWINDOW_H
