/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

KeySequencer::KeySequencer (const QString& labelStr, QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.DescLabel_->setText (labelStr);
}

QKeySequence KeySequencer::GetResult () const
{
	return Result_;
}

namespace
{
	void FixCodes (int& code, QKeyEvent *event)
	{
		auto fixSingle = [&code, event] (Qt::KeyboardModifier keyM, Qt::Modifier codeM)
		{
			if (event->modifiers () & keyM)
				code += codeM;
		};

		fixSingle (Qt::ControlModifier, Qt::CTRL);
		fixSingle (Qt::AltModifier, Qt::ALT);
		fixSingle (Qt::ShiftModifier, Qt::SHIFT);
	}
}

void KeySequencer::keyPressEvent (QKeyEvent *event)
{
	int code = 0;
	FixCodes (code, event);

	const int key = event->key ();
	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Shift)
		code += key;

	const QKeySequence ts (code);

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
	FixCodes (code, event);

	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Shift)
		code += key;

	Result_ = QKeySequence (code);

	accept ();
	QDialog::keyReleaseEvent (event);
}

