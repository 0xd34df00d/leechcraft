/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

namespace LeechCraft
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

		QSqlQuery AddMessage_;
		QSqlQuery RemoveMessage_;
		QSqlQuery UpdateMessage_;

		QSqlQuery AddExtendedMessageParams_;
		QSqlQuery AddNewCommentMessageParams_;
		QSqlQuery AddReceivedMessageParams_;
		QSqlQuery AddSentMessageParams_;

		QSqlQuery GetAllMessages_;
		QSqlQuery GetMessage_;

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
