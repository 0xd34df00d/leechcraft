/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <boost/optional.hpp>
#include <QStringList>
#include <util/sll/either.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/ihistoryplugin.h>

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	struct UsersForAccount
	{
		QStringList Users_;
		QStringList NameCache_;
	};

	using LogItem = HistoryItem;
	using LogList_t = QList<LogItem>;

	using UsersForAccountResult_t = Util::Either<QString, UsersForAccount>;

	using ChatLogsResult_t = Util::Either<QString, LogList_t>;

	using SearchResult_t = Util::Either<QString, boost::optional<int>>;

	using DaysResult_t = Util::Either<QString, QList<int>>;
}
}
}
