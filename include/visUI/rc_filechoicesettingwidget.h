#ifndef UI_RCFILECHOICESETTINGWIDGET_H
#define UI_RCFILECHOICESETTINGWIDGET_H

#include <rc_model.h>

#include "rc_settingwidget.h"

class QString;
class QLineEdit;
class QPushButton;

class rcFileChoiceSettingWidget : public rcSettingWidget
{
    Q_OBJECT

public:
    rcFileChoiceSettingWidget( QWidget* parent , const rcSettingInfo& setting );
    ~rcFileChoiceSettingWidget();

public slots:
	void browseRequest( void );
	void valueChanged( void );
	void settingChanged( void );
    void updateState( rcExperimentState state );
    // browse for single file
    void singleFileBrowse( void );
    // browse for multiple files
    void multiFileBrowse( void );
    // browse for directory
    void directoryBrowse( void );
    // browse for directory
    void directoryBrowse4Tiffs ( void );
    // Import images
    void imageImport( void );
    // import movie
    void movieImport( void );
    // Import tif directory
    void tifDirImport( void );
    // import molecular devices stk file
    void stkImport( void );
    // import recent movie, skip browsing.     
    void recentMovieFile ( const QString& fname);
    
protected:

private:

	QLineEdit*		mFilenameWidget;
    rcExperimentFileFormat mFormat;
};


#endif // UI_RCFILECHOICESETTINGWIDGET_H
