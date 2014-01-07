#ifndef UI_RCMENUBAR_H
#define UI_RCMENUBAR_H

#include <qmenubar.h>
//Added by qt3to4:
#include <Q3PopupMenu>

#include <rc_model.h>


// slot    void requestTrackingDisplayGL();



class Q3PopupMenu;
class Q3TextBrowser;

class rcMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    rcMenuBar( QWidget* parent=0, const char* name=0, Qt::WFlags f=0 );
    ~rcMenuBar();

public slots:
	void updateState( rcExperimentState state );
    void about();
    void help(); 
    void inputSource( int i );
    void updateAnalysisRect( const rcRect& rect );

    void doSave();
    void doExport();
    void doExportMovie();
    void doExportNativeMovie();
   
    void doOpen();
    void doOpenSettings();
    void requestCellInfoDisplay();
   
signals:


protected:

private:
    Q3PopupMenu* _fileMenu;
    Q3PopupMenu* _analysisMenu;
    Q3PopupMenu* _captureMenu;
    Q3PopupMenu* _viewMenu;
    Q3TextBrowser* _helpBrowser;
    rcExperimentState _lastState;
    
    // menu item ids for file menu
    int         _fileNewId;
    int         _fileOpenId;
    int         _fileOpenSettingsId;
    int         _fileSaveId;
    int         _fileExportId;
    int         _movieExportId;
    int         _movieNativeExportId;
    int         _fileCloseId;
    int         _fileImportImagesId;
    int         _dirImportImagesId;
    int         _fileImportMovieId;
    int         _fileImportSTKId;
    int         _fileQuitId;
    // menu item ids for analysis menu
    int         _analysisAnalyzeId;
    // menu item ids for capture menu
    int         _fileSaveMovieId;
    // menu item ids for view menu
    // OpenGL widget
    int         _viewTrackingGLId;
    // Cell info widget
    int         _viewCellInfoId;
};


#endif // UI_RCMENUBAR_H
