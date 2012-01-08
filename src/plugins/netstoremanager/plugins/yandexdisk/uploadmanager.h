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

#ifndef PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_UPLOADMANAGER_H
#define PLUGINS_NETSTOREMANAGER_PLUGINS_YANDEXDISK_UPLOADMANAGER_H
#include <QObject>
#include <QNetworkCookie>

class QNetworkRequest;
class QNetworkAccessManager;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	class Account;

	class UploadManager : public QObject
	{
		Q_OBJECT

		Account *A_;
		QNetworkAccessManager *Mgr_;

		QString Path_;
	public:
		UploadManager (const QString&, Account*);
	private slots:
		void handleGotCookies (const QList<QNetworkCookie>&);
		void handleGotStorage ();
		void handleUploadProgress (qint64, qint64);
		void handleUploadFinished ();
		void handleVerReqFinished ();
	signals:
		void finished ();
		void gotError (const QString&, const QString&);
		void statusChanged (const QString&, const QString&);
		void gotUploadURL (const QUrl&, const QString&);
		void uploadProgress (quint64, quint64, const QString&);
	};
}
}
}

#endif
