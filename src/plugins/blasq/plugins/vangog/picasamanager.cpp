/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "picasamanager.h"
#include <QNetworkRequest>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sll/parsejson.h>
#include "picasaaccount.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	PicasaManager::PicasaManager (PicasaAccount *account, QObject *parent)
	: QObject (parent)
	, Account_ (account)
	, FirstRequest_ (true)
	{
	}

	void PicasaManager::Schedule (std::function<void (QString)> func)
	{
		ApiCallsQueue_ << func;
		RequestAccessToken ();
	}

	QString PicasaManager::GetAccessToken () const
	{
		return AccessToken_;
	}

	QDateTime PicasaManager::GetAccessTokenExpireDate () const
	{
		return AccessTokenExpireDate_;
	}

	void PicasaManager::UpdateCollections ()
	{
		ApiCallsQueue_ << [this] (const QString& key) { RequestCollections (key); };
		RequestAccessToken ();
	}

	void PicasaManager::UpdatePhotos (const QByteArray& albumId)
	{
		ApiCallsQueue_ << [this, albumId] (const QString& key) { RequestPhotos (albumId, key); };
	}

	void PicasaManager::DeletePhoto (const QByteArray& photoId, const QByteArray& albumId)
	{
		ApiCallsQueue_ << [this, photoId, albumId] (const QString& key)
				{ DeletePhoto (photoId, albumId, key); };
		RequestAccessToken ();
	}

	void PicasaManager::DeleteAlbum (const QByteArray& albumId)
	{
		ApiCallsQueue_ << [this, albumId] (const QString& key)
				{ DeleteAlbum (albumId, key); };
		RequestAccessToken ();
	}

	namespace
	{
		QString GetPicasaAccessFromInt (int access)
		{
			switch (access)
			{
			case 0:
				return "public";
			case 1:
				return "private";
			}

			return "private";
		}
	}

	void PicasaManager::CreateAlbum (const QString& name, const QString& desc, int access)
	{
		QString accessStr = GetPicasaAccessFromInt (access);
		ApiCallsQueue_ << [this, name, desc, accessStr] (const QString& key)
				{ CreateAlbum (name, desc, accessStr, key); };
		RequestAccessToken ();
	}

	QByteArray PicasaManager::CreateDomDocument (const QByteArray& content, QDomDocument &document)
	{
		QString errorMsg;
		int errorLine = -1, errorColumn = -1;

		if (QString::fromUtf8 (content).contains ("Invalid token"))
		{
			AccessToken_ = "";
			RequestAccessToken ();
			return QByteArray ();
		}

		if (!ApiCallsQueue_.isEmpty ())
			ApiCallsQueue_.removeFirst ();

		if (!document.setContent (content, &errorMsg, &errorLine, &errorColumn))
		{
			qWarning () << Q_FUNC_INFO
					<< errorMsg
					<< "in line:"
					<< errorLine
					<< "column:"
					<< errorColumn;
			return QByteArray ();
		}

		return content;
	}

	void PicasaManager::RequestAccessToken ()
	{
		if (FirstRequest_)
		{
			FirstRequest_ = false;
			AccessToken_ = Account_->GetAccessToken ();
			AccessTokenExpireDate_ = Account_->GetAccessTokenExpireDate ();
		}

		if (!AccessToken_.isEmpty () &&
				QDateTime::currentDateTime ().addSecs (60) < AccessTokenExpireDate_)
		{
			if (ApiCallsQueue_.isEmpty ())
				return;

			ApiCallsQueue_.first () (AccessToken_);
			return;
		}

		QNetworkRequest request (QUrl ("https://accounts.google.com/o/oauth2/token"));
		QString str = QString ("refresh_token=%1&client_id=%2&client_secret=%3&grant_type=%4")
				.arg (Account_->GetRefreshToken ())
				.arg ("844868161425.apps.googleusercontent.com")
				.arg ("l09HkM6nbPMEYcMdcdeGBdaV")
				.arg ("refresh_token");

		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

		QNetworkReply *reply = Account_->GetProxy ()->
				GetNetworkAccessManager ()->post (request, str.toUtf8 ());

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleAuthTokenRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::ParseError (const QVariantMap& map)
	{
		qWarning () << Q_FUNC_INFO << map;
	}

	namespace
	{
		QNetworkRequest CreateRequest (const QUrl& url)
		{
			QNetworkRequest request (url);
			request.setRawHeader ("GData-Version", "2");

			return request;
		}
	}

	void PicasaManager::RequestCollections (const QString& key)
	{
		QString urlStr = QString ("https://picasaweb.google.com/data/feed/api/user/%1?access_token=%2&access=all")
				.arg (Account_->GetLogin ())
				.arg (key);
		QNetworkReply *reply = Account_->GetProxy ()->GetNetworkAccessManager ()->
				get (CreateRequest (QUrl (urlStr)));

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestCollectionFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::RequestPhotos (const QByteArray& albumId, const QString& key)
	{
		QString urlStr = QString ("https://picasaweb.google.com/data/feed/api/user/%1/albumid/%2?access_token=%3&imgmax=d")
				.arg (Account_->GetLogin ())
				.arg (QString::fromUtf8 (albumId))
				.arg (key);
		QNetworkReply *reply = Account_->GetProxy ()->GetNetworkAccessManager ()->
				get (CreateRequest (QUrl (urlStr)));

		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleRequestPhotosFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::DeletePhoto (const QByteArray& photoId,
			const QByteArray& albumId, const QString& key)
	{
		QString str = QString ("https://picasaweb.google.com/data/entry/api/user/%1/albumid/%2/photoid/%3?access_token=%4")
				.arg (Account_->GetLogin ())
				.arg (QString::fromUtf8 (albumId))
				.arg (QString::fromUtf8 (photoId))
				.arg (key);
		QNetworkRequest request = CreateRequest (QUrl (str));
		request.setRawHeader ("If-Match", "*");
		QNetworkReply *reply = Account_->GetProxy ()->
				GetNetworkAccessManager ()->deleteResource (request);
		Reply2Id_ [reply] = photoId;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleDeletePhotoFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::DeleteAlbum (const QByteArray& albumId, const QString& key)
	{
		QString str = QString ("https://picasaweb.google.com/data/entry/api/user/%1/albumid/%2?access_token=%4")
				.arg (Account_->GetLogin ())
				.arg (QString::fromUtf8 (albumId))
				.arg (key);
		QNetworkRequest request = CreateRequest (QUrl (str));
		request.setRawHeader ("If-Match", "*");
		QNetworkReply *reply = Account_->GetProxy ()->
				GetNetworkAccessManager ()->deleteResource (request);
		Reply2Id_ [reply] = albumId;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleDeleteAlbumFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::CreateAlbum (const QString& name, const QString& desc,
			const QString& accessStr, const QString& key)
	{
		QDomDocument doc;
		QDomElement root = doc.createElement ("entry");
		root.setAttribute ("xmlns","http://www.w3.org/2005/Atom");
		root.setAttribute ("xmlns:media","http://search.yahoo.com/mrss/");
		root.setAttribute ("xmlns:gphoto","http://schemas.google.com/photos/2007");
		doc.appendChild (root);
		QDomElement title = doc.createElement ("title");
		title.setAttribute ("type", "text");
		root.appendChild (title);
		QDomText albumName = doc.createTextNode (name);
		title.appendChild (albumName);
		QDomElement summary = doc.createElement ("summary");
		summary.setAttribute ("type", "text");
		root.appendChild (summary);
		QDomText description = doc.createTextNode (desc);
		summary.appendChild (description);
		QDomElement access = doc.createElement ("gphoto:access");
		root.appendChild (access);
		QDomText accessRights = doc.createTextNode (accessStr);
		access.appendChild (accessRights);
		QDomElement timestamp = doc.createElement ("gphoto:timestamp");
		root.appendChild (timestamp);
		QDomText timestampValue = doc.createTextNode (QString::number (QDateTime::currentSecsSinceEpoch ()));
		timestamp.appendChild (timestampValue);
		QDomElement category = doc.createElement ("category");
		category.setAttribute ("scheme", "http://schemas.google.com/g/2005#kind");
		category.setAttribute ("term", "http://schemas.google.com/photos/2007#album");
		root.appendChild (category);

		QString urlString = QString ("https://picasaweb.google.com/data/feed/api/user/%1?access_token=%2")
				.arg (Account_->GetLogin ())
				.arg (key);
		QNetworkRequest request = CreateRequest (QUrl (urlString));
		request.setHeader (QNetworkRequest::ContentTypeHeader, "application/atom+xml");
		auto reply = Account_->GetProxy ()->GetNetworkAccessManager ()->
				post (request, doc.toByteArray ());
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleCreateAlbumFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void PicasaManager::handleAuthTokenRequestFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();

		const auto& map = Util::ParseJson (reply, Q_FUNC_INFO).toMap ();
		if (map.isEmpty ())
			return;

		AccessToken_ = map.value ("access_token").toString ();
		AccessTokenExpireDate_ = QDateTime::currentDateTime ().addSecs (map.value ("expires_in").toInt ());
		qDebug () << "your access token"
				<< AccessToken_
				<< "expires in"
				<< AccessTokenExpireDate_.toString (Qt::ISODate);
		if (AccessToken_.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO << "access token is empty";
			return;
		}

		if (ApiCallsQueue_.isEmpty ())
			return;

		ApiCallsQueue_.first () (AccessToken_);
	}

	namespace
	{
		Access PicasaRightsToAccess (const QString& rights)
		{
			if (rights == "protected" || rights == "private")
				return Access::Private;
			else
				return Access::Public;
		}

		Author PicasaAuthorToAuthor (const QDomElement& elem)
		{
			Author author;

			const auto& fields = elem.childNodes ();
			for (int i = 0, size = fields.size (); i < size; ++i)
			{
				const auto& field = fields.at (i).toElement ();
				const QString& name = field.tagName ();
				const QString& value = field.text ();
				if (name == "name")
					author.Name_ = value;
				else if (name == "uri")
					author.Image_ = value;
			}

			return author;
		}

		Thumbnail PicasaMediaGroupToThumbnail (const QDomElement& elem)
		{
			Thumbnail thumbnail;
			const auto& fields = elem.childNodes ();
			for (int i = 0, count = fields.size (); i < count; ++i)
			{
				const auto& field = fields.at (i).toElement ();
				const QString& name = field.tagName ();
				if (name == "media:thumbnail")
				{
					thumbnail.Height_ = field.attribute ("height").toInt ();
					thumbnail.Width_ = field.attribute ("width").toInt ();
					thumbnail.Url_ = QUrl (field.attribute ("url"));
					break;
				}
			}

			return thumbnail;
		}

		Album PicasaEntryToAlbum (const QDomElement& elem)
		{
			Album album;

			const auto& fields = elem.childNodes ();
			for (int i = 0, count = fields.size (); i < count; ++i)
			{
				const auto& field = fields.at (i).toElement ();
				const QString& name = field.tagName ();
				const QString& value = field.text ();
				if (name == "published")
					album.Published_ = QDateTime::fromString (value, Qt::ISODate);
				else if (name == "updated")
					album.Updated_ = QDateTime::fromString (value, Qt::ISODate);
				else if (name == "title")
					album.Title_ = value;
				else if (name == "summary")
					album.Description_ = value;
				else if (name == "rights")
					album.Access_ = PicasaRightsToAccess (value);
				else if (name == "author")
					album.Author_ = PicasaAuthorToAuthor (field);
				else if (name == "gphoto:id")
					album.ID_ = value.toUtf8 ();
				else if (name == "gphoto:numphotos")
					album.NumberOfPhoto_ = value.toInt ();
				else if (name == "gphoto:bytesUsed")
					album.BytesUsed_ = value.toULongLong ();
				else if (name == "media:group")
					//TODO check is it possible to more then one thumbnail
					album.Thumbnails_ << PicasaMediaGroupToThumbnail (field);
			}

			return album;
		}
	}

	QList<Album> PicasaManager::ParseAlbums (const QDomDocument& document)
	{
		QList<Album> albums;
		const auto& entryElements = document.elementsByTagName ("entry");
		if (entryElements.isEmpty ())
			return albums;

		for (int i = 0, count = entryElements.count (); i < count; ++i)
		{
			const auto& album = PicasaEntryToAlbum (entryElements.at (i).toElement ());
			UpdatePhotos (album.ID_);
			albums << album;
		}

		return albums;
	}

	void PicasaManager::handleRequestCollectionFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QDomDocument document;
		reply->deleteLater ();
		if (CreateDomDocument (reply->readAll (), document).isEmpty ())
			return;
		emit gotAlbums (ParseAlbums (document));
		RequestAccessToken ();
	}

	namespace
	{
		Exif PicasaExifTagsToExif (const QDomElement& element)
		{
			Exif exif;
			const auto& fields = element.childNodes ();
			for (int i = 0, count = fields.size (); i < count; ++i)
			{
				const auto& field = fields.at (i).toElement ();
				const QString& name = field.tagName ();
				const QString& value = field.text ();
				if (name == "exif:fstop")
					exif.FNumber_ = value.toInt ();
				else if (name == "exif:make")
					exif.Manufacturer_ = value;
				else if (name == "exif:model")
					exif.Model_ = value;
				else if (name == "exif:exposure")
					exif.Exposure_ = value.toFloat ();
				else if (name == "exif:flash")
					exif.Flash_ = (value == "true");
				else if (name == "exif:focallength")
					exif.FocalLength_ = value.toFloat ();
				else if (name == "exif:iso")
					exif.ISO_ = value.toInt ();
			}
			return exif;
		}

		Photo PicasaEntryToPhoto (const QDomElement& elem)
		{
			Photo photo;
			const auto& fields = elem.childNodes ();
			QDomElement mediaGroupElement;
			for (int i = 0, count = fields.size (); i < count; ++i)
			{
				const auto& field = fields.at (i).toElement ();
				const QString& name = field.tagName ();
				const QString& value = field.text ();
				if (name == "title")
					photo.Title_ = value;
				else if (name == "published")
					photo.Published_ = QDateTime::fromString (value, Qt::ISODate);
				else if (name == "updated")
					photo.Updated_ = QDateTime::fromString (value, Qt::ISODate);
				else if (name == "gphoto:id")
					photo.ID_ = value.toUtf8 ();
				else if (name == "gphoto:albumid")
					photo.AlbumID_ = value.toUtf8 ();
				else if (name == "gphoto:access")
					photo.Access_ = PicasaRightsToAccess (value);
				else if (name == "gphoto:width")
					photo.Width_ = value.toInt ();
				else if (name == "gphoto:height")
					photo.Height_ = value.toInt ();
				else if (name == "gphoto:size")
					photo.Size_ = value.toULongLong ();
				else if (name == "exif:tags")
					photo.Exif_ = PicasaExifTagsToExif (field);
				else if (name == "media:group")
				{
					const auto& mgFields = field.childNodes ();
					for (int i = 0, count = mgFields.size (); i < count; ++i)
					{
						const auto& mgField = mgFields.at (i).toElement ();
						const auto& mgName = mgFields.at (i).toElement ().tagName ();
						const auto& mgValue = mgFields.at (i).toElement ().text ();
						if (mgName == "media:content")
						{
							photo.Height_ = mgField.attribute ("height").toInt ();
							photo.Width_ = mgField.attribute ("width").toInt ();
							photo.Url_ = QUrl (mgField.attribute ("url"));
						}
						else if (mgName == "media:keywords")
							photo.Tags_ = mgValue.split (',');
						else if (mgName == "media:thumbnail")
						{
							Thumbnail thumbnail;
							thumbnail.Height_ = mgField.attribute ("height").toInt ();
							thumbnail.Width_ = mgField.attribute ("width").toInt ();
							thumbnail.Url_ = QUrl (mgField.attribute ("url"));
							photo.Thumbnails_ << thumbnail;
						}
					}
				}
			}

			return photo;
		}

		QList<Photo> ParsePhotos (const QDomDocument& document)
		{
			QList<Photo> photos;
			const auto& EntryElements = document.elementsByTagName ("entry");
			if (EntryElements.isEmpty ())
				return photos;
			for (int i = 0, count = EntryElements.count (); i < count; ++i)
				photos << PicasaEntryToPhoto (EntryElements.at (i).toElement ());;

			return photos;
		}
	}

	void PicasaManager::handleRequestPhotosFinished ()
	{
		QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QDomDocument document;
		reply->deleteLater ();
		if (CreateDomDocument (reply->readAll (), document).isEmpty ())
			return;

		emit gotPhotos (ParsePhotos (document));
		RequestAccessToken ();
	}

	void PicasaManager::handleDeletePhotoFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		ApiCallsQueue_.removeFirst ();
		const auto& content = reply->readAll ();
		const auto& id = Reply2Id_.take (reply);
		content.isEmpty () ?
			emit deletedPhoto (id) :
			emit gotError (reply->error (), QString::fromUtf8 (content));
		reply->deleteLater ();
		RequestAccessToken ();
	}

	void PicasaManager::handleDeleteAlbumFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		qDebug () << Q_FUNC_INFO << reply->readAll ();
	}

	void PicasaManager::handleCreateAlbumFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		QDomDocument doc;
		if (CreateDomDocument (reply->readAll (), doc).isEmpty ())
			return;
		emit gotAlbum (ParseAlbums (doc).value (0));
		RequestAccessToken ();
	}

	Photo PicasaManager::handleImageUploaded (const QByteArray& content)
	{
		QDomDocument document;
		if (CreateDomDocument (content, document).isEmpty ())
			return {};

		const auto& photo = ParsePhotos (document).value (0);
		emit gotPhoto (photo);
		RequestAccessToken ();

		return photo;
	}

	void PicasaManager::handleNetworkError (QNetworkReply::NetworkError error)
	{
		auto reply = qobject_cast<QNetworkReply *> (sender ());
		QString errorText;
		if (reply)
		{
			errorText = reply->errorString ();
			reply->deleteLater ();
		}
		emit gotError (error, errorText);
	}

}
}
}
