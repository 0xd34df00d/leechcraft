/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tabunhidelistview.h"
#include <QQuickItem>
#include <QtDebug>
#include <util/util.h>
#include <util/qml/unhidelistmodel.h>

namespace LC
{
namespace SB2
{
	TabUnhideListView::TabUnhideListView (const QList<TabClassInfo>& tcs,
			ICoreProxy_ptr proxy, QWidget *parent)
	: UnhideListViewBase (proxy,
		[&tcs] (QStandardItemModel *model)
		{
			for (const auto& tc : tcs)
			{
				auto item = new QStandardItem;
				item->setData (tc.TabClass_, Util::UnhideListModel::Roles::ItemClass);
				item->setData (tc.VisibleName_, Util::UnhideListModel::Roles::ItemName);
				item->setData (tc.Description_, Util::UnhideListModel::Roles::ItemDescription);
				item->setData (Util::GetAsBase64Src (tc.Icon_.pixmap (32, 32).toImage ()),
						Util::UnhideListModel::Roles::ItemIcon);
				model->appendRow (item);
			}
		},
		parent)
	{
		connect (rootObject (),
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SLOT (unhide (QString)),
				Qt::QueuedConnection);
	}

	void TabUnhideListView::unhide (const QString& idStr)
	{
		const auto& id = idStr.toUtf8 ();
		emit unhideRequested (id);

		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->data (Util::UnhideListModel::Roles::ItemClass).toByteArray () == id)
			{
				Model_->removeRow (i);
				break;
			}

		if (!Model_->rowCount ())
			deleteLater ();
	}
}
}
