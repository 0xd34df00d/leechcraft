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

#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <QNetworkAccessManager>
#include <QTimer>

namespace LeechCraft
{
	class SslErrorsDialog;

	class NetworkAccessManager : public QNetworkAccessManager
	{
		Q_OBJECT

		std::auto_ptr<QTimer> CookieSaveTimer_;
		boost::shared_ptr<SslErrorsDialog> ErrorsDialog_;
	public:
		NetworkAccessManager (QObject* = 0);
		virtual ~NetworkAccessManager ();
	protected:
		QNetworkReply* createRequest (Operation,
				const QNetworkRequest&, QIODevice*);
	private:
		void DoCommonAuth (const QString&, QAuthenticator*);
	private slots:
		void handleAuthentication (QNetworkReply*, QAuthenticator*);
		void handleAuthentication (const QNetworkProxy&, QAuthenticator*);
		void handleSslErrors (QNetworkReply*, const QList<QSslError>&);
		void saveCookies () const;
		void handleFilterTrackingCookies ();
	signals:
		void requestCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);
		void error (const QString&) const;
	};
};

#endif

