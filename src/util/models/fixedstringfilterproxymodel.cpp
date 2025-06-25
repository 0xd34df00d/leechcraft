/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fixedstringfilterproxymodel.h"

namespace LC::Util
{
	FixedStringFilterProxyModel::FixedStringFilterProxyModel (Qt::CaseSensitivity cs, QObject *parent)
	: QSortFilterProxyModel { parent }
	, CaseSensitivity_ { cs }
	{
		setFilterCaseSensitivity (cs);
	}

	void FixedStringFilterProxyModel::SetCaseSensitivity (Qt::CaseSensitivity cs)
	{
		CaseSensitivity_ = cs;
		setFilterCaseSensitivity (cs);
	}

	void FixedStringFilterProxyModel::SetFilterString (const QString& filter)
	{
		FilterFixedString_ = filter;
		QSortFilterProxyModel::setFilterFixedString (filter);
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
}
