#ifndef UI_rcCHOICE_RADIO_BUTTON_H
#define UI_rcCHOICE_RADIO_BUTTON_H

#include <qradiobutton.h>

#include <rc_setting.h>

class QCheckBox;

class rcChoiceRadioButton : public QRadioButton
{
	Q_OBJECT

public:
	rcChoiceRadioButton( QWidget* parent, const rcSettingChoice& choice )
		: QRadioButton( choice.getText() , parent )
	{
		_choiceValue = choice.getValue();
		connect( this , SIGNAL(clicked()) , this , SLOT(clickRelay()) );
	}

	int getChoiceValue( void ) { return _choiceValue; }

public slots:
	void clickRelay( void )
	{
		emit choiceSelected( _choiceValue );
	}

signals:
	void choiceSelected( int choiceValue );

private:
	int		_choiceValue;
};

#endif  // UI_rcCHOICE_RADIO_BUTTON_H
