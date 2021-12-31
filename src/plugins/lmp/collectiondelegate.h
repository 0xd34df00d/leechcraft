/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>
#include <QPixmapCache>
#include <QCache>

namespace LC::LMP
{
	class CollectionDelegate : public QStyledItemDelegate
	{
		const QPixmap DefaultAlbum_;
		mutable QCache<QString, QPixmap> PXCache_;
	public:
		explicit CollectionDelegate (QObject* = nullptr);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
		QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const override;
	private:
		void PaintBorder (QPainter*, const QStyleOptionViewItem&) const;
		void PaintWPixmap (QPainter*, const QStyleOptionViewItem&, const QModelIndex&, const QPixmap&) const;
		void PaintOther (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
		void PaintAlbum (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
	};
}
