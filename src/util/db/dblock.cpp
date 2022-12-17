/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dblock.h"
#include <stdexcept>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMutexLocker>
#include <QVariant>
#include <QtDebug>

namespace LC::Util
{
	QSet<QString> DBLock::LockedBases_;

	QMutex DBLock::LockedMutex_;

	DBLock::DBLock (QSqlDatabase& database)
	: Database_ { database }
	{
	}

	DBLock::~DBLock ()
	{
		if (!Initialized_)
			return;

		if (Good_ ?
				!Database_.commit () :
				!Database_.rollback ())
			DumpError (Database_.lastError ());

		{
			QMutexLocker locker (&LockedMutex_);
			LockedBases_.remove (Database_.connectionName ());
		}
	}

	void DBLock::Init ()
	{
		{
			QMutexLocker locker (&LockedMutex_);
			const auto& conn = Database_.connectionName ();
			if (LockedBases_.contains (conn))
				return;
			LockedBases_ << conn;
		}

		if (!Database_.transaction ())
		{
			DumpError (Database_.lastError ());
			throw std::runtime_error ("Could not start transaction");
		}
		Initialized_ = true;
	}

	void DBLock::Good ()
	{
		Good_ = true;
	}

	void DBLock::DumpError (const QSqlError& lastError)
	{
		qCritical () << lastError.text () << "|"
				<< lastError.type ()
				<< lastError.nativeErrorCode ();
	}

	void DBLock::DumpError (const QSqlQuery& lastQuery)
	{
		qCritical () << "query:" << lastQuery.lastQuery ().simplified ();
		DumpError (lastQuery.lastError ());

		const auto& boundValues = lastQuery.boundValues ();
		qCritical () << "bound keys:" << boundValues.size () << boundValues.keys ();
		qCritical () << "bound values:" << boundValues;
	}

	void DBLock::Execute (QSqlQuery& query)
	{
		if (query.exec ())
			return;

		DumpError (query);
		throw std::runtime_error ("Query execution failed.");
	}
}
