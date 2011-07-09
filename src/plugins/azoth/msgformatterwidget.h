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

#ifndef PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#define PLUGINS_AZOTH_MSGFORMATTERWIDGET_H
#include <boost/function.hpp>
#include <QWidget>

class QTextCharFormat;
class QTextEdit;

namespace LeechCraft
{
namespace Azoth
{
	class MsgFormatterWidget : public QWidget
	{
		Q_OBJECT
		
		QTextEdit *Edit_;
		
		QAction *FormatBold_;
		QAction *FormatItalic_;
		QAction *FormatUnderline_;
		QAction *FormatStrikeThrough_;
	public:
		MsgFormatterWidget (QTextEdit*, QWidget* = 0);
	private:
		void CharFormatActor (boost::function<void (QTextCharFormat*)>);
	private slots:
		void handleBold ();
		void handleItalic ();
		void handleUnderline ();
		void handleStrikeThrough ();
		
		void updateState (const QTextCharFormat&);
	};
}
}

#endif
