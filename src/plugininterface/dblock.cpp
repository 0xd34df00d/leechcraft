/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "dblock.h"
#include <stdexcept>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>

LeechCraft::Util::DBLock::DBLock (QSqlDatabase& database)
: Database_ (database)
, Good_ (false)
, Initialized_ (false)
{
}

LeechCraft::Util::DBLock::~DBLock ()
{
	if (!Initialized_)
		return;

	if (Good_ ? !Database_.commit () : !Database_.rollback ())
		DumpError (Database_.lastError ());
}

void LeechCraft::Util::DBLock::Init ()
{
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
		<< lastError.databaseText () << "|"
		<< lastError.driverText () << "|"
		<< lastError.type () << "|"
		<< lastError.number ();
}

void LeechCraft::Util::DBLock::DumpError (const QSqlQuery& lastQuery)
{
	qWarning () << lastQuery.lastQuery ();
	DumpError (lastQuery.lastError ());
}

