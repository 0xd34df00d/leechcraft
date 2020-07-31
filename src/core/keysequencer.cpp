/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "keysequencer.h"
#include <QKeyEvent>
#include <QtDebug>

KeySequencer::KeySequencer (const QString& labelStr, QWidget *parent)
: QDialog (parent)
, LastCode_ (0)
{
	Ui_.setupUi (this);
	Ui_.DescLabel_->setText (labelStr);
	Ui_.DescLabel_->setFocus ();
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
		fixSingle (Qt::MetaModifier, Qt::META);
	}
}

void KeySequencer::keyPressEvent (QKeyEvent *event)
{
	event->accept ();

	LastCode_ = 0;
	FixCodes (LastCode_, event);

	const int key = event->key ();
	if (key != Qt::Key_Control &&
			key != Qt::Key_Alt &&
			key != Qt::Key_Meta &&
			key != Qt::Key_Shift)
		LastCode_ += key;

	const QKeySequence ts (LastCode_);

	Ui_.Shortcut_->setText (ts.toString (QKeySequence::NativeText));
}

void KeySequencer::keyReleaseEvent (QKeyEvent *event)
{
	event->accept ();

	auto testCode = LastCode_;
	for (auto code : { Qt::CTRL, Qt::ALT, Qt::SHIFT, Qt::META })
		testCode &= ~code;
	if (!testCode)
	{
		reject ();
		return;
	}

	Result_ = QKeySequence (LastCode_);

	accept ();
}

