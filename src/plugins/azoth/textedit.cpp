/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "textedit.h"
#include <QKeyEvent>
#include "chattab.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
	TextEdit::TextEdit (QWidget *parent)
	: QTextEdit (parent)
	{
	}

	void TextEdit::keyPressEvent (QKeyEvent *event)
	{
		const QString& modOption = XmlSettingsManager::Instance ()
				.property ("SendOnModifier").toString ();
		Qt::KeyboardModifiers sendMod = Qt::NoModifier;
		if (modOption == "CtrlEnter")
			sendMod = Qt::ControlModifier;
		else if (modOption == "ShiftEnter")
			sendMod = Qt::ShiftModifier;

		bool sendMsgButton = event->key () == Qt::Key_Return ||
				event->key () == Qt::Key_Enter;
		if (sendMsgButton && event->modifiers () == sendMod)
			emit keyReturnPressed ();
		else if (event->key () == Qt::Key_Tab)
		{
			if (event->modifiers () == Qt::NoModifier)
				emit keyTabPressed ();
			else
				event->ignore ();
		}
		else if (event->modifiers () & Qt::ShiftModifier &&
				(event->key () == Qt::Key_PageUp ||
				 event->key () == Qt::Key_PageDown))
			emit scroll (event->key () == Qt::Key_PageUp ? -1 : 1);
		else
		{
			emit clearAvailableNicks ();
			QTextEdit::keyPressEvent (event);
		}
	}
}
}
