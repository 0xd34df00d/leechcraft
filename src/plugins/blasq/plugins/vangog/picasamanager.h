/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QDateTime>
#include <QDomDocument>
#include <QStringList>
#include <QQueue>
#include <QUrl>
#include <QVariant>
#include <QNetworkReply>
#include "structures.h"

class QNetworkReply;

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	class PicasaAccount;

	class PicasaManager : public QObject
	{
		Q_OBJECT

		PicasaAccount * const Account_;
		QQueue<std::function<void (const QString&)>> ApiCallsQueue_;
		QString AccessToken_;
		QDateTime AccessTokenExpireDate_;
		bool FirstRequest_;

		QHash<QNetworkReply*, QByteArray> Reply2Id_;

	public:
		PicasaManager (PicasaAccount *account, QObject *parent = 0);

		void Schedule (std::function<void (QString)> func);

		QString GetAccessToken () const;
		QDateTime GetAccessTokenExpireDate () const;

		void UpdateCollections ();
		void UpdatePhotos (const QByteArray& albumId);

		void DeletePhoto (const QByteArray& photoId, const QByteArray& albumId);
		void DeleteAlbum (const QByteArray& albumId);
		void CreateAlbum (const QString& name, const QString& desc, int access);
	private:
		QByteArray CreateDomDocument (const QByteArray& content, QDomDocument &document);
		void RequestAccessToken ();
		void ParseError (const QVariantMap& map);

		void RequestCollections (const QString& key);
		void RequestPhotos (const QByteArray& albumId, const QString& key);

		void DeletePhoto (const QByteArray& photoId,
				const QByteArray& albumId, const QString& key);
		void DeleteAlbum (const QByteArray& albumId, const QString& key);
		void CreateAlbum (const QString& name, const QString& desc,
				const QString& accessStr, const QString& key);

		QList<Album> ParseAlbums (const QDomDocument& document);

	private slots:
		void handleAuthTokenRequestFinished ();
		void handleRequestCollectionFinished ();
		void handleRequestPhotosFinished ();
		void handleDeletePhotoFinished ();
		void handleDeleteAlbumFinished ();
		void handleCreateAlbumFinished ();
		void handleNetworkError (QNetworkReply::NetworkError error);
	public slots:
		Photo handleImageUploaded (const QByteArray& image = QByteArray ());
	signals:
		void gotAlbums (const QList<Album>& albums);
		void gotAlbum (const Album album);
		void gotPhotos (const QList<Photo>& photos);
		void gotPhoto (const Photo& photos);
		void deletedPhoto (const QByteArray& id);
		void gotError (int errorCode, const QString& errorString);
	};
}
}
}
