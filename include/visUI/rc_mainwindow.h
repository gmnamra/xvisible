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

#include <QMainWindow>
#include <QListWidget>
#include <rc_model.h>
#include "lpwidget.h"

#include "rc_imagecanvasgl.h"

class rcStatusBar;

class rcMainWindow : public QMainWindow
{ 
    Q_OBJECT

public:
    rcMainWindow(QWidget* parent = 0);
    ~rcMainWindow();

public slots:
    void settingChanged();
    void reload_plotter (const CurveData* );
    void reload_plotter2d (const CurveData2d* );    
    
  protected:
    void closeEvent( QCloseEvent* );

  private:
    void createDockWindows ();
    rcStatusBar*         mStatusBar;
    QListWidget *plotlist;

    
    QMenu *viewMenu;
    
};

#endif // UI_RCMAINWINDOW_H
