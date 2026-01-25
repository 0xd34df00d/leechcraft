/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#define PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#include <functional>
#include <QWidget>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextFrameFormat>

class QTextEdit;

namespace LC
{
namespace Azoth
{
	class MsgFormatterWidget : public QWidget
	{
		Q_OBJECT

		QTextEdit *Edit_;

		const QTextCharFormat StockCharFormat_;
		const QTextBlockFormat StockBlockFormat_;
		const QTextFrameFormat StockFrameFormat_;

		QAction *FormatBold_;
		QAction *FormatItalic_;
		QAction *FormatUnderline_;
		QAction *FormatStrikeThrough_;

		QAction *FormatColor_;
		QAction *FormatFont_;

		QAction *FormatAlignLeft_;
		QAction *FormatAlignCenter_;
		QAction *FormatAlignRight_;
		QAction *FormatAlignJustify_;

		QAction *AddEmoticon_;

		bool HasCustomFormatting_;

		QWidget *SmilesTooltip_;
	public:
		MsgFormatterWidget (QTextEdit*, QWidget* = 0);

		bool HasCustomFormatting () const;
		void Clear ();
		QString GetNormalizedRichText () const;

		void HidePopups ();
	private:
		void CharFormatActor (std::function<void (QTextCharFormat*)>);
		void BlockFormatActor (std::function<void (QTextBlockFormat*)>);
		QTextCharFormat GetActualCharFormat () const;
	private slots:
		void handleBold ();
		void handleItalic ();
		void handleUnderline ();
		void handleStrikeThrough ();

		void handleTextColor ();
		void handleFont ();

		void handleParaAlignment ();

		void handleAddEmoticon ();
		void handleEmoPackChanged ();
		void insertEmoticon ();

		void checkCleared ();
		void updateState (const QTextCharFormat&);
	};
}
}

#endif
