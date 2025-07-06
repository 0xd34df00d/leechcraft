/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "findproxy.h"
#include <interfaces/structures.h>

namespace LC
{
namespace HistoryHolder
{
	FindProxy::FindProxy (const QAbstractItemModel_ptr& model, const Request& r)
	: Src_ { model }
	, R_ (r)
	{
		setSourceModel (model.get ());

		const auto cs = r.CaseSensitive_ ? Qt::CaseSensitive : Qt::CaseInsensitive;

		switch (r.Type_)
		{
		case Request::RTWildcard:
			setFilterWildcard (r.String_);
			setFilterCaseSensitivity (cs);
			break;
		case Request::RTRegexp:
			setFilterRegularExpression (r.String_);
			setFilterCaseSensitivity (cs);
			break;
		default:
			SetFilterString (r.String_);
			SetCaseSensitivity (cs);
			SetTagsMode (r.Type_ == Request::RTTag);
			break;
		}
	}

	QAbstractItemModel* FindProxy::GetModel ()
	{
		return this;
	}

	QByteArray FindProxy::GetUniqueSearchID () const
	{
		return QString { "org.LeechCraft.HistoryHolder.%1.%2" }
				.arg (R_.Type_)
				.arg (R_.String_)
				.toUtf8 ();
	}

	QStringList FindProxy::GetCategories () const
	{
		return { R_.Category_ };
	}

	QStringList FindProxy::GetTagsForIndex (int row) const
	{
		return sourceModel ()->index (row, 0).data (RoleTags).toStringList ();
	}
}
}
