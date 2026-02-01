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
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include "../../xmlsettingsmanager.h"
#include "chattab.h"

namespace LC::Azoth
{
	TextEdit::TextEdit (QWidget *parent)
	: QTextEdit { parent }
	, DefaultFont_ { font () }
	{
		XmlSettingsManager::Instance ().RegisterObject ("MsgEditFontSize", this,
				[this] (int size)
				{
					if (size == 5)	// keep 5 in sync with the settings
					{
						setFont (DefaultFont_);
						return;
					}

					auto newFont = font ();
					newFont.setPixelSize (size);
					setFont (newFont);
				});

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

		connect (this,
				&QTextEdit::textChanged,
				this,
				&TextEdit::UpdateVisibleLines);
		XmlSettingsManager::Instance ().RegisterObject ("MinLinesHeight",
				this,
				[this] { ForceUpdateVisibleLines (); });
	}

	void TextEdit::ForceUpdateVisibleLines ()
	{
		PreviousHeight_ = 0;
		UpdateVisibleLines ();
	}

	void TextEdit::SetShortcutManager (Util::ShortcutManager& sm)
	{
		auto remove = [this] (auto selector)
		{
			return [this, selector]
			{
				auto c = textCursor ();
				selector (c);
				c.removeSelectedText ();
			};
		};
		auto wordShortcut = new QShortcut ({},
				this,
				remove ([] (QTextCursor& c) { c.select (QTextCursor::WordUnderCursor); }));
		auto bolShortcut = new QShortcut (QString ("Ctrl+U"),
				this,
				remove ([] (QTextCursor& c) { c.movePosition (QTextCursor::StartOfLine, QTextCursor::KeepAnchor); }));
		auto eolShortcut = new QShortcut (QString ("Ctrl+K"),
				this,
				remove ([] (QTextCursor& c) { c.movePosition (QTextCursor::EndOfLine, QTextCursor::KeepAnchor); }));

		sm.RegisterShortcut ("org.Azoth.TextEdit.DeleteWord",
				{ tr ("Delete the word before the cursor"), QKeySequence {}, {} },
				wordShortcut);
		sm.RegisterShortcut ("org.Azoth.TextEdit.DeleteBOL",
				{
					tr ("Delete from cursor to the beginning of line"),
					bolShortcut->key (),
					{}
				},
				bolShortcut);
		sm.RegisterShortcut ("org.Azoth.TextEdit.DeleteEOL",
				{
					tr ("Delete from cursor to the end of line"),
					eolShortcut->key (),
					{}
				},
				eolShortcut);
	}

	void TextEdit::keyPressEvent (QKeyEvent *event)
	{
		if (IsMessageSend (*event))
			emit messageSendRequested ();
		else if (event->modifiers () & Qt::ShiftModifier &&
				(event->key () == Qt::Key_PageUp ||
				 event->key () == Qt::Key_PageDown))
			emit scrollRequested (event->key () == Qt::Key_PageUp ? -1 : 1);
		else
			QTextEdit::keyPressEvent (event);
	}

	bool TextEdit::IsMessageSend (const QKeyEvent& event) const
	{
		const auto key = event.key ();
		if (const bool isEnter = key == Qt::Key_Return || (AllowKeypadEnter_ && key == Qt::Key_Enter);
			!isEnter)
			return false;

		const auto modsOk = event.modifiers () == SendMods_ ||
				(AllowKeypadEnter_ && event.modifiers () == (SendMods_ | Qt::KeypadModifier));
		return modsOk;
	}

	void TextEdit::UpdateVisibleLines ()
	{
		const int docHeight = document ()->size ().toSize ().height ();
		if (docHeight == PreviousHeight_)
			return;

		PreviousHeight_ = docHeight;
		const int minLines = XmlSettingsManager::Instance ().property ("MinLinesHeight").toInt ();
		const int minHeight = fontMetrics ().lineSpacing () * minLines + document ()->documentMargin () * 2;
		const int resHeight = std::min (parentWidget ()->height () / 3, std::max (docHeight, minHeight));
		setMaximumHeight (resHeight);
		setMinimumHeight (resHeight);
	}
}
