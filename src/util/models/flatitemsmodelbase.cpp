/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flatitemsmodelbase.h"

namespace LC::Util
{
	FlatItemsModelBase::FlatItemsModelBase (QStringList headers, QObject *parent)
	: QAbstractItemModel { parent }
	, Headers_ { std::move (headers) }
	{
	}

	int FlatItemsModelBase::columnCount (const QModelIndex& index) const
	{
		return index.isValid () ? 0 : Headers_.size ();
	}

	QVariant FlatItemsModelBase::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return {};

		return GetData (index.row (), index.column (), role);
	}

	QVariant FlatItemsModelBase::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
			return {};

		return Headers_.value (section);
	}

	QModelIndex FlatItemsModelBase::index (int row, int col, const QModelIndex& parent) const
	{
		if (parent.isValid () ||
				row >= GetItemsCount () ||
				col >= Headers_.size ())
			return {};

		return createIndex (row, col);
	}

	QModelIndex FlatItemsModelBase::parent (const QModelIndex&) const
	{
		return {};
	}

	int FlatItemsModelBase::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : GetItemsCount ();
	}
}
