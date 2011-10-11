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

#ifndef PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#define PLUGINS_POSHUKU_PROGRESSLINEEDIT_H
#include <boost/shared_ptr.hpp>
#include <QKeyEvent>
#include <QLineEdit>
#include <QToolButton>

class QModelIndex;
class QToolBar;
class QToolButton;

namespace LeechCraft
{
namespace Poshuku
{
	class ProgressLineEdit : public QLineEdit
	{
		Q_OBJECT

		bool IsCompleting_;
		QString PreviousUrl_;
		QToolButton *ClearButton_;
	public:
		ProgressLineEdit (QWidget* = 0);
		virtual ~ProgressLineEdit ();
		bool IsCompleting () const;
	protected:
		void keyPressEvent (QKeyEvent *);
		void resizeEvent (QResizeEvent *);
	private slots:
		void handleCompleterActivated ();
		void textChanged (const QString& text);
	};
}
}

#endif
