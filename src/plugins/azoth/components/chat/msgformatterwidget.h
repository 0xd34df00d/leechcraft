/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QWidget>
#include "interfaces/azoth/azothcommon.h"

class QTextEdit;
class QTextCharFormat;

namespace LC::Azoth
{
	class MsgFormatterWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aztoh::MsgFormatterWidget)

		const GlobalStrongestId EntryId_;
		QTextEdit& Edit_;

		QAction *Toggle_;

		QAction *FormatBold_;
		QAction *FormatItalic_;
		QAction *FormatUnderline_;
		QAction *FormatStrikeThrough_;

		QAction *AddEmoticon_;

		QWidget *SmilesTooltip_;
	public:
		explicit MsgFormatterWidget (const GlobalStrongestId& entryId, QTextEdit&);

		QAction& GetToggle () const;

		std::optional<QString> GetRichText () const;
	private:
		void SetupToggle ();

		auto CharFormatter (auto setter);
		auto CharFormatter (auto setter, auto conv);

		void CharFormatActor (auto);
		void BlockFormatActor (auto);
		QTextCharFormat GetActualCharFormat () const;

		void HandleEmoPackChanged (const QString&);
		void UpdateState (const QTextCharFormat&);
	};
}
