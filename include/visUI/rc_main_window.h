#ifndef UI_RCMENUBAR_H
#define UI_RCMENUBAR_H

#include <QtGui>
#include <QList>
#include <QQueue>
#include <rc_model.h>


#include "rc_statusbar.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextEdit;
QT_END_NAMESPACE

class rcMainWindow : public QMainWindow
{
        Q_OBJECT

public:
    rcMainWindow( ); //QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );


public slots:
   void updateState( rcExperimentState state );
    void about();
    void help(); 
    void inputSource( int i );
    void settingChanged ();
    void updateAnalysisRect( const rcRect& rect );
    
    void doSave();
    void doExport();
    void doExportMovie();
    void doExportNativeMovie();
    void enableExportSmMatrix();
   
    void doOpen();
    void doOpenSettings();
    void importRecentMovie ();
    void reload_plotter (const CurveData* );

    
signals:


protected:
   void closeEvent( QCloseEvent* );       
private:
    void createActions ();
    void createMenus ();
    void createHelpBrowser ();
    void createToolBars ();
    void setCurrentFile(const QString &fileName);
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);
    void createDockWindows ();
    bool _new_matrix;
    
    rcStatusBar*         mStatusBar;
    QListWidget *plotlist;
    QMenuBar*          mMenuBar;
    
    QMenu* _fileMenu;
    QMenu* _analysisMenu;
    QMenu* _viewMenu;
    QMenu* _helpMenu;    
    QTextBrowser* _helpBrowser;
    rcExperimentState _lastState;
    QQueue<QDockWidget*> _docks;
    QScrollArea*    mScrollArea;
    QScrollArea*    mScrollTracks;
    
    enum { MaxRecentFiles = 5 };
    QAction *recentFileActs[MaxRecentFiles];
    
    QString curFile;
    
    // menu item ids for file menu

    QAction      *separatorAct;    
    QAction      *NewId;
    QAction      *OpenId;
    QAction      *OpenSettingsId;
    QAction      *SaveId;
    QAction      *ExportId;
    QAction      *MovieExportId;
    QAction      *MovieNativeExportId;
    QAction      *MatrixExportId;    
    QAction      *CloseId;
    QAction      *ImportImagesId;
    QAction      *ImportMovieId;
    QAction      *ImportSTKId;
    QAction      *QuitId;
    QAction      *HelpId;    
    QAction      *AboutId;        
    // menu item ids for analysis menu
    QAction      *AnalysisAnalyzeId;
    // menu item ids for capture menu
    QAction      *SaveMovieId;
    // menu item ids for view menu
    // OpenGL widget
    int         _viewTrackingGLId;
    // Cell info widget
    int         _viewCellInfoId;
};


#endif // UI_RCMENUBAR_H
