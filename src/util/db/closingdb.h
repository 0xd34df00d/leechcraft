/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSqlDatabase>
#include "dbconfig.h"

namespace LC::Util
{
	class UTIL_DB_API ClosingDB final
	{
		QSqlDatabase DB_;
	public:
		ClosingDB (const QString& driver, const QString& connName);
		~ClosingDB ();

		operator const QSqlDatabase& () const;
		operator QSqlDatabase& ();

		const QSqlDatabase* operator-> () const;
		QSqlDatabase* operator-> ();
	};
}
