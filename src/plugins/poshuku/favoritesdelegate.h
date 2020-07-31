/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_FAVORITESDELEGATE_H
#define PLUGINS_POSHUKU_FAVORITESDELEGATE_H
#include <memory>
#include <QItemDelegate>
#include <util/tags/tagscompleter.h>

namespace LC
{
namespace Poshuku
{
	class FavoritesDelegate : public QItemDelegate
	{
		Q_OBJECT

		mutable std::unique_ptr<Util::TagsCompleter> TagsCompleter_;
	public:
		FavoritesDelegate (QObject* = 0);

		QWidget* createEditor (QWidget*, const QStyleOptionViewItem&,
				const QModelIndex&) const;
		void setEditorData (QWidget*, const QModelIndex&) const;
		void setModelData (QWidget*, QAbstractItemModel*,
				const QModelIndex&) const;
		void updateEditorGeometry (QWidget*, const QStyleOptionViewItem&,
				const QModelIndex&) const;
	};
}
}

#endif
