/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_AUTHMANAGER_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_AUTHMANAGER_H
#include <QObject>
#include <QSet>
#include <QNetworkCookie>

class QNetworkAccessManager;
class QNetworkReply;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class Account;

	class AuthManager : public QObject
	{
		Q_OBJECT

		Account *A_;
		QNetworkAccessManager *Mgr_;
		QHash<QPair<QString, QString>, QList<QNetworkCookie>> Cookies_;
		QSet<QNetworkReply*> PendingReplies_;
		int RecurCount_;

		QString CurLogin_;
		QString CurPass_;
	public:
		AuthManager (Account*);

		void GetCookiesFor (const QString& login, const QString& pass, bool clear = false);
	private:
		void GetCookiesForImpl (const QString& login, const QString& pass,
				const QString& captcha);
	private slots:
		void handleFinished ();
	signals:
		void gotCookies (const QList<QNetworkCookie>&);
		void gotError (const QString&);
	};
}
}
}

#endif
