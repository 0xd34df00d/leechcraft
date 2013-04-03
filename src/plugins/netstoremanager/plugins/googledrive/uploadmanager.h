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
#include <QPointer>
#include <QStandardItem>
#include <interfaces/netstoremanager/istorageaccount.h>

class QNetworkAccessManager;

namespace LeechCraft
{
namespace NetStoreManager
{
namespace GoogleDrive
{
	class Account;
	class DriveManager;

	class UploadManager : public QObject
	{
		Q_OBJECT

		Account *Account_;
		QString FilePath_;
		QNetworkAccessManager *NAM_;
		QStringList ParentId_;
		QStringList Id_;
	public:
		UploadManager (const QString& path, UploadType ut,
				const QByteArray& parentId, Account *account,
				const QByteArray& id = QByteArray ());
	private:
		void InitiateUploadSession ();
		void InitiateUpdateSession ();

	private slots:
		void handleUploadProgress (qint64 sent, qint64 total, const QString& filePath);
		void handleStatusChanged (const QString& status, const QString& filePath);
		void handleError (const QString& error, const QString& filePath);
		void handleFinished (const QByteArray& id, const QString& filePath);
	signals:
		void uploadError (const QString& str, const QString& filePath);
		void uploadProgress (quint64 sent, quint64 total, const QString& filePath);
		void uploadStatusChanged (const QString& status, const QString& filePath);
		void finished (const QByteArray& id, const QString& filepath);
	};
}
}
}
