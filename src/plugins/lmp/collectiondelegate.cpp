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

#include "collectiondelegate.h"
#include <QPainter>
#include <QApplication>
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	const int IconSize = 32;
	const int Padding = 2;

	CollectionDelegate::CollectionDelegate (QObject *parent)
	: QStyledItemDelegate (parent)
	, DefaultAlbum_ (QIcon::fromTheme ("media-optical").pixmap (64, 64))
	, PXCache_ (100000)
	{
	}

	void CollectionDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& optionOld, const QModelIndex& index) const
	{
		const int type = index.data (LocalCollection::Role::Node).toInt ();
		if (type != LocalCollection::NodeType::Album)
		{
			QStyledItemDelegate::paint (painter, optionOld, index);
			return;
		}

		const QStyleOptionViewItemV4 option = optionOld;
		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		painter->save ();

		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);
		const int maxIconHeight = option.rect.height () - Padding * 2;

		const QString& path = index.data (LocalCollection::Role::AlbumArt).value<QString> ();
		QPixmap *cached = PXCache_ [path];
		QPixmap px = cached ? *cached : QPixmap (path);
		const bool special = !px.isNull ();
		if (!special)
			px = DefaultAlbum_;
		px = px.scaled (maxIconHeight, maxIconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		if (!cached && special)
			PXCache_.insert (path, new QPixmap (px), px.size ().width () * px.size ().height ());

		painter->drawPixmap (option.rect.left () + Padding, option.rect.top () + Padding, px);

		const auto& text = index.data ().toString ();
		painter->setFont (option.font);
		painter->drawText (option.rect.adjusted (maxIconHeight + 2 * Padding, 0, 0, 0),
				Qt::AlignVCenter, text);

		painter->restore ();
	}

	QSize CollectionDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize result = QStyledItemDelegate::sizeHint (option, index);
		if (index.data (LocalCollection::Role::Node).toInt () == LocalCollection::NodeType::Album)
			result.setHeight (std::max (result.height (), IconSize));
		return result;
	}
}
}
