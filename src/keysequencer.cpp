#include "keysequencer.h"
#include <QKeyEvent>
#include <QtDebug>

KeySequencer::KeySequencer (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
}

QKeySequence KeySequencer::GetResult () const
{
	return Result_;
}

void KeySequencer::keyPressEvent (QKeyEvent *event)
{
	int code = 0;
	if (event->modifiers () & Qt::ControlModifier)
		code += Qt::CTRL;
	if (event->modifiers () & Qt::AltModifier)
		code += Qt::ALT;
	if (event->modifiers () & Qt::ShiftModifier)
		code += Qt::SHIFT;

	int key = event->key ();
	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Shift)
		code += key;

	QKeySequence ts (code);

	Ui_.Shortcut_->setText (ts.toString ());
	QDialog::keyPressEvent (event);
}

void KeySequencer::keyReleaseEvent (QKeyEvent *event)
{
	int key = event->key ();
	if (key == Qt::Key_Control ||
			key == Qt::Key_Alt ||
			key == Qt::Key_Shift)
	{
		reject ();
		return;
	}

	int code = 0;
	if (event->modifiers () & Qt::ControlModifier)
		code += Qt::CTRL;
	if (event->modifiers () & Qt::AltModifier)
		code += Qt::ALT;
	if (event->modifiers () & Qt::ShiftModifier)
		code += Qt::SHIFT;

	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Shift)
		code += key;

	Result_ = QKeySequence (code);

	accept ();
	QDialog::keyReleaseEvent (event);
}

