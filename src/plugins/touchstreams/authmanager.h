/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QDateTime>
#include <interfaces/core/icoreproxy.h>

class QUrl;

namespace LeechCraft
{
namespace Util
{
	class CustomCookieJar;
}

namespace TouchStreams
{
	class AuthManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;

		QNetworkAccessManager *AuthNAM_;
		Util::CustomCookieJar *Cookies_;

		QString Token_;
		QDateTime ReceivedAt_;
		qint32 ValidFor_;

		bool IsRequesting_;
	public:
		AuthManager (ICoreProxy_ptr, QObject* = 0);

		void GetAuthKey ();
		void Reauth ();
	private:
		void HandleError ();
		void RequestURL (const QUrl&);
		void RequestAuthKey ();
		bool CheckIsBlank (QUrl);
	private slots:
		void handleGotForm ();
		void handleFormFetchError ();
		void handleViewUrlChanged (const QUrl&);
	signals:
		void gotAuthKey (const QString&);
	};
}
}
