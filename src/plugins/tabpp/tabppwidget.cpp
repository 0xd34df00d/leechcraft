/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "tabppwidget.h"
#include <QIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			TabPPWidget::TabPPWidget (const QString& title, QWidget *parent)
			: QDockWidget (title, parent)
			, FirstTime_ (true)
			{
				Ui_.setupUi (this);
				connect (GetActivatorAction (),
						SIGNAL (hovered ()),
						this,
						SLOT (handleActivatorHovered ()));

				Ui_.View_->setModel (Core::Instance ().GetModel ());

				connect (Core::Instance ().GetModel (),
						SIGNAL (rowsInserted (const QModelIndex&,
								int, int)),
						Ui_.View_,
						SLOT (expandAll ()));
				connect (Core::Instance ().GetModel (),
						SIGNAL (rowsRemoved (const QModelIndex&,
								int, int)),
						Ui_.View_,
						SLOT (expandAll ()));

				connect (Ui_.View_,
						SIGNAL (clicked (const QModelIndex&)),
						this,
						SLOT (selected (const QModelIndex&)));
			}

			QTreeView* TabPPWidget::GetView () const
			{
				return Ui_.View_;
			}

			QAction* TabPPWidget::GetActivatorAction () const
			{
				return toggleViewAction ();
			}

			void TabPPWidget::handleActivatorHovered ()
			{
				if (isVisible ())
					return;

				if (FirstTime_)
				{
					FirstTime_ = false;
					setFloating (true);
				}
				show ();
			}

			void TabPPWidget::selected (const QModelIndex& index)
			{
				Core::Instance ().HandleSelected (index);
			}
		};
	};
};

