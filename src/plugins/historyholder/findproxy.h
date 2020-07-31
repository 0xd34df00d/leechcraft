/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <util/tags/tagsfiltermodel.h>
#include <interfaces/ifinder.h>

using QAbstractItemModel_ptr = std::shared_ptr<QAbstractItemModel>;

namespace LC
{
namespace HistoryHolder
{
	class FindProxy final : public Util::TagsFilterModel
						  , public IFindProxy
	{
		Q_OBJECT
		Q_INTERFACES (IFindProxy)

		const QAbstractItemModel_ptr Src_;
		const Request R_;
	public:
		FindProxy (const QAbstractItemModel_ptr&, const Request&);

		QAbstractItemModel* GetModel ();
		QByteArray GetUniqueSearchID () const;
		QStringList GetCategories () const;
	protected:
		QStringList GetTagsForIndex (int) const;
	};
}
}
