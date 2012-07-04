/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "frienditemdelegate.h"
#include <QApplication>
#include <QPainter>
#include <QTreeView>
#include <QtDebug>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	FriendItemDelegate::FriendItemDelegate (QTreeView *parent)
	: QStyledItemDelegate (parent)
	, ColoringItems_ (true)
	, View_ (parent)
	{
		XmlSettingsManager::Instance ().RegisterObject ("ColoringFriendsList",
				this, "handleColoringItemChanged");
		handleColoringItemChanged ();
	}

	void FriendItemDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QStyleOptionViewItemV4 o = option;
		const QRect& r = o.rect;

		QStyle *style = o.widget ?
				o.widget->style () :
				QApplication::style ();

		const QString& backgroundColor = index.data (ItemColorRoles::BackgroundColor)
				.toString ();
		const QString& foregroundColor = index.data (ItemColorRoles::ForegroundColor)
				.toString ();
		if (index.parent ().isValid () &&
				ColoringItems_)
		{
			if (!backgroundColor.isEmpty ())
				painter->fillRect (o.rect, QColor (backgroundColor));
			if (!foregroundColor.isEmpty ())
				o.palette.setColor (QPalette::Text, QColor (foregroundColor));
		}

		QStyledItemDelegate::paint (painter, o, index);

		painter->save ();
		if (!index.parent ().isValid ())
		{
			const int textWidth = o.fontMetrics.width (index.data ().value<QString> () + " ");
			const int rem = r.width () - textWidth;

			const QString& str = QString (" (%1) ")
					.arg (index.model ()->rowCount (index));

			if (o.state & QStyle::State_Selected)
				painter->setPen (o.palette.color (QPalette::HighlightedText));

			const QRect numRect (r.left () + textWidth - 1, r.top () + CPadding,
					rem - 1, r.height () - 2 * CPadding);
			painter->drawText (numRect, Qt::AlignVCenter | Qt::AlignLeft, str);

		}

		painter->restore ();
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
