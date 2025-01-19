/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fotobilderaccount.h"
#include <QDomDocument>
#include <QCryptographicHash>
#include <QInputDialog>
#include <QMainWindow>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardItemModel>
#include <QtDebug>
#include <QUuid>
#include <QFileInfo>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include <util/sll/unreachable.h>
#include <util/sll/util.h>
#include <util/xpc/util.h>
#include "albumsettingsdialog.h"
#include "fotobilderservice.h"
#include "util.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	namespace
	{
		const QString Url ("http://pics.livejournal.com/interface/simple");
		//Available sizes: 100, 300, 320, 600, 640, 900, 1000

		const int SmallSize = 320;
		const int MediumSize = 640;
		const QString SmallSizeStr = QString::number (SmallSize);
		const QString MediumSizeStr = QString::number (MediumSize);
	}

	FotoBilderAccount::FotoBilderAccount (const QString& name, FotoBilderService *service,
			ICoreProxy_ptr proxy, const QString& login, const QByteArray& id)
	: QObject (service)
	, Name_ (name)
	, Service_ (service)
	, Proxy_ (proxy)
	, ID_ (id.isEmpty () ? QUuid::createUuid ().toByteArray () : id)
	, Login_ (login)
	, CollectionsModel_ (new NamedModel<QStandardItemModel> (this))
	{
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });
	}

	ICoreProxy_ptr FotoBilderAccount::GetProxy () const
	{
		return Proxy_;
	}

	QByteArray FotoBilderAccount::Serialize () const
	{
		QByteArray result;
		{
			QDataStream out (&result, QIODevice::WriteOnly);
			out << static_cast<quint8> (1)
					<< Name_
					<< ID_
					<< Login_;
		}
		return result;
	}

	FotoBilderAccount* FotoBilderAccount::Deserialize (const QByteArray& ba,
			FotoBilderService *service, ICoreProxy_ptr proxy)
	{
		QDataStream in (ba);

		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return nullptr;
		}

		QString name;
		QString login;
		QByteArray id;
		in >> name
				>> id
				>> login;

		return new FotoBilderAccount (name, service, proxy, login, id);
	}

	QObject* FotoBilderAccount::GetQObject ()
	{
		return this;
	}

	IService* FotoBilderAccount::GetService () const
	{
		return Service_;
	}

	QString FotoBilderAccount::GetName () const
	{
		return Name_;
	}

	QByteArray FotoBilderAccount::GetID () const
	{
		return ID_;
	}

	QAbstractItemModel* FotoBilderAccount::GetCollectionsModel () const
	{
		return CollectionsModel_;
	}

	auto FotoBilderAccount::MakeRunnerGuard ()
	{
		const bool shouldRun = CallsQueue_.isEmpty ();
		return Util::MakeScopeGuard ([this, shouldRun]
				{
					if (shouldRun)
						CallsQueue_.dequeue () (QString ());
				});
	}

	void FotoBilderAccount::CreateCollection (const QModelIndex&)
	{
		AlbumSettingsDialog dia ({}, Login_, this);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& name = dia.GetName ();
		int priv = dia.GetPrivacyLevel ();

		auto guard = MakeRunnerGuard ();
		CallsQueue_ << [this] (const QString&) { GetChallenge (); };
		CallsQueue_ << [this, name, priv] (const QString& challenge)
		{
			CreateGallery (name, priv, challenge);
		};
	}

	bool FotoBilderAccount::HasUploadFeature (ISupportUploads::Feature feature) const
	{
		switch (feature)
		{
		case Feature::RequiresAlbumOnUpload:
		case Feature::SupportsDescriptions:
			return true;
		}

		Util::Unreachable ();
	}

	void FotoBilderAccount::UploadImages (const QModelIndex& collection, const QList<UploadItem>& items)
	{
		if (!items.count ())
			return;

		const auto& aidStr = collection.data (CollectionRole::ID).toByteArray ();
		UploadImagesRequest (aidStr, items);
	}

	namespace
	{
		QNetworkRequest CreateRequest (const QMap<QByteArray, QByteArray>& fields)
		{
			QNetworkRequest request (Url);
			for (const auto& field : fields.keys ())
				request.setRawHeader (field, fields [field]);

			return request;
		}

		QByteArray CreateDomDocumentFromReply (QNetworkReply *reply, QDomDocument &document)
		{
			if (!reply)
				return QByteArray ();

			const auto& content = reply->readAll ();
			reply->deleteLater ();
			QString errorMsg;
			int errorLine = -1, errorColumn = -1;
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

		QString LocalizedErrorFromCode (int code)
		{
			switch (code)
			{
			case 100:
				return QObject::tr ("User error");
			case 101:
				return QObject::tr ("No user specified");
			case 102:
				return QObject::tr ("Invalid user");
			case 103:
				return QObject::tr ("Unknown user");
			case 200:
				return QObject::tr ("Client error");
			case 202:
				return QObject::tr ("Invalid mode");
			case 211:
				return QObject::tr ("Invalid argument");
			case 212:
				return QObject::tr ("Missing required argument");
			case 213:
				return QObject::tr ("Invalid image for upload");
			case 300:
				return QObject::tr ("Access error");
			case 301:
				return QObject::tr ("No auth specified");
			case 302:
				return QObject::tr ("Invalid auth");
			case 303:
				return QObject::tr ("Account status does not allow upload");
			case 400:
				return QObject::tr ("Limit error");
			case 401:
				return QObject::tr ("No disk space remaining");
			case 402:
				return QObject::tr ("Insufficient disk space remaining");
			case 500:
				return QObject::tr ("Internal Server Error");
			case 510:
				return QObject::tr ("Error creating pic");
			case 512:
				return QObject::tr ("Error creating gallery");
			default:
				return QString ();
			}
		}
	}

	void FotoBilderAccount::CallNextFunctionFromQueue ()
	{
		if (!CallsQueue_.isEmpty () && !(CallsQueue_.count () % 2))
			CallsQueue_.dequeue () (QString ());
	}

	namespace
	{
		struct Error
		{
			int Code_;
			QString Text_;
		};

		std::optional<Error> GetChildError (const QDomElement& elem)
		{
			const auto errorElem = elem.firstChildElement ("Error");
			if (errorElem.isNull ())
				return {};
			return Error { errorElem.attribute ("code"_qs).toInt (), errorElem.text () };
		}

		std::optional<Error> GetError (const QDomElement& fbResponse)
		{
			if (const auto err = GetChildError (fbResponse))
				return err;

			for (const auto& child : Util::DomChildren (fbResponse, {}))
				if (const auto err = GetChildError (child))
					return err;

			return {};
		}
	}

	bool FotoBilderAccount::IsErrorReply (const QByteArray& content)
	{
		QDomDocument doc;
		if (!doc.setContent (content))
		{
			qWarning () << "unable to parse XML:" << content;
			return true;
		}

		const auto& fbResponse = doc.documentElement ().firstChildElement ("FBResponse");
		if (fbResponse.isNull ())
		{
			qWarning () << "no FBResponse in" << content;
			return true;
		}

		const auto err = GetError (fbResponse);
		if (!err)
			return false;

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blasq DeathNote",
				tr ("%1 (original message: %2)").arg (LocalizedErrorFromCode (err->Code_), err->Text_),
				Priority::Warning));
		return true;
	}

	void FotoBilderAccount::Login ()
	{
		auto guard = MakeRunnerGuard ();
		CallsQueue_ << [this] (const QString&) { GetChallenge (); };
		CallsQueue_ << [this] (const QString& challenge) { LoginRequest (challenge); };
	}

	void FotoBilderAccount::RequestGalleries ()
	{
		auto guard = MakeRunnerGuard ();
		CallsQueue_ << [this] (const QString&) { GetChallenge (); };
		CallsQueue_ << [this] (const QString& challenge) { GetGalsRequest (challenge); };
	}

	void FotoBilderAccount::RequestPictures ()
	{
		auto guard = MakeRunnerGuard ();
		CallsQueue_ << [this] (const QString&) { GetChallenge (); };
		CallsQueue_ << [this] (const QString& challenge) { GetPicsRequest (challenge); };
	}

	void FotoBilderAccount::UpdateCollections ()
	{
		if (FirstRequest_)
		{
			Login ();
			FirstRequest_ = false;
		}

		RequestGalleries ();
	}

	void FotoBilderAccount::GetChallenge ()
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->get (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "GetChallenge" }
				}));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGetChallengeRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	namespace
	{
		QByteArray GetAuthHeader (const QByteArray& id, const QString& name,
				const ICoreProxy_ptr& proxy, const QString& challenge)
		{
			const auto& hashed = GetHashedChallenge (GetAccountPassword (id, name, proxy), challenge);
			return ("crp:" + challenge + ":" + hashed).toUtf8 ();
		}
	}

	void FotoBilderAccount::LoginRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->get (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "Login" },
					{ "X-FB-Auth", GetAuthHeader (GetID (), GetName (), Proxy_, challenge) },
					{ "X-FB-Login.ClientVersion", "LeechCraft Blasq/" + Proxy_->GetVersion ().toUtf8 () }
				}));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleLoginRequestFinished ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::GetGalsRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->get (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "GetGals" },
					{ "X-FB-Auth", GetAuthHeader (GetID (), GetName (), Proxy_, challenge) }
				}));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotAlbums ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::GetPicsRequest (const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->get (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "GetPics" },
					{ "X-FB-Auth", GetAuthHeader (GetID (), GetName (), Proxy_, challenge) }
				}));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGotPhotos ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::CreateGallery (const QString& name, int privacyLevel,
			const QString& challenge)
	{
		auto reply = Proxy_->GetNetworkAccessManager ()->get (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "CreateGals" },
					{ "X-FB-Auth", GetAuthHeader (GetID (), GetName (), Proxy_, challenge) },
					{ "X-FB-CreateGals.Gallery._size", "1" },
					{ "X-FB-CreateGals.Gallery.0.ParentID", "0" },
					{ "X-FB-CreateGals.Gallery.0.GalName", name.toUtf8 () },
					{ "X-FB-CreateGals.Gallery.0.GalSec", QString::number (privacyLevel).toUtf8 () }
				}));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleGalleryCreated ()));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::UploadImagesRequest (const QByteArray& albumId, const QList<UploadItem>& items)
	{
		auto guard = MakeRunnerGuard ();
		for (const auto& item : items)
		{
			CallsQueue_ << [this] (const QString&) { GetChallenge (); };
			CallsQueue_ << [albumId, item, this] (const QString& challenge)
			{
				UploadOneImage (albumId, item, challenge);
			};
		}
	}

	void FotoBilderAccount::UploadOneImage (const QByteArray& id,
			const UploadItem& item, const QString& challenge)
	{
		QFile file (item.FilePath_);
		if (!file.open (QIODevice::ReadOnly))
			return;

		auto content = file.readAll ();
		QByteArray md5 = QCryptographicHash::hash (content, QCryptographicHash::Md5);
		file.close ();
		auto reply = Proxy_->GetNetworkAccessManager ()->put (CreateRequest ({
					{ "X-FB-User", Login_.toUtf8 () },
					{ "X-FB-Mode", "UploadPic" },
					{ "X-FB-Auth", GetAuthHeader (GetID (), GetName (), Proxy_, challenge) },
					{ "X-FB-AuthVerifier", "md5=" + md5 + "&mode=UploadPic" },
					{ "X-FB-UploadPic.ImageData", QDateTime::currentDateTime ()
							.toString (Qt::ISODate).toUtf8 () },
					{ "X-FB-UploadPic.MD5", md5 },
					//TODO access to images
					{ "X-FB-UploadPic.PicSec", "255" },
					{ "X-FB-UploadPic.Meta.Filename", QFileInfo (item.FilePath_)
							.fileName ().toUtf8 () },
					{ "X-FB-UploadPic.Meta.Title", QFileInfo (item.FilePath_)
							.fileName ().toUtf8 () },
					{ "X-FB-UploadPic.Meta.Description", item.Description_.toUtf8 () },
					{ "X-FB-UploadPic.Gallery._size", "1" },
					{ "X-FB-UploadPic.Gallery.0.GalID", id },
					{ "X-FB-UploadPic.ImageSize", QString::number (QFileInfo (item.FilePath_)
							.size ()).toUtf8 () }
				}),
				content);
		Reply2UploadItem_ [reply] = item;
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleImageUploaded ()));
		connect (reply,
				SIGNAL (uploadProgress (qint64, qint64)),
				this,
				SLOT (handleUploadProgress (qint64, qint64)));
		connect (reply,
				SIGNAL (error (QNetworkReply::NetworkError)),
				this,
				SLOT (handleNetworkError (QNetworkReply::NetworkError)));
	}

	void FotoBilderAccount::handleGetChallengeRequestFinished ()
	{
		QDomDocument document;
		const QByteArray& content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		const auto& challengeElem = document.documentElement ()
				.firstChildElement ("FBResponse")
				.firstChildElement ("GetChallengeResponse")
				.firstChildElement ("Challenge");
		if (challengeElem.isNull ())
			return;

		const auto& challenge = challengeElem.text ();
		if (!CallsQueue_.isEmpty ())
			CallsQueue_.dequeue () (challenge.trimmed ());
	}

	namespace
	{
		Quota ParseLoginResponse (const QDomDocument& document)
		{
			Quota quota;

			const auto& list = document.elementsByTagName ("Quota");
			if (!list.isEmpty ())
			{
				const auto& fieldsList = list.at (0).childNodes ();
				for (int i = 0, size = fieldsList.size (); i < size; ++i)
				{
					const auto& fieldElem = fieldsList.at (i).toElement ();
					if (fieldElem.tagName () == "Total")
						quota.Total_ = fieldElem.text ().toULongLong ();
					else if (fieldElem.tagName () == "Used")
						quota.Used_ = fieldElem.text ().toULongLong ();
					else if (fieldElem.tagName () == "Remaining")
						quota.Remaining_ = fieldElem.text ().toULongLong ();
				}
			}

			return quota;
		}

		Access Security2Access (int sec)
		{
			if (!sec)
				return Access::Private;

			if (sec == 253 || sec == 255)
				return Access::Public;

			if (sec == 254)
				return Access::FriendsOnly;

			if (sec >= 1 && sec <= 30)
				return Access::CustomUsers;

			return Access::Public;
		}

		QList<Album> ParseGetGalsRequest (const QDomDocument& document)
		{
			QList<Album> albums;
			const auto& list = document.elementsByTagName ("Gal");
			for (int i = 0, size = list.size (); i < size; ++i)
			{
				Album album;
				const auto& node = list.at (i);
				album.ID_ = node.toElement ().attribute ("id").toUtf8 ();
				const auto& fieldsList = node.childNodes ();
				for (int j = 0, sz = fieldsList.size (); j < sz; ++j)
				{
					const auto& fieldElem = fieldsList.at (j).toElement ();
					if (fieldElem.tagName () == "Name")
						album.Title_ = fieldElem.text ();
					else if (fieldElem.tagName () == "Date")
						album.CreationDate_ = QDateTime::fromString (fieldElem.text (),
								"yyyy-dd-mm hh:MM:ss");
					else if (fieldElem.tagName () == "URL")
						album.Url_ = QUrl (fieldElem.text ());
					else if (fieldElem.tagName () == "Sec")
						album.Access_ = Security2Access (fieldElem.text ().toInt());
				}
				albums << album;
			}

			return albums;
		}

		QList<Thumbnail> GenerateThumbnails (const QUrl& originalUrl)
		{
			Thumbnail small;
			small.Url_ = originalUrl.toString ().replace ("original", SmallSizeStr);
			small.Height_ = SmallSize;
			small.Width_ = SmallSize;

			Thumbnail medium;
			medium.Url_ = originalUrl.toString ().replace ("original", MediumSizeStr);
			medium.Height_ = MediumSize;
			medium.Width_ = MediumSize;
			return { small, medium };
		}

		Photo CreatePhoto (const QDomNodeList& fields)
		{
			Photo photo;
			for (int j = 0, sz = fields.size (); j < sz; ++j)
			{
				const auto& fieldElem = fields.at (j).toElement ();
				if (fieldElem.tagName () == "PicID")
					photo.ID_ = fieldElem.text ().toUtf8 ();
				else if (fieldElem.tagName () == "Bytes")
					photo.Size_ = fieldElem.text ().toULongLong ();
				else if (fieldElem.tagName () == "Format")
					photo.Format_ = fieldElem.text ();
				else if (fieldElem.tagName () == "Height")
					photo.Height_ = fieldElem.text ().toInt ();
				else if (fieldElem.tagName () == "MD5")
					photo.MD5_ = fieldElem.text ().toUtf8 ();
				else if (fieldElem.tagName () == "Meta")
				{
					if (fieldElem.attribute ("name") == "title")
						photo.Title_ = fieldElem.text ();
					else if (fieldElem.attribute ("name") == "description")
						photo.Description_ = fieldElem.text ();
					else if (fieldElem.attribute ("name") == "filename")
						photo.OriginalFileName_ = fieldElem.text ();
				}
				else if (fieldElem.tagName () == "URL")
					photo.Url_ = QUrl (fieldElem.text ());
				else if (fieldElem.tagName () == "Width")
					photo.Width_ = fieldElem.text ().toInt ();
				else if (fieldElem.tagName () == "Sec")
					photo.Access_ = Security2Access (fieldElem.text ().toInt());
			}
			return photo;
		}

		QList<Photo> ParseGetPicsRequest (const QDomDocument& document)
		{
			QList<Photo> photos;
			const auto& list = document.elementsByTagName ("Pic");
			for (int i = 0, size = list.size (); i < size; ++i)
			{

				const auto& picNode = list.at (i);
				const auto& fieldsList = picNode.childNodes ();
				Photo photo = CreatePhoto (fieldsList);
				photo.ID_ = picNode.toElement ().attribute ("id").toUtf8 ();
				photo.Thumbnails_ = GenerateThumbnails (photo.Url_);
				photos << photo;
			}

			return photos;
		}

		Photo ParseUploadedPictureResponse (const QDomDocument& document)
		{
			const auto& list = document.elementsByTagName ("UploadPicResponse");
			Photo photo;
			if (list.isEmpty ())
				return photo;

			const auto& picNode = list.at (0);
			const auto& fieldsList = picNode.childNodes ();
			photo = CreatePhoto (fieldsList);
			photo.Thumbnails_ = GenerateThumbnails (photo.Url_);

			return photo;
		}
	}

	void FotoBilderAccount::handleLoginRequestFinished ()
	{
		QDomDocument document;
		const QByteArray& content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (IsErrorReply (content))
			return;

		Quota_ = ParseLoginResponse (document);
		CallNextFunctionFromQueue ();
	}

	void FotoBilderAccount::handleNetworkError (QNetworkReply::NetworkError err)
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;
		reply->deleteLater ();
		qWarning () << Q_FUNC_INFO
				<< err
				<< reply->errorString ();
		emit networkError (err, reply->errorString ());
		CallNextFunctionFromQueue ();
	}

	void FotoBilderAccount::handleGotAlbums ()
	{
		QDomDocument document;
		const QByteArray& content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (IsErrorReply (content))
			return;

		if (auto rc = CollectionsModel_->rowCount ())
			CollectionsModel_->removeRows (0, rc);
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });

		AllPhotosItem_ = new QStandardItem (tr ("All photos"));
		AllPhotosItem_->setData (ItemType::AllPhotos, CollectionRole::Type);
		AllPhotosItem_->setEditable (false);
		CollectionsModel_->appendRow (AllPhotosItem_);

		//TODO enable due to upload issues
		const auto& albums = ParseGetGalsRequest (document);
		for (const auto& album : albums)
		{
			auto item = new QStandardItem (album.Title_);
			item->setData (ItemType::Collection, CollectionRole::Type);
			item->setEditable (false);
			item->setData (album.ID_, CollectionRole::ID);
			CollectionsModel_->appendRow (item);
			Id2AlbumItem_ [album.ID_] = item;
		}

		RequestPictures ();
	}

	namespace
	{
		QStandardItem* CreatePhotoItem (const Photo& photo)
		{
			const auto& name = photo.Title_.isEmpty () ?
				photo.OriginalFileName_ :
				photo.Title_;
			auto item = new QStandardItem (name);
			item->setEditable (false);
			item->setData (ItemType::Image, CollectionRole::Type);
			item->setData (photo.ID_, CollectionRole::ID);
			item->setData (name , CollectionRole::Name);

			item->setData (photo.Url_, CollectionRole::Original);
			item->setData (QSize (photo.Width_, photo.Height_),
					CollectionRole::OriginalSize);
			if (!photo.Thumbnails_.isEmpty ())
			{
				auto first = photo.Thumbnails_.first ();
				auto last = photo.Thumbnails_.last ();
				item->setData (first.Url_, CollectionRole::SmallThumb);
				item->setData (QSize (first.Width_, first.Height_),
						CollectionRole::SmallThumbSize);
				item->setData (last.Url_, CollectionRole::MediumThumb);
				item->setData (QSize (last.Width_, last.Height_),
						CollectionRole::MediumThumb);
			}
			return item;
		}
	}

	void FotoBilderAccount::handleGotPhotos ()
	{
		QDomDocument document;
		const auto& content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (IsErrorReply (content))
			return;

		for (const auto& photo : ParseGetPicsRequest (document))
			AllPhotosItem_->appendRow (CreatePhotoItem (photo));
		emit doneUpdating ();
		CallNextFunctionFromQueue ();
	}

	void FotoBilderAccount::handleGalleryCreated ()
	{
		QDomDocument document;
		const auto& content = CreateDomDocumentFromReply (qobject_cast<QNetworkReply*> (sender ()),
				document);
		if (content.isEmpty ())
			return;

		if (IsErrorReply (content))
			return;

		auto galleries = document.elementsByTagName ("Gallery");
		Album album;
		for (int i = 0, size = galleries.count (); i < size; ++i)
		{
			const auto& elem = galleries.at (i).toElement ();
			const auto& albumFields = elem.childNodes ();
			for (int j = 0, count = albumFields.size (); j < count; ++j)
			{
				const auto& fieldElem = albumFields.at (j).toElement ();
				if (fieldElem.tagName () == "GalID")
					album.ID_ = fieldElem.text ().toUtf8 ();
				else if (fieldElem.tagName () == "GalName")
					album.Title_ = fieldElem.text ();
				else if (fieldElem.tagName () == "GalURL")
					album.Url_ = QUrl (fieldElem.text ());
			}
		}

		if (album.ID_.isEmpty ())
			return;

		auto item = new QStandardItem (album.Title_);
		item->setEditable (false);
		item->setData (ItemType::Collection, CollectionRole::Type);
		item->setData (album.ID_, CollectionRole::ID);

		CollectionsModel_->appendRow (item);

		Id2AlbumItem_ [album.ID_] = item;
		CallNextFunctionFromQueue ();
	}

	void FotoBilderAccount::handleUploadProgress (qint64 sent, qint64 total)
	{
		qDebug () << Q_FUNC_INFO << sent << total;
	}

	void FotoBilderAccount::handleImageUploaded ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		QDomDocument document;
		const QByteArray& content = CreateDomDocumentFromReply (reply, document);
		if (content.isEmpty ())
			return;

		if (IsErrorReply (content))
			return;

		const auto& item = Reply2UploadItem_.take (reply);

		auto pic = ParseUploadedPictureResponse (document);
		pic.Title_ = QFileInfo (item.FilePath_).fileName ();
		AllPhotosItem_->appendRow (CreatePhotoItem (pic));
		emit doneUpdating ();

		emit itemUploaded (item, pic.Url_);
		CallNextFunctionFromQueue ();
	}
}
}
}
