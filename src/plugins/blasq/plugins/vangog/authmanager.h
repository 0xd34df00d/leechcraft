/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
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
namespace Blasq
{
namespace Vangog
{
	class PicasaAccount;

	class AuthManager : public QObject
	{
		Q_OBJECT

		QInputDialog *InputDialog_;
		QMap<QInputDialog*, PicasaAccount*> Dialog2Account_;
		QMap<QNetworkReply*, PicasaAccount*> Reply2Account_;

		ICoreProxy_ptr Proxy_;

	public:
		AuthManager (ICoreProxy_ptr proxy, QObject *parent = 0);
		void Auth (PicasaAccount *acc);
	private:
		void RequestAuthToken (const QString& code, PicasaAccount *acc);

	private slots:
		void handleDialogFinished (int code);
		void handleRequestAuthTokenFinished ();

	signals:
		void authSuccess (QObject *accObj);
	};
}
}
}

