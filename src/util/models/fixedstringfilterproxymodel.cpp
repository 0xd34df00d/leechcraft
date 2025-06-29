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
	{
		Filter_.setCaseSensitivity (cs);
		setRecursiveFilteringEnabled (true);
	}

	void FixedStringFilterProxyModel::SetCaseSensitivity (Qt::CaseSensitivity cs)
	{
		Filter_.setCaseSensitivity (cs);
		invalidateFilter ();
	}

	Qt::CaseSensitivity FixedStringFilterProxyModel::GetCaseSensitivity () const
	{
		return Filter_.caseSensitivity ();
	}

	void FixedStringFilterProxyModel::SetFilterRoles (const QList<int>& roles)
	{
		Roles_ = roles;
		invalidateFilter ();
	}

	QList<int> FixedStringFilterProxyModel::GetFilterRoles () const
	{
		return Roles_;
	}

	void FixedStringFilterProxyModel::SetFilterColumns (const QList<int>& columns)
	{
		Columns_ = columns;
		invalidateFilter ();
	}

	QList<int> FixedStringFilterProxyModel::GetFilterColumns () const
	{
		return Columns_;
	}

	void FixedStringFilterProxyModel::SetFilterString (const QString& filter)
	{
		Filter_.setPattern (filter);
		invalidateFilter ();
	}

	QString FixedStringFilterProxyModel::GetFilterString () const
	{
		return Filter_.pattern ();
	}

	bool FixedStringFilterProxyModel::IsFilterSet () const
	{
		return !Filter_.patternView ().isEmpty ();
	}

	bool FixedStringFilterProxyModel::IsMatch (const QString& text) const
	{
		return Filter_.indexIn (text) >= 0;
	}

	bool FixedStringFilterProxyModel::filterAcceptsRow (int row, const QModelIndex& parent) const
	{
		if (!IsFilterSet ())
			return true;

		const auto checkColumn = [&, this] (int col)
		{
			const auto& idx = sourceModel ()->index (row, col, parent);
			return std::ranges::any_of (Roles_,
					[this, &idx] (int role) { return IsMatch (idx.data (role).toString ()); });
		};

		return Columns_.isEmpty () ?
				std::ranges::any_of (std::views::iota (0, sourceModel ()->columnCount (parent)), checkColumn) :
				std::ranges::any_of (Columns_, checkColumn);
	}
}
