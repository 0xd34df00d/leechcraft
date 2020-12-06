/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "closingdb.h"
#include <QtDebug>

namespace LC::Util
{
	ClosingDB::ClosingDB (const QString& driver, const QString& connName)
	: DB_ { QSqlDatabase::contains (connName) ?
				QSqlDatabase::database (connName) :
				QSqlDatabase::addDatabase (driver, connName) }
	{
	}

	ClosingDB::~ClosingDB ()
	{
	}

	ClosingDB::operator const QSqlDatabase& () const
	{
		return DB_;
	}

	ClosingDB::operator QSqlDatabase& ()
	{
		return DB_;
	}

	const QSqlDatabase* ClosingDB::operator-> () const
	{
		return &DB_;
	}

	QSqlDatabase* ClosingDB::operator-> ()
	{
		return &DB_;
	}
}
