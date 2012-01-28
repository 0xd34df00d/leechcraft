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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_ACTORBASE_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_ACTORBASE_H
#include <QObject>
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

	class ActorBase : public QObject
	{
		Q_OBJECT
	protected:
		Account *A_;
		QNetworkAccessManager *Mgr_;

		ActorBase (Account*);

		virtual QNetworkReply* MakeRequest () = 0;
		virtual void HandleReply (QNetworkReply*) = 0;
	protected slots:
		virtual void handleGotCookies (const QList<QNetworkCookie>&);
		virtual void handleReplyFinished ();
	signals:
		void statusChanged (const QString&);
		void gotError (const QString&);

		void finished ();
	};
}
}
}

#endif
