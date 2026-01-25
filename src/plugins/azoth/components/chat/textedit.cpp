/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textedit.h"
#include <QKeyEvent>
#include <QShortcut>
#include <QtDebug>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include "../../xmlsettingsmanager.h"
#include "../../core.h"
#include "chattab.h"

namespace LC
{
namespace Azoth
{
	TextEdit::TextEdit (QWidget *parent)
	: QTextEdit (parent)
	{
		auto wordShortcut = new QShortcut ({},
				this,
				SLOT (deleteWord ()));
		auto bolShortcut = new QShortcut (QString ("Ctrl+U"),
				this,
				SLOT (deleteBOL ()));
		auto eolShortcut = new QShortcut (QString ("Ctrl+K"),
				this,
				SLOT (deleteEOL ()));

		auto sm = Core::Instance ().GetShortcutManager ();
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteWord",
				{ tr ("Delete the word before the cursor"), QKeySequence {}, {} },
				wordShortcut);
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteBOL",
				{
					tr ("Delete from cursor to the beginning of line"),
					bolShortcut->key (),
					{}
				},
				bolShortcut);
		sm->RegisterShortcut ("org.Azoth.TextEdit.DeleteEOL",
				{
					tr ("Delete from cursor to the end of line"),
					eolShortcut->key (),
					{}
				},
				eolShortcut);

		DefaultFont_ = font ();

		XmlSettingsManager::Instance ().RegisterObject ("MsgEditFontSize",
				this, "handleMsgFontSize");
		handleMsgFontSize ();

		XmlSettingsManager::Instance ().RegisterObject ("KPEnterAlias", this,
				[this] (bool allow) { AllowKeypadEnter_ = allow; });
		XmlSettingsManager::Instance ().RegisterObject ("SendOnModifier", this,
				[this] (const QString& modOption)
				{
					if (modOption == "CtrlEnter"_qs)
						SendMods_ = Qt::ControlModifier;
					else if (modOption == "ShiftEnter"_qs)
						SendMods_ = Qt::ShiftModifier;
					else
						SendMods_ = {};
				});
	}

	void TextEdit::keyPressEvent (QKeyEvent *event)
	{
		if (IsMessageSend (*event))
			emit messageSendRequested ();
		else if (event->key () == Qt::Key_Tab && event->modifiers () == Qt::NoModifier)
			emit keyTabPressed ();
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

	bool TextEdit::IsMessageSend (QKeyEvent& event) const
	{
		const auto key = event.key ();
		if (const bool isEnter = key == Qt::Key_Return || (AllowKeypadEnter_ && key == Qt::Key_Enter);
			!isEnter)
			return false;

		const auto modsOk = event.modifiers () == SendMods_ ||
				(AllowKeypadEnter_ && event.modifiers () == (SendMods_ | Qt::KeypadModifier));
		return modsOk;
	}

	void TextEdit::handleMsgFontSize ()
	{
		const auto size = XmlSettingsManager::Instance ().property ("MsgEditFontSize").toInt ();
		if (size == 5)	// keep 5 in sync with the settings
		{
			setFont (DefaultFont_);
			return;
		}

		auto newFont = font ();
		newFont.setPixelSize (size);
		setFont (newFont);
	}

	void TextEdit::deleteWord ()
	{
		auto c = textCursor ();

		const auto pos = c.position ();
		c.movePosition (QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
		if (pos == c.position ())
			c.movePosition (QTextCursor::PreviousWord, QTextCursor::KeepAnchor);

		c.removeSelectedText ();
	}

	void TextEdit::deleteBOL ()
	{
		auto c = textCursor ();
		c.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
		c.removeSelectedText ();
	}

	void TextEdit::deleteEOL ()
	{
		auto c = textCursor ();
		c.movePosition (QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
		c.removeSelectedText ();
	}
}
}
