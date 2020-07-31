/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "profiletypes.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LocalStorage : public QObject
	{
		Q_OBJECT

		QSqlDatabase MetidaDB_;

		QSqlQuery AddAccount_;
		QSqlQuery RemoveAccount_;

	public:
		explicit LocalStorage (const QByteArray& id, QObject *parent = 0);

		void AddAccount (const QByteArray& accounId);
		void RemoveAccount (const QByteArray& accounId);

	private:
		void CreateTables ();
		void PrepareQueries ();
	};
}
}
}
