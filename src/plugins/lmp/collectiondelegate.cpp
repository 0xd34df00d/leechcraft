/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectiondelegate.h"
#include <QPainter>
#include <QApplication>
#include "localcollectionmodel.h"

namespace LC
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
		const int type = index.data (LocalCollectionModel::Role::Node).toInt ();

		auto option = optionOld;

		auto& pal = option.palette;
		if (!(option.features & QStyleOptionViewItem::Alternate))
		{
			QLinearGradient grad (0, 0, option.rect.width (), 0);
			grad.setColorAt (0, pal.color (QPalette::Window).darker (105));
			grad.setColorAt (0.5, pal.color (QPalette::Window).darker (120));
			grad.setColorAt (1, pal.color (QPalette::Window).darker (105));
			option.backgroundBrush = QBrush (grad);
		}

		if (type != LocalCollectionModel::NodeType::Album)
			PaintOther (painter, option, index);
		else
			PaintAlbum (painter, option, index);
	}

	QSize CollectionDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize result = QStyledItemDelegate::sizeHint (option, index);
		if (index.data (LocalCollectionModel::Role::Node).toInt () == LocalCollectionModel::NodeType::Album)
			result.setHeight (std::max (result.height (), IconSize));
		return result;
	}

	void CollectionDelegate::PaintBorder (QPainter *painter, const QStyleOptionViewItem& option) const
	{
		const auto& pal = option.palette;
		QLinearGradient grad (0, 0, option.rect.width (), 0);
		grad.setColorAt (0, pal.color (QPalette::Button));
		grad.setColorAt (0.5, pal.color (QPalette::Dark));
		grad.setColorAt (1, pal.color (QPalette::Button));

		painter->setPen ({ grad, 1 });
		painter->drawLine (option.rect.bottomLeft (), option.rect.bottomRight ());
	}

	void CollectionDelegate::PaintWPixmap (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index, const QPixmap& px) const
	{
		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		painter->save ();

		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

		const int maxIconHeight = option.rect.height () - Padding * 2;
		const bool hasIcon = !px.isNull ();
		if (hasIcon)
		{
			const auto horShift = (maxIconHeight - px.size ().width ()) / 2;
			const auto vertShift = (option.rect.height () - px.size ().height ()) / 2;
			painter->drawPixmap (option.rect.left () + horShift,
					option.rect.top () + vertShift,
					px);
		}

		const auto& text = index.data ().toString ();
		if (option.state & QStyle::State_Selected)
			painter->setPen (option.palette.color (QPalette::HighlightedText));
		painter->setFont (option.font);
		painter->drawText (option.rect.adjusted (hasIcon ? maxIconHeight + 2 * Padding : 0, 0, 0, 0),
				Qt::AlignVCenter, text);

		PaintBorder (painter, option);

		painter->restore ();
	}

	void CollectionDelegate::PaintOther (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const int maxIconHeight = option.rect.height () - Padding * 2;
		const auto& icon = index.data (Qt::DecorationRole).value<QIcon> ();
		PaintWPixmap (painter, option, index,
				icon.pixmap ({ maxIconHeight, maxIconHeight }));
	}

	void CollectionDelegate::PaintAlbum (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const QString& path = index.data (LocalCollectionModel::Role::AlbumArt).value<QString> ();
		QPixmap *cached = PXCache_ [path];
		QPixmap px = cached ? *cached : QPixmap (path);
		const bool special = !px.isNull ();
		if (!special)
			px = DefaultAlbum_;

		const int maxIconHeight = option.rect.height () - Padding * 2;
		px = px.scaled (maxIconHeight, maxIconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		if (!cached && special)
			PXCache_.insert (path, new QPixmap (px), px.size ().width () * px.size ().height ());

		PaintWPixmap (painter, option, index, px);
	}
}
}
