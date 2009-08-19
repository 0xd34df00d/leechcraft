/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "progresslineedit.h"
#include <QTimer>
#include <QCompleter>
#include <QAbstractItemView>
#include <QToolBar>
#include <QtDebug>
#include "urlcompletionmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			ProgressLineEdit::ProgressLineEdit (QWidget *parent)
			: QLineEdit (parent)
			, IsCompleting_ (false)
			{
			}
			
			ProgressLineEdit::~ProgressLineEdit ()
			{
			}
			
			bool ProgressLineEdit::IsCompleting () const
			{
				return IsCompleting_;
			}
			
			void ProgressLineEdit::AddAction (QAction *act)
			{
				addAction (act);
			}
			
			void ProgressLineEdit::focusInEvent (QFocusEvent *e)
			{
				QLineEdit::focusInEvent (e);
			
				disconnect (completer (),
						0,
						this,
						0);
				connect (completer (),
						SIGNAL (activated (const QModelIndex&)),
						this,
						SLOT (handleActivated (const QModelIndex&)));
				connect (completer (),
						SIGNAL (highlighted (const QModelIndex&)),
						this,
						SLOT (handleHighlighted (const QModelIndex&)));
			}
			
			void ProgressLineEdit::keyPressEvent (QKeyEvent *e)
			{
				QLineEdit::keyPressEvent (e);
				IsCompleting_ = false;
			}
			
			void ProgressLineEdit::handleActivated (const QModelIndex& index)
			{
				QString url = qobject_cast<URLCompletionModel*> (completer ()->
						model ())->index (index.row (), 0)
					.data (URLCompletionModel::RoleURL).toString ();
			
				setText (url);
				IsCompleting_ = false;
				emit returnPressed ();
			}
			
			void ProgressLineEdit::handleHighlighted (const QModelIndex& index)
			{
				IsCompleting_ = index.isValid ();
			
				QString url = qobject_cast<URLCompletionModel*> (completer ()->
						model ())->index (index.row (), 0)
					.data (URLCompletionModel::RoleURL).toString ();
			
				setText (url);
			}
		};
	};
};

