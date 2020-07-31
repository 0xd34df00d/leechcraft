/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QStringList>
#include <QtPlugin>
#include <QUrl>
#include <QMap>
#include <QDateTime>
#include <util/sll/eitherfwd.h>

class QStandardItem;

template<typename>
class QFuture;

namespace LC
{
namespace NetStoreManager
{
	enum ListingRole
	{
		ID = Qt::UserRole + 20,
		ParentID,
		InTrash,
		IsDirectory,
		ModifiedDate,
		Hash,
		HashType
	};

	enum ListingOp
	{
		NoneOp = 0x00,
		Delete = 0x01,
		TrashSupporting = 0x02,
		DirectorySupport = 0x04
	};

	Q_DECLARE_FLAGS (ListingOps, ListingOp);

	enum class HashAlgorithm
	{
		Md4,
		Md5,
		Sha1
	};

	struct StorageItem
	{
		QByteArray ID_;
		QByteArray ParentID_;

		QString Name_;
		QDateTime ModifyDate_;

		quint64 Size_ = 0;

		QByteArray Hash_;

		QUrl Url_;
		QUrl ShareUrl_;
		QMap<QUrl, QPair<QString, QString>> ExportLinks;

		bool Shared_ = false;

		bool IsDirectory_ = false;

		bool IsTrashed_ = false;

		HashAlgorithm HashType_;

		QString MimeType_;

		bool IsValid () const
		{
			return !ID_.isEmpty ();
		}
	};

	struct Change
	{
		QByteArray ID_;
		bool Deleted_;

		QByteArray ItemID_;
		StorageItem Item_;
	};

	class ISupportFileListings
	{
	public:
		virtual ~ISupportFileListings () {}

		struct InvalidItem {};
		struct UserCancelled {};

		virtual ListingOps GetListingOps () const = 0;
		virtual HashAlgorithm GetCheckSumAlgorithm () const = 0;

		using RefreshResult_t = Util::Either<QString, QList<StorageItem>>;
		virtual QFuture<RefreshResult_t> RefreshListing () = 0;

		virtual void RefreshChildren (const QByteArray& parentId) = 0;

		virtual void Delete (const QList<QByteArray>& ids, bool ask = true) = 0;
		virtual void MoveToTrash (const QList<QByteArray>& ids) = 0;
		virtual void RestoreFromTrash (const QList<QByteArray>& ids) = 0;
		virtual void Copy (const QList<QByteArray>& ids, const QByteArray& newParentId) = 0;
		virtual void Move (const QList<QByteArray>& ids, const QByteArray& newParentId) = 0;

		using RequestUrlError_t = std::variant<InvalidItem, UserCancelled, QString>;
		using RequestUrlResult_t = Util::Either<RequestUrlError_t, QUrl>;
		virtual QFuture<RequestUrlResult_t> RequestUrl (const QByteArray& id) = 0;

		virtual void CreateDirectory (const QString& name, const QByteArray& parentId) = 0;

		virtual void Rename (const QByteArray& id, const QString& newName) = 0;
		virtual void RequestChanges () = 0;
	protected:
		virtual void listingUpdated (const QByteArray& parentId) = 0;

		virtual void gotChanges (const QList<Change>& changes) = 0;
		virtual void gotNewItem (const StorageItem& item, const QByteArray& parentId) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::NetStoreManager::ISupportFileListings,
		"org.Deviant.LeechCraft.NetStoreManager.ISupportFileListings/1.0")
Q_DECLARE_OPERATORS_FOR_FLAGS (LC::NetStoreManager::ListingOps)

