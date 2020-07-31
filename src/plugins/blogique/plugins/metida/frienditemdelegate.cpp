/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "frienditemdelegate.h"
#include <QApplication>
#include <QPainter>
#include <QTreeView>
#include <QtDebug>
#include "xmlsettingsmanager.h"
#include "friendsproxymodel.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	FriendItemDelegate::FriendItemDelegate (QSortFilterProxyModel *sortModel, QTreeView *parent)
	: QStyledItemDelegate (parent)
	, ColoringItems_ (true)
	, View_ (parent)
	, SortModel_ (sortModel)
	{
		XmlSettingsManager::Instance ().RegisterObject ("ColoringFriendsList",
				this, "handleColoringItemChanged");
		handleColoringItemChanged ();
	}

	void FriendItemDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto o = option;
		const QString& backgroundColor = SortModel_->mapToSource (index.sibling (index.row (), Columns::UserName))
				.data (ItemColorRoles::BackgroundColor).toString ();
		const QString& foregroundColor = SortModel_->mapToSource (index.sibling (index.row (), Columns::UserName))
				.data (ItemColorRoles::ForegroundColor).toString ();

		if (ColoringItems_)
		{
			if (!backgroundColor.isEmpty ())
				painter->fillRect (o.rect, QColor (backgroundColor));
			if (!foregroundColor.isEmpty ())
				o.palette.setColor (QPalette::Text, QColor (foregroundColor));
		}

		QStyledItemDelegate::paint (painter, o, index);
	}

	void FriendItemDelegate::handleColoringItemChanged ()
	{
		ColoringItems_ = XmlSettingsManager::Instance ()
				.Property ("ColoringFriendsList", true).toBool ();

		View_->viewport ()->update ();
		View_->update ();
	}

}
}
}
