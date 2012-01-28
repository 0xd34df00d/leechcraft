/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#define PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#include <functional>
#include <QWidget>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QTextFrameFormat>

class QTextEdit;

namespace LeechCraft
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
