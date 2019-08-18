/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "dblock.h"
#include <stdexcept>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QMutexLocker>
#include <QVariant>
#include <QtDebug>

QSet<QString> LeechCraft::Util::DBLock::LockedBases_;
QMutex LeechCraft::Util::DBLock::LockedMutex_;

LeechCraft::Util::DBLock::DBLock (QSqlDatabase& database)
: Database_ (database)
{
}

LeechCraft::Util::DBLock::~DBLock ()
{
	if (!Initialized_)
		return;

	if (Good_ ? !Database_.commit () : !Database_.rollback ())
		DumpError (Database_.lastError ());

	{
		QMutexLocker locker (&LockedMutex_);
		LockedBases_.remove (Database_.connectionName ());
	}
}

void LeechCraft::Util::DBLock::Init ()
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

void LeechCraft::Util::DBLock::Good ()
{
	Good_ = true;
}

void LeechCraft::Util::DBLock::DumpError (const QSqlError& lastError)
{
	qWarning () << lastError.text () << "|"
		<< lastError.type ();
}

void LeechCraft::Util::DBLock::DumpError (const QSqlQuery& lastQuery)
{
	qWarning () << "query:" << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
	qWarning () << "bound values:" << lastQuery.boundValues ();
}

void LeechCraft::Util::DBLock::Execute (QSqlQuery& query)
{
	if (query.exec ())
		return;

	DumpError (query);
	throw std::runtime_error ("Query execution failed.");
}

