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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_FLGETTER_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_FLGETTER_H
#include <QObject>
#include <QNetworkCookie>
#include "flitem.h"

class QNetworkAccessManager;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class Account;

	class FLGetter : public QObject
	{
		Q_OBJECT

		Account *A_;
		QNetworkAccessManager *Mgr_;
	public:
		FLGetter (Account*);
	private slots:
		void handleGotCookies (const QList<QNetworkCookie>&);
		void handleGotList ();
	signals:
		void gotFiles (const QList<FLItem>&);
		void statusChanged (const QString&);
		void gotError (const QString&);

		void finished ();
	};
}
}
}

#endif
