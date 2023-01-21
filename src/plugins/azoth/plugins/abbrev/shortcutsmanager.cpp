/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "shortcutsmanager.h"
#include <QTextEdit>
#include <QtDebug>
#include "abbrevsmanager.h"

namespace LC::Azoth::Abbrev
{
	ShortcutsManager::ShortcutsManager (AbbrevsManager *abbrevs, QObject *parent)
	: QObject { parent }
	, Abbrevs_ { abbrevs }
	{
	}

	void ShortcutsManager::HandleTab (QWidget *tab)
	{
		const auto shortcut = new QShortcut { tab };
		shortcut->setKey (Sequence_);
		connect (shortcut,
				&QShortcut::activated,
				this,
				[this, tab] { HandleActivated (tab); });
		Tab2SC_ [tab] = shortcut;

		connect (tab,
				&QObject::destroyed,
				this,
				[this, tab] { Tab2SC_.remove (tab); });
	}

	QMap<QByteArray, ActionInfo> ShortcutsManager::GetActionInfo () const
	{
		return
		{
			{
				"org.LeechCraft.Azoth.Abbrev.Expand",
				{
					tr ("Expand abbreviations in current message edit text."),
					QKeySequence {},
					{}
				}
			},
		};
	}

	void ShortcutsManager::SetShortcut (const QByteArray&, const QKeySequences_t& seqs)
	{
		Sequence_ = seqs.value (0);

		for (const auto sc : Tab2SC_)
			sc->setKey (Sequence_);
	}

	void ShortcutsManager::HandleActivated (QWidget *tab)
	{
		QTextEdit *edit = nullptr;
		QMetaObject::invokeMethod (tab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get message edit"
					<< tab;
			return;
		}

		const auto& text = edit->toPlainText ();
		const auto& processed = Abbrevs_->Process (text);
		if (text == processed)
			return;

		edit->setPlainText (processed);
		edit->moveCursor (QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
	}
}
