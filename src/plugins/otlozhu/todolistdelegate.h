/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStyledItemDelegate>

namespace LC
{
namespace Otlozhu
{
	class TodoListDelegate : public QStyledItemDelegate
	{
		QAbstractItemView *View_;
	public:
		TodoListDelegate (QAbstractItemView*);

		QWidget* createEditor (QWidget*,
				const QStyleOptionViewItem&, const QModelIndex&) const;
		void updateEditorGeometry (QWidget*,
				const QStyleOptionViewItem&, const QModelIndex&) const;

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const;
	};
}
}
