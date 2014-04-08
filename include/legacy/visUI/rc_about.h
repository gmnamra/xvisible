
#ifndef MOVIDAABOUT_H
#define MOVIDAABOUT_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPushButton;
class QLabel;
class QTextEdit;
class QFrame;


class rcAbout : public QDialog
{
    Q_OBJECT

public:
    rcAbout( QWidget* parent = 0, const char* name = 0 );
    ~rcAbout();

private:
    QFrame* mF_Box;
    QFrame* mLN_Line0;

    QGridLayout* rcAboutLayout;
    QGridLayout* mF_BoxLayout;

    QHBoxLayout* mHBL_LogoBig;

    QLabel* mL_Copy;
    QLabel* mL_GPL;
    QLabel* mL_Hosted;
    QLabel* mL_LogoBig;
    QLabel* mL_Title;
    QLabel* mL_Version;

    QPushButton* mB_Close;

    QTextEdit* mTE_Box;

    QVBoxLayout* mVBL_About;
    QVBoxLayout* mVBL_SF;

  //	URLLabel* mUL_Mail;
  //    URLLabel* mUL_Project;
  //    URLLabel* mUL_SFLogo;



protected slots:
    virtual void languageChange();
};

#endif // MOVIDAABOUT_H
