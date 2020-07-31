/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <interfaces/structures.h>
#include <util/sll/eitherfwd.h>

class QFile;

namespace LC
{
namespace NetStoreManager
{
struct StorageItem;

namespace GoogleDrive
{
	class Account;

	struct DriveItem
	{
		enum ItemLabel
		{
			ILNone = 0x00,
			ILFavorite = 0x01,
			ILHidden = 0x02,
			ILRemoved = 0x04,
			ILShared = 0x08,
			ILViewed = 0x10
		};
		Q_DECLARE_FLAGS (ItemLabels, ItemLabel)

		enum class Roles
		{
			Owner,
			Writer,
			Reader
		};

		enum AdditionalRole
		{
			ARNone = 0x00,
			ARCommenter = 0x01
		};
		Q_DECLARE_FLAGS (AdditionalRoles, AdditionalRole)

		enum class PermissionTypes
		{
			User,
			Group,
			Domain,
			Anyone
		};

		QString Id_;

		QString ParentId_;
		bool ParentIsRoot_;

		QString Name_;
		QString OriginalFileName_;
		QString Md5_;
		QString Mime_;
		QString FileExtension_;

		QMap<QUrl, QString> ExportLinks_;

		quint64 FileSize_;

		QStringList OwnerNames_;
		QString LastModifiedBy_;

		bool Editable_;
		bool WritersCanShare_;

		bool IsFolder_;

		ItemLabels Labels_;

		QDateTime CreateDate_;
		QDateTime ModifiedDate_;
		QDateTime LastViewedByMe_;

		QUrl DownloadUrl_;
		QUrl ShareUrl_;

		bool Shared_;

		Roles PermissionRole_;
		AdditionalRoles PermissionAdditionalRole_;
		PermissionTypes PermissionType_;

		DriveItem ()
		: ParentIsRoot_ (false)
		, FileSize_ (0)
		, Editable_ (false)
		, WritersCanShare_ (false)
		, IsFolder_ (false)
		, Shared_ (false)
		{
		}

		bool operator== (const DriveItem& item) const
		{
			return Id_ == item.Id_;
		}
	};

	StorageItem ToStorageItem (const DriveItem&);

	struct DriveChanges
	{
		QString Id_;
		QString FileId_;
		bool Deleted_;
		DriveItem FileResource_;
	};

	class DriveManager : public QObject
	{
		Q_OBJECT

		const QString DirectoryId_;

		Account *Account_;
		QQueue<std::function<void (const QString&)>> ApiCallQueue_;
		QQueue<std::function<void (const QUrl&)>> DownloadsQueue_;
		QHash<QNetworkReply*, QString> Reply2FilePath_;
		QHash<QNetworkReply*, QString> Reply2DownloadAccessToken_;
		bool SecondRequestIfNoItems_ = true;

	public:
		DriveManager (Account *acc, QObject *parent = 0);

		using ListingResult_t = Util::Either<QString, QList<StorageItem>>;
		QFuture<ListingResult_t> RefreshListing ();

		void RemoveEntry (const QByteArray& id);
		void MoveEntryToTrash (const QByteArray& id);
		void RestoreEntryFromTrash (const QByteArray& id);
		void Copy (const QByteArray& id, const QString& parentId);
		void Move (const QByteArray& id, const QString& parentId);

		using ShareResult_t = Util::Either<QString, QUrl>;
		QFuture<ShareResult_t> ShareEntry (const QString& id);
		void Upload (const QString& filePath,
				const QStringList& parentId = QStringList ());
		void Download (const QString& id, const QString& filePath,
				TaskParameters tp, bool open);

		void CreateDirectory (const QString& name,
				const QString& parentId = QString ());
		void Rename (const QString& id, const QString& newName);

		void RequestFileChanges (qlonglong startId, const QString& pageToken = QString ());
	private:
		void RequestFiles (const QString& key,
				QFutureInterface<ListingResult_t>, const QString& nextPageToken = {});
		void RequestSharingEntry (const QString& id, const QString& key, QFutureInterface<ShareResult_t>);
		void RequestEntryRemoving (const QString& id, const QString& key);
		void RequestMovingEntryToTrash (const QString& id, const QString& key);
		void RequestRestoreEntryFromTrash (const QString& id, const QString& key);
		void RequestUpload (const QString& filePath, const QString& parent,
				const QString& key);
		void RequestCreateDirectory (const QString& name,
				const QString& parentId, const QString& key);
		void RequestCopyItem (const QString& id,
				const QString& parentId, const QString& key);
		void RequestMoveItem (const QString& id,
				const QString& parentId, const QString& key);
		void GetFileChanges (qlonglong startId, const QString& pageToken, const QString& key);
		void RequestFileInfo (const QString& id, const QString& key);
		void RequestRenameItem (const QString& id,
				const QString& name,  const QString& key);

		void FindSyncableItems (const QStringList& paths,
				const QString& baseDir, const QList<DriveItem>& items);

		void RequestAccessToken ();
		QString ParseError (const QVariantMap& map);
	private slots:
		void handleAuthTokenRequestFinished ();
		void handleRequestEntryRemoving ();
		void handleRequestMovingEntryToTrash ();
		void handleRequestRestoreEntryFromTrash ();
		void handleUploadRequestFinished ();
		void handleUploadFinished ();
		void handleUploadProgress (qint64 uploaded, qint64 total);
		void handleUploadError (QNetworkReply::NetworkError error);
		void handleCreateDirectory ();
		void handleCopyItem ();
		void handleMoveItem ();
		void handleGetFileChanges ();
		void handleGetFileInfo ();
		void handleItemRenamed ();

	signals:
		void uploadProgress (qint64 sent, qint64 total, const QString& filePath);
		void uploadStatusChanged (const QString& status, const QString& filePath);
		void uploadError (const QString& str, const QString& filePath);
		void finished (const QString& id, const QString& path);

		void gotNewItem (const DriveItem& item);

		void gotChanges (const QList<DriveChanges>& changes);
	};
}
}
}
