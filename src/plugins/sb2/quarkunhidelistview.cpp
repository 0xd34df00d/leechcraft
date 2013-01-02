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

#include "quarkunhidelistview.h"
#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/util.h>
#include "unhidelistmodel.h"
#include "quarkmanager.h"
#include "viewmanager.h"

namespace LeechCraft
{
namespace SB2
{
	QuarkUnhideListView::QuarkUnhideListView (const QList<QuarkComponent>& components,
			ViewManager *viewMgr, ICoreProxy_ptr proxy, QWidget *parent)
	: UnhideListViewBase (proxy, parent)
	, ViewManager_ (viewMgr)
	{
		BeginModelFill ();
		for (const auto& comp : components)
		{
			QuarkManager_ptr manager;
			try
			{
				manager.reset (new QuarkManager (comp, viewMgr, proxy));
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error creating manager for quark"
						<< comp.Url_;
				continue;
			}

			auto item = new QStandardItem;
			item->setData (manager->GetID (), UnhideListModel::Roles::ItemClass);
			item->setData (manager->GetName (), UnhideListModel::Roles::ItemName);
			item->setData (QString (), UnhideListModel::Roles::ItemDescription);
			item->setData (Util::GetAsBase64Src (manager->GetIcon ().pixmap (32, 32).toImage ()),
					UnhideListModel::Roles::ItemIcon);
			Model_->appendRow (item);

			ID2Component_ [manager->GetID ()] = { comp, manager };
		}
		EndModelFill ();

		connect (rootObject (),
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SLOT (unhide (QString)),
				Qt::QueuedConnection);
	}

	void QuarkUnhideListView::unhide (const QString& itemClass)
	{
		const auto& info = ID2Component_.take (itemClass);
		ViewManager_->UnhideQuark (info.Comp_, info.Manager_);

		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->data (UnhideListModel::Roles::ItemClass) == itemClass)
			{
				Model_->removeRow (i);
				break;
			}
	}
}
}
