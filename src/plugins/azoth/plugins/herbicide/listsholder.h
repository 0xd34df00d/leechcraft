/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QSet>
#include <QRegExp>

class QVariant;

namespace LC
{
namespace Azoth
{
class IAccount;

namespace Herbicide
{
	class ListsHolder
	{
		const std::function<QVariant (IAccount*, QByteArray)> PropGetter_;

		struct ListInfo
		{
			QSet<QRegExp> White_;
			QSet<QRegExp> Black_;
		};

		QHash<QByteArray, ListInfo> Acc2ListInfo_;
	public:
		ListsHolder (const std::function<QVariant (IAccount*, QByteArray)>&);

		QSet<QRegExp> GetWhitelist (IAccount*);
		QSet<QRegExp> GetBlacklist (IAccount*);

		void ReloadLists (IAccount*);
	private:
		ListInfo& PreloadList (IAccount*);
	};
}
}
}
