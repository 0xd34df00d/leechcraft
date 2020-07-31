/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

namespace LC
{
namespace Util
{
	class DBLock;
}

namespace Azoth
{
namespace LastSeen
{
	struct EntryStats;

	class OnDiskStorage : public QObject
	{
	public:
		struct Record;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<Record> AdaptedRecord_;
	public:
		OnDiskStorage (QObject* = nullptr);

		std::optional<EntryStats> GetEntryStats (const QString&);
		void SetEntryStats (const QString&, const EntryStats&);

		Util::DBLock BeginTransaction ();
	};
}
}
}
