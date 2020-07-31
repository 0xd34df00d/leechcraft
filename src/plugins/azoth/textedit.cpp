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
#include "chattab.h"
#include "xmlsettingsmanager.h"
#include "core.h"

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

		const bool kpEnter = XmlSettingsManager::Instance ()
				.property ("KPEnterAlias").toBool ();
		const bool sendMsgButton = event->key () == Qt::Key_Return ||
				(kpEnter && event->key () == Qt::Key_Enter);
		const bool modifiersOk = event->modifiers () == sendMod ||
				(kpEnter && event->modifiers () == (sendMod | Qt::KeypadModifier));
		if (sendMsgButton && modifiersOk)
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
		else if (event->modifiers () == Qt::ControlModifier &&
				(event->key () >= Qt::Key_0 && event->key () <= Qt::Key_9))
			event->ignore ();
		else
		{
			emit clearAvailableNicks ();
			QTextEdit::keyPressEvent (event);
		}
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
