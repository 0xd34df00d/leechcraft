/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>
#include <interfaces/core/icoreproxyfwd.h>

class QTreeView;

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	class PlaylistDelegate : public QStyledItemDelegate
	{
		QTreeView *View_;

		const ICoreProxy_ptr Proxy_;
	public:
		PlaylistDelegate (QTreeView*, QObject*, const ICoreProxy_ptr&);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
		QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const;
	private:
		void PaintOneShot (const QVariant&,
				QStyleOptionViewItem&, QPainter*, QStyle*, bool) const;
		void PaintRules (const QVariant&, QStyleOptionViewItem&, QPainter*, QStyle*) const;
		void PaintTrack (QPainter*, QStyleOptionViewItem, const QModelIndex&) const;
		void PaintAlbum (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
	};
}
}
