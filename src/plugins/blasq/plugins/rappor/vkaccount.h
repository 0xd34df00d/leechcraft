/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <QObject>
#include <interfaces/blasq/iaccount.h>
#include <interfaces/blasq/isupportuploads.h>
#include <interfaces/blasq/isupportdeletes.h>
#include <interfaces/core/icoreproxy.h>

class QNetworkReply;
class QDomElement;
class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Util
{
	class QueueManager;

	namespace SvcAuth
	{
		class VkAuthManager;
	}
}

namespace Blasq
{
namespace Rappor
{
	class VkService;
	class UploadManager;

	class VkAccount : public QObject
					, public IAccount
					, public ISupportUploads
					, public ISupportDeletes
	{
		Q_OBJECT
		Q_INTERFACES (LC::Blasq::IAccount
				LC::Blasq::ISupportUploads
				LC::Blasq::ISupportDeletes)

		QString Name_;
		const QByteArray ID_;
		VkService * const Service_;
		const ICoreProxy_ptr Proxy_;

		bool IsUpdating_ = false;

		QStandardItemModel * const CollectionsModel_;
		QStandardItem *AllPhotosItem_ = 0;
		QHash<int, QStandardItem*> Albums_;

		Util::SvcAuth::VkAuthManager * const AuthMgr_;

		QByteArray LastCookies_;

		QList<std::function<void (QString)>> CallQueue_;
		Util::QueueManager *RequestQueue_;

		UploadManager * const UploadManager_;
	public:
		VkAccount (const QString&, VkService*, ICoreProxy_ptr,
				const QByteArray& id = QByteArray (),
				const QByteArray& cookies = QByteArray ());

		QByteArray Serialize () const;
		static VkAccount* Deserialize (const QByteArray&, VkService*, ICoreProxy_ptr);

		void Schedule (std::function<void (QString)>);

		QObject* GetQObject () override;
		IService* GetService () const override;
		QString GetName () const override;
		QByteArray GetID () const override;

		QAbstractItemModel* GetCollectionsModel () const override;
		void UpdateCollections () override;

		bool HasUploadFeature (Feature) const override;
		void CreateCollection (const QModelIndex& parent) override;
		void UploadImages (const QModelIndex& collection, const QList<UploadItem>& paths) override;

		bool SupportsFeature (DeleteFeature) const override;
		void Delete (const QModelIndex&) override;
	private:
		void HandleAlbumElement (const QDomElement&);
		bool HandlePhotoElement (const QDomElement&, bool atEnd = true);
	private slots:
		void handleGotAlbums ();
		void handleGotPhotos ();

		void handleAlbumCreated ();
		void handlePhotosInfosFetched ();

		void handleAuthKey (const QString&);
		void handleCookies (const QByteArray&);
	signals:
		void accountChanged (VkAccount*);

		void itemUploaded (const UploadItem&, const QUrl&) override;

		void doneUpdating () override;
	};
}
}
}
