/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <QObject>
#include <QQueue>
#include <QDateTime>
#include <QUrl>
#include <QStringList>
#include <QVariant>
#include <QNetworkReply>
#include <QFuture>
#include <util/sll/eitherfwd.h>
#include <interfaces/structures.h>

class QFile;

namespace LC
{
namespace NetStoreManager
{
struct StorageItem;

namespace DBox
{
	class Account;
	class ChunkIODevice;

	struct DBoxItem
	{
		QString Id_;
		QString ParentID_;
		QByteArray Revision_;

		QString Name_;
		quint64 FileSize_ = 0;
		bool IsFolder_ = false;
		bool IsDeleted_ = false;
		QString FolderHash_;
		QDateTime ModifiedDate_;
		QString MimeType_;


		bool operator== (const DBoxItem& item) const
		{
			return Id_ == item.Id_;
		}
	};

	StorageItem ToStorageItem (const DBoxItem&);

	enum class ShareType
	{
		Preview,
		Share
	};

	class DriveManager : public QObject
	{
		Q_OBJECT

		const QString DirectoryId_;

		Account *Account_;
		QQueue<std::function<void ()>> ApiCallQueue_;
		QHash<QNetworkReply*, QString> Reply2Id_;
		QHash<QNetworkReply*, QString> Reply2FilePath_;
		QHash<QNetworkReply*, QString> Reply2ParentId_;
		QHash<QNetworkReply*, quint64> Reply2Offset_;
		bool SecondRequestIfNoItems_;
	public:
		DriveManager (Account *acc, QObject *parent = 0);

		void RequestUserId ();

		using RefreshResult_t = Util::Either<QString, QList<StorageItem>>;
		QFuture<RefreshResult_t> RefreshListing (const QByteArray& parentId = {});

		using ShareResult_t = Util::Either<QString, QUrl>;
		QFuture<ShareResult_t> ShareEntry (const QString& id, ShareType type);

		void CreateDirectory (const QString& name,
				const QString& parentId = {});
		void RemoveEntry (const QByteArray& id);
		void Copy (const QByteArray& id, const QString& parentId);
		void Move (const QByteArray& id, const QString& parentId);

		void Upload (const QString& filePath,
				const QStringList& parentId = QStringList ());
		QUrl GenerateDownloadUrl (const QString& id) const;
	private:
		std::shared_ptr<void> MakeRunnerGuard ();
		void RequestAccountInfo ();
		void RequestFiles (const QByteArray& parentId, QFutureInterface<RefreshResult_t>);
		void RequestSharingEntry (const QString& id, ShareType type, QFutureInterface<ShareResult_t>);
		void RequestCreateDirectory (const QString& name, const QString& parentId);
		void RequestEntryRemoving (const QString& id);
		void RequestCopyItem (const QString& id, const QString& parentId);
		void RequestMoveItem (const QString& id, const QString& parentId);

		void RequestUpload (const QString& filePath, const QString& parent);
		void RequestChunkUpload (const QString& filePath, const QString& parent,
				const QString& uploadId = {}, quint64 offset = 0);

		void ParseError (const QVariantMap& map);
	private slots:
		void handleGotAccountInfo ();
		void handleCreateDirectory ();
		void handleRequestEntryRemoving ();
		void handleCopyItem ();
		void handleMoveItem ();
		void handleUploadFinished ();
		void handleChunkUploadFinished ();
		void handleUploadProgress (qint64 uploaded, qint64 total);
		void handleUploadError (QNetworkReply::NetworkError error);
	signals:
		void uploadProgress (qint64 sent, qint64 total, const QString& filePath);
		void uploadStatusChanged (const QString& status, const QString& filePath);
		void uploadError (const QString& str, const QString& filePath);
		void finished (const QString& id, const QString& path);

		void gotNewItem (const DBoxItem& item);
	};
}
}
}
