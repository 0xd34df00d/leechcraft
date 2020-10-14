/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkunhidelistview.h"
#include <QQuickItem>
#include <QtDebug>
#include <util/util.h>
#include <util/qml/unhidelistmodel.h>
#include "quarkmanager.h"
#include "viewmanager.h"

namespace LC::SB2
{
	QuarkUnhideListView::QuarkUnhideListView (const QuarkComponents_t& components,
			ViewManager *viewMgr, ICoreProxy_ptr proxy, QWidget *parent)
	: Util::UnhideListViewBase (std::move (proxy),
			[&components, viewMgr] (QStandardItemModel *model)
			{
				for (const auto& comp : components)
				{
					std::unique_ptr<QuarkManager> qm;
					try
					{
						qm = std::make_unique<QuarkManager> (comp, viewMgr);
					}
					catch (const std::exception& e)
					{
						qWarning () << Q_FUNC_INFO
								<< "error creating manager for quark"
								<< comp->Url_;
						continue;
					}

					const auto& manifest = qm->GetManifest ();

					auto item = new QStandardItem;
					item->setData (manifest.GetID (), Util::UnhideListModel::Roles::ItemClass);
					item->setData (manifest.GetName (), Util::UnhideListModel::Roles::ItemName);
					item->setData (manifest.GetDescription (), Util::UnhideListModel::Roles::ItemDescription);
					item->setData (Util::GetAsBase64Src (manifest.GetIcon ().pixmap (32, 32).toImage ()),
							Util::UnhideListModel::Roles::ItemIcon);
					model->appendRow (item);
				}
			},
			parent)
	, ViewManager_ (viewMgr)
	{
		for (const auto& comp : components)
		{
			try
			{
				const auto& manager = std::make_shared<QuarkManager> (comp, ViewManager_);
				const auto& manifest = manager->GetManifest ();
				ID2Component_ [manifest.GetID ()] = { comp, manager };
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "skipping component"
						<< comp->Url_
						<< ":"
						<< e.what ();
			}
		}

		connect (rootObject (),
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SLOT (unhide (QString)),
				Qt::QueuedConnection);
	}

	void QuarkUnhideListView::unhide (const QString& itemClass)
	{
		if (!ID2Component_.contains (itemClass))
			return;

		const auto& info = ID2Component_.take (itemClass);
		ViewManager_->UnhideQuark (info.Comp_, info.Manager_);

		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->data (Util::UnhideListModel::Roles::ItemClass) == itemClass)
			{
				Model_->removeRow (i);
				break;
			}
	}
}
