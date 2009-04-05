#ifndef KEYSEQUENCER_H
#define KEYSEQUENCER_H
#include <QDialog>
#include "ui_keysequencer.h"

class KeySequencer : public QDialog
{
	Q_OBJECT

	Ui::KeySequencer Ui_;
	QKeySequence Result_;
public:
	KeySequencer (QWidget* = 0);
	QKeySequence GetResult () const;
protected:
	virtual void keyPressEvent (QKeyEvent*);
	virtual void keyReleaseEvent (QKeyEvent*);
};

#endif

