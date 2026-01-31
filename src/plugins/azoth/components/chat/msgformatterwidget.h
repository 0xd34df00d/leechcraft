/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextFrameFormat>

class QTextEdit;

namespace LC::Azoth
{
	class MsgFormatterWidget : public QWidget
	{
		Q_OBJECT

		QTextEdit& Edit_;

		const QTextCharFormat StockCharFormat_;
		const QTextBlockFormat StockBlockFormat_;
		const QTextFrameFormat StockFrameFormat_;

		QAction *FormatBold_;
		QAction *FormatItalic_;
		QAction *FormatUnderline_;
		QAction *FormatStrikeThrough_;

		QAction *AddEmoticon_;

		QWidget *SmilesTooltip_;

		bool HasCustomFormatting_ = false;
	public:
		explicit MsgFormatterWidget (QTextEdit&);

		void Clear ();
		std::optional<QString> GetNormalizedRichText () const;
	private:
		bool HasCustomFormatting () const;

		auto CharFormatter (auto setter);
		auto CharFormatter (auto setter, auto conv);

		void CharFormatActor (auto);
		void BlockFormatActor (auto);
		QTextCharFormat GetActualCharFormat () const;

		void HandleEmoPackChanged (const QString&);
		void UpdateState (const QTextCharFormat&);
	};
}
