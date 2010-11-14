/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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

	const int key = event->key ();
	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Shift)
		code += key;

	const QKeySequence& ts (code);

	Ui_.Shortcut_->setText (ts.toString ());
	QDialog::keyPressEvent (event);
}

void KeySequencer::keyReleaseEvent (QKeyEvent *event)
{
	const int key = event->key ();
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

