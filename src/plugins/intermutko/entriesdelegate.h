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
namespace Intermutko
{
	class EntriesDelegate : public QStyledItemDelegate
	{
	public:
		QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
		void setEditorData (QWidget*, const QModelIndex&) const override;
		void setModelData (QWidget*, QAbstractItemModel*, const QModelIndex&) const override;
	};
}
}
