/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPointer>
#include <QStandardItem>
#include <interfaces/netstoremanager/istorageaccount.h>

class QNetworkAccessManager;

namespace LC
{
namespace NetStoreManager
{
namespace DBox
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
		void handleFinished (const QString& id, const QString& filePath);
	signals:
		void uploadError (const QString& str, const QString& filePath);
		void uploadProgress (quint64 sent, quint64 total, const QString& filePath);
		void uploadStatusChanged (const QString& status, const QString& filePath);
		void finished (const QByteArray& id, const QString& filepath);
	};
}
}
}
