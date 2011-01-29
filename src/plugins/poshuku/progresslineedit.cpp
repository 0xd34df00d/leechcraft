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
#include "core.h"

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
				QCompleter *completer = new QCompleter (this);
				completer->setModel (Core::Instance ().GetURLCompletionModel ());
				completer->setCompletionRole (URLCompletionModel::RoleURL);
				completer->setCompletionMode (QCompleter::UnfilteredPopupCompletion);
				setCompleter (completer);

				connect (completer,
						SIGNAL (activated (const QModelIndex&)),
						this,
						SLOT (handleCompleterActivated ()));

				connect (this,
						SIGNAL (textEdited (const QString&)),
						Core::Instance ().GetURLCompletionModel (),
						SLOT (setBase (const QString&)));
			}

			ProgressLineEdit::~ProgressLineEdit ()
			{
			}

			bool ProgressLineEdit::IsCompleting () const
			{
				return IsCompleting_;
			}

			void ProgressLineEdit::handleCompleterActivated ()
			{
				emit returnPressed ();
			}
		};
	};
};

