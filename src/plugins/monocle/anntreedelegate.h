/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStyledItemDelegate>

class QTreeView;
class QTextDocument;

namespace LC
{
namespace Monocle
{
	class AnnTreeDelegate : public QStyledItemDelegate
	{
		QTreeView * const View_;
		int PrevWidth_ = -1;
	public:
		AnnTreeDelegate (QTreeView*, QObject*);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
		QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const;

		bool eventFilter (QObject*, QEvent*);
	private:
		std::shared_ptr<QTextDocument> GetDoc (const QModelIndex&, int) const;
		QString GetText (const QModelIndex&) const;
	};
}
}
