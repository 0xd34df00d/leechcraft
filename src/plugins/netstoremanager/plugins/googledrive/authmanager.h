/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/structures.h>

class QInputDialog;

namespace LC
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
		void authSuccess (QObject *accObj);
	};
}
}
}

