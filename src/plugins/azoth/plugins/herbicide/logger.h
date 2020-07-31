/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <util/db/oral/oralfwd.h>

namespace LC
{
namespace Azoth
{
class ICLEntry;
class IAccount;

namespace Herbicide
{
	class Logger : public QObject
	{
	public:
		struct AccountRecord;
		struct EntryRecord;
		struct EventRecord;
	private:
		QSqlDatabase DB_;

		Util::oral::ObjectInfo_ptr<AccountRecord> AdaptedAccount_;
		Util::oral::ObjectInfo_ptr<EntryRecord> AdaptedEntry_;
		Util::oral::ObjectInfo_ptr<EventRecord> AdaptedEvent_;
	public:
		Logger (QObject* = nullptr);

		enum class Event
		{
			Granted,
			Denied,
			Challenged,
			Succeeded,
			Failed
		};

		void LogEvent (Event, const ICLEntry*, const QString& descr);
	private:
		int InsertAccount (const IAccount*);
		int InsertEntry (int, const ICLEntry*);
	};
}
}
}
