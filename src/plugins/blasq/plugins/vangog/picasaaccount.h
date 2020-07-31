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
#include <QSet>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/blasq/iaccount.h>
#include <interfaces/blasq/isupportdeletes.h>
#include <interfaces/blasq/isupportuploads.h>
#include "picasamanager.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace Blasq
{
namespace Vangog
{
	class UploadManager;
	class PicasaService;

	class PicasaAccount : public QObject
						, public IAccount
						, public ISupportDeletes
						, public ISupportUploads
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IAccount LC::Blasq::ISupportDeletes
				LC::Blasq::ISupportUploads)

		QString Name_;
		PicasaService * const Service_;
		const ICoreProxy_ptr Proxy_;
		QByteArray ID_;
		QString Login_;
		QString AccessToken_;
		QDateTime AccessTokenExpireDate_;
		QString RefreshToken_;
		bool Ready_;

		PicasaManager *PicasaManager_;

		QStandardItemModel * const CollectionsModel_;
		QStandardItem *AllPhotosItem_;
		QHash<QByteArray, QStandardItem*> AlbumId2AlbumItem_;
		QHash<QByteArray, QSet<QByteArray>> AlbumID2PhotosSet_;
		QHash<QStandardItem*, QByteArray> Item2PhotoId_;
		QHash<QByteArray, QModelIndex> DeletedPhotoId2Index_;

		Util::QueueManager *RequestQueue_;
		UploadManager *UploadManager_;

	public:
		enum PicasaRole
		{
			AlbumId = CollectionRole::CollectionRoleMax + 1
		};

		PicasaAccount (const QString& name, PicasaService *service,
				ICoreProxy_ptr proxy, const QString& login,
				const QByteArray& id = QByteArray ());

		ICoreProxy_ptr GetProxy () const;

		void Release ();
		QByteArray Serialize () const;
		static PicasaAccount* Deserialize (const QByteArray& data,
				PicasaService *service, ICoreProxy_ptr proxy);

		void Schedule (std::function<void (QString)> func);

		QObject* GetQObject () override;
		IService* GetService () const override;
		QString GetName () const override;
		QByteArray GetID () const override;

		QString GetLogin () const;
		QString GetAccessToken () const;
		QDateTime GetAccessTokenExpireDate () const;
		void SetRefreshToken (const QString& token);
		QString GetRefreshToken () const;

		QAbstractItemModel* GetCollectionsModel () const override;

		void UpdateCollections () override;

		void Delete (const QModelIndex& index) override;
		bool SupportsFeature (DeleteFeature) const override;

		void CreateCollection (const QModelIndex& parent) override;
		bool HasUploadFeature (Feature) const override;
		void UploadImages (const QModelIndex& collection, const QList<UploadItem>& paths) override;

		void ImageUploadResponse (const QByteArray& content, const UploadItem&);
	private:
		bool TryToEnterLoginIfNoExists ();
		void CreatePhotoItem (const Photo& photo);

	private slots:
		void handleGotAlbums (const QList<Album>& albums);
		void handleGotAlbum (const Album& album);
		void handleGotPhotos (const QList<Photo>& photos);
		void handleGotPhoto (const Photo& photo);
		void handleDeletedPhotos (const QByteArray& id);
		void handleGotError (int errorCode, const QString& errorString);

	signals:
		void accountChanged (PicasaAccount *acc);
		void doneUpdating () override;
		void itemUploaded (const UploadItem&, const QUrl&) override;
	};
}
}
}
