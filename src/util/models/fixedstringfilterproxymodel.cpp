/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fixedstringfilterproxymodel.h"
#include <ranges>

namespace LC::Util
{
	FixedStringFilterProxyModel::FixedStringFilterProxyModel (QObject *parent)
	: FixedStringFilterProxyModel { Qt::CaseInsensitive, parent }
	{
	}

	FixedStringFilterProxyModel::FixedStringFilterProxyModel (Qt::CaseSensitivity cs, QObject *parent)
	: QSortFilterProxyModel { parent }
	, CaseSensitivity_ { cs }
	{
		setRecursiveFilteringEnabled (IsRecursiveFiltering_);
	}

	void FixedStringFilterProxyModel::SetCaseSensitivity (Qt::CaseSensitivity cs)
	{
		CaseSensitivity_ = cs;
		invalidateFilter ();
	}

	Qt::CaseSensitivity FixedStringFilterProxyModel::GetCaseSensitivity () const
	{
		return CaseSensitivity_;
	}

	void FixedStringFilterProxyModel::SetFilterRoles (const QList<int>& roles)
	{
		FilterRoles_ = roles;
		invalidateFilter ();
	}

	QList<int> FixedStringFilterProxyModel::GetFilterRoles () const
	{
		return FilterRoles_;
	}

	void FixedStringFilterProxyModel::SetFilterColumns (const QList<int>& columns)
	{
		FilterColumns_ = columns;
		invalidateFilter ();
	}

	QList<int> FixedStringFilterProxyModel::GetFilterColumns () const
	{
		return FilterColumns_;
	}

	void FixedStringFilterProxyModel::SetRecursiveFiltering (bool enabled)
	{
		IsRecursiveFiltering_ = enabled;
		setRecursiveFilteringEnabled (enabled);
	}

	bool FixedStringFilterProxyModel::GetRecursiveFiltering () const
	{
		return IsRecursiveFiltering_;
	}

	void FixedStringFilterProxyModel::SetFilterString (const QString& filter)
	{
		FilterFixedString_ = filter;
		invalidateFilter ();
	}

	QString FixedStringFilterProxyModel::GetFilterString () const
	{
		return FilterFixedString_;
	}

	bool FixedStringFilterProxyModel::IsFilterSet () const
	{
		return !FilterFixedString_.isEmpty ();
	}

	bool FixedStringFilterProxyModel::IsMatch (const QString& text) const
	{
		return text.contains (FilterFixedString_, CaseSensitivity_);
	}

	bool FixedStringFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (!IsFilterSet ())
			return true;

		const auto checkColumn = [&, this] (int col)
		{
			const auto& idx = sourceModel ()->index (row, col, parent);
			return std::ranges::any_of (FilterRoles_,
					[this, &idx] (int role) { return IsMatch (idx.data (role).toString ()); });
		};

		return FilterColumns_.isEmpty () ?
				std::ranges::any_of (std::views::iota (0, sourceModel ()->columnCount (parent)), checkColumn) :
				std::ranges::any_of (FilterColumns_, checkColumn);
	}
}
