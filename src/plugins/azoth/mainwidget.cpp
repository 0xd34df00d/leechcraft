/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "mainwidget.h"
#include <QToolBar>
#include <QMenu>
#include <QVBoxLayout>
#include "core.h"
#include "sortfilterproxymodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			MainWidget::MainWidget (QWidget *parent)
			: QWidget (parent)
			, UpperBar_ (new QToolBar)
			, MenuGeneral_ (new QMenu (tr ("General")))
			, MenuNewAccount_ (new QMenu (tr ("New account")))
			, ProxyModel_ (new SortFilterProxyModel ())
			{
				Ui_.setupUi (this);
				ProxyModel_->setSourceModel (Core::Instance ().GetCLModel ());
				Ui_.CLTree_->setModel (ProxyModel_);

				QVBoxLayout *lay = qobject_cast<QVBoxLayout*> (layout ());
				lay->insertWidget (0, UpperBar_);

				MenuGeneral_->addMenu (MenuNewAccount_);
				UpperBar_->addAction (MenuGeneral_->menuAction ());
			}

			void MainWidget::AddAccountCreators (const QList<QAction*>& actions)
			{
				MenuNewAccount_->addActions (actions);
			}

			void MainWidget::AddMUCJoiners (const QList<QAction*>& actions)
			{
				MenuGeneral_->addActions (actions);
			}

			void MainWidget::on_CLTree__activated (const QModelIndex& index)
			{
				if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
					return;

				Core::Instance ().OpenChat (ProxyModel_->mapToSource (index));
			}
		}
	}
}
