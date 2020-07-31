/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QNetworkReply>
#include <QObject>
#include <QSet>
#include <QQueue>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/blasq/iaccount.h>
#include <interfaces/blasq/isupportuploads.h>
#include "structures.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	class FotoBilderService;

	class FotoBilderAccount : public QObject
							, public IAccount
							, public ISupportUploads
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IAccount LC::Blasq::ISupportUploads)

		QString Name_;
		FotoBilderService * const Service_;
		const ICoreProxy_ptr Proxy_;
		QByteArray ID_;
		QString Login_;
		bool FirstRequest_ = true;
		Quota Quota_;

		QStandardItemModel * const CollectionsModel_;
		QStandardItem *AllPhotosItem_ = nullptr;

		QHash<QByteArray, QStandardItem*> Id2AlbumItem_;
		QHash<QNetworkReply*, UploadItem> Reply2UploadItem_;

		QQueue<std::function<void (const QString&)>> CallsQueue_;
	public:
		FotoBilderAccount (const QString& name, FotoBilderService *service,
				ICoreProxy_ptr proxy, const QString& login,
				const QByteArray& id = QByteArray ());

		ICoreProxy_ptr GetProxy () const;

		QByteArray Serialize () const;
		static FotoBilderAccount* Deserialize (const QByteArray& data,
				FotoBilderService *service, ICoreProxy_ptr proxy);

		QObject* GetQObject () override;
		IService* GetService () const override;
		QString GetName () const override;
		QByteArray GetID () const override;

		QAbstractItemModel* GetCollectionsModel () const override;

		void CreateCollection (const QModelIndex& parent) override;
		bool HasUploadFeature (Feature) const override;
		void UploadImages (const QModelIndex& collection, const QList<UploadItem>& paths) override;

		void Login ();
		void RequestGalleries ();
		void RequestPictures ();

		void UpdateCollections () override;

	private:
		auto MakeRunnerGuard ();
		void CallNextFunctionFromQueue ();
		bool IsErrorReply (const QByteArray& content);
		void GetChallenge ();
		void LoginRequest (const QString& challenge);
		void GetGalsRequest (const QString& challenge);
		void GetPicsRequest (const QString& challenge);
		void CreateGallery (const QString& name, int privacyLevel, const QString& challenge);
		void UploadImagesRequest (const QByteArray& albumId, const QList<UploadItem>& items);
		void UploadOneImage (const QByteArray& id,
				const UploadItem& item, const QString& challenge);

	private slots:
		void handleGetChallengeRequestFinished ();
		void handleLoginRequestFinished ();

		void handleNetworkError (QNetworkReply::NetworkError err);

		void handleGotAlbums ();
		void handleGotPhotos ();
		void handleGalleryCreated ();
		void handleUploadProgress (qint64 sent, qint64 total);
		void handleImageUploaded ();

	signals:
		void accountChanged (FotoBilderAccount *acc);
		void doneUpdating () override;
		void networkError (QNetworkReply::NetworkError err, const QString& errString);
		void itemUploaded (const UploadItem&, const QUrl&) override;
	};
}
}
}
