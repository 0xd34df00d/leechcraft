/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "listsholder.h"
#include <QVariant>
#include <util/sll/functor.h>
#include <interfaces/azoth/iaccount.h>

namespace LC
{
namespace Azoth
{
namespace Herbicide
{
	ListsHolder::ListsHolder (const std::function<QVariant (IAccount*, QByteArray)>& propGetter)
	: PropGetter_ { propGetter }
	{
		ReloadLists (nullptr);
	}

	namespace
	{
		QByteArray GetID (IAccount *acc)
		{
			return acc ?
					acc->GetAccountID () :
					QByteArray {};
		}
	}

	QSet<QRegExp> ListsHolder::GetWhitelist (IAccount *acc)
	{
		return PreloadList (acc).White_;
	}

	QSet<QRegExp> ListsHolder::GetBlacklist (IAccount *acc)
	{
		return PreloadList (acc).Black_;
	}

	namespace
	{
		QSet<QRegExp> GetRegexps (const QVariant& var)
		{
			QSet<QRegExp> result;

			const auto& strings = var.toStringList ();
			for (auto string : strings)
			{
				string = string.trimmed ();
				if (string.isEmpty ())
					continue;
				result << QRegExp (string);
			}

			return result;
		}
	}

	void ListsHolder::ReloadLists (IAccount *acc)
	{
		auto& listInfo = Acc2ListInfo_ [GetID (acc)];

		listInfo.White_ = GetRegexps (PropGetter_ (acc, "WhitelistRegexps"));
		listInfo.Black_ = GetRegexps (PropGetter_ (acc, "BlacklistRegexps"));
	}

	ListsHolder::ListInfo& ListsHolder::PreloadList (IAccount *acc)
	{
		const auto& id = GetID (acc);
		if (!Acc2ListInfo_.contains (id))
			ReloadLists (acc);

		return Acc2ListInfo_ [id];
	}
}
}
}
