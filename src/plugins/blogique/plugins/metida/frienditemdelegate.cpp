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

namespace LC
{
namespace Blogique
{
namespace Metida
{
	FriendItemDelegate::FriendItemDelegate (QTreeView *parent)
	: QStyledItemDelegate (parent)
	, View_ (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("ColoringFriendsList",
				this,
				[this] (bool enabled)
				{
					ColoringItems_ = enabled;

					View_->viewport ()->update ();
					View_->update ();
				});
	}

	void FriendItemDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto o = option;
		if (ColoringItems_)
		{
			const auto& nameIdx = index.siblingAtColumn (Columns::UserName);
			const auto& backgroundColor = nameIdx.data (ItemColorRoles::BackgroundColor).toString ();
			const auto& foregroundColor = nameIdx.data (ItemColorRoles::ForegroundColor).toString ();

			if (!backgroundColor.isEmpty ())
				painter->fillRect (o.rect, QColor (backgroundColor));
			if (!foregroundColor.isEmpty ())
				o.palette.setColor (QPalette::Text, QColor (foregroundColor));
		}

		QStyledItemDelegate::paint (painter, o, index);
	}
}
}
}
