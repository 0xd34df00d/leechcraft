/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

class QInputDialog;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Account;

	class AuthManager : public QObject
	{
		Q_OBJECT

		const QString ClientId_;
		const QString ClientSecret_;
		const QString Scope_;
		const QString ResponseType_;
		const QString RedirectUri_;

		QInputDialog *InputDialog_;
		QMap<QInputDialog*, Account*> Dialog2Account_;
		QMap<QNetworkReply*, Account*> Reply2Account_;
	public:
		AuthManager (QObject *parent = 0);
		void Auth (Account *acc);
	private:
		void RequestAuthToken (const QString& code, Account *acc);

	private slots:
		void handleDialogFinished (int code);
		void handleRequestAuthTokenFinished ();

	signals:
		void gotEntity (LeechCraft::Entity e);
		void authSuccess (QObject *accObj);
	};
}
}
}

