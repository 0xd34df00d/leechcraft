/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "flickraccount.h"
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDomDocument>
#include <QOAuth1>
#include <QOAuthHttpServerReplyHandler>
#include <QRestReply>
#include <QStandardItemModel>
#include <QUrlQuery>
#include <QUuid>
#include <QtDebug>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/domchildrenrange.h>
#include <util/sll/qtutil.h>
#include <util/sll/raiisignalconnection.h>
#include <util/threads/coro.h>
#include <util/xpc/util.h>
#include "flickrservice.h"

namespace LC
{
namespace Blasq
{
namespace Spegnersi
{
	const auto RequestTokenURL = "https://www.flickr.com/services/oauth/request_token"_qs;
	const auto UserAuthURL = "https://www.flickr.com/services/oauth/authorize"_qs;
	const auto AccessTokenURL = "https://www.flickr.com/services/oauth/access_token"_qs;

	namespace
	{
		class FlickrServerReplyHandler : public QOAuthHttpServerReplyHandler
		{
		public:
			using QOAuthHttpServerReplyHandler::QOAuthHttpServerReplyHandler;
		protected:
			void networkReplyFinished (QNetworkReply *reply) override
			{
				if (const auto& contentType = reply->header (QNetworkRequest::ContentTypeHeader).toString ();
					!contentType.startsWith (u"text/plain"_qsv))
					return QOAuthHttpServerReplyHandler::networkReplyFinished (reply);

				const QRestReply rest { reply };
				if (rest.hasError ())
					return emit tokenRequestErrorOccurred (QAbstractOAuth::Error::NetworkError, reply->errorString ());
				if (!rest.isHttpStatusSuccess ())
					return emit tokenRequestErrorOccurred (QAbstractOAuth::Error::ServerError, reply->errorString ());

				const auto& data = reply->readAll ();
				emit replyDataReceived (data);

				QVariantMap tokens;
				for (const auto& [name, value] : QUrlQuery { QString::fromUtf8 (data) }.queryItems (QUrl::FullyDecoded))
					tokens [name] = value;
				emit tokensReceived (tokens);
			}
		};
	}

	FlickrAccount::FlickrAccount (const QString& name,
			FlickrService *service, ICoreProxy_ptr proxy, const QByteArray& id)
	: QObject (service)
	, Name_ (name)
	, ID_ (id.isEmpty () ? QUuid::createUuid ().toByteArray () : id)
	, Service_ (service)
	, Proxy_ (proxy)
	, AuthMgr_ { new QOAuth1 { "08efe88f972b2b89bd35e42bb26f970e", "f70ac4b1ab7c499b", proxy->GetNetworkAccessManager (), this } }
	, CollectionsModel_ (new NamedModel<QStandardItemModel> (this))
	{
		auto handler = new FlickrServerReplyHandler { 12345, this };
		AuthMgr_->setReplyHandler (handler);
		AuthMgr_->setSignatureMethod (QOAuth1::SignatureMethod::Hmac_Sha1);
		AuthMgr_->setAuthorizationUrl (UserAuthURL);
		AuthMgr_->setTemporaryCredentialsUrl (RequestTokenURL);
		AuthMgr_->setTokenCredentialsUrl (AccessTokenURL);
		AuthMgr_->setContentType (QAbstractOAuth::ContentType::WwwFormUrlEncoded);

		connect (AuthMgr_,
				&QOAuth1::authorizeWithBrowser,
				this,
				[] (const QUrl& url)
				{
					const auto& e = Util::MakeEntity (url, {}, FromUserInitiated | OnlyHandle);
					if (!GetProxyHolder ()->GetEntityManager ()->HandleEntity (e))
						QDesktopServices::openUrl (url);
				});
		connect (AuthMgr_,
				&QOAuth1::granted,
				this,
				[this, handler]
				{
					handler->close ();
					emit accountChanged (this);
				});
	}

	QByteArray FlickrAccount::Serialize () const
	{
		QByteArray result;
		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << static_cast<quint8> (1)
					<< ID_
					<< Name_
					<< AuthMgr_->token ()
					<< AuthMgr_->tokenSecret ();
		}
		return result;
	}

	FlickrAccount* FlickrAccount::Deserialize (const QByteArray& ba, FlickrService *service, ICoreProxy_ptr proxy)
	{
		QDataStream istr (ba);

		quint8 version = 0;
		istr >> version;
		if (version != 1)
		{
			qWarning () << "unknown version" << version;
			return nullptr;
		}

		QByteArray id;
		QString name;
		QString authToken;
		QString authSecret;
		istr >> id
				>> name
				>> authToken
				>> authSecret;

		const auto acc = new FlickrAccount (name, service, proxy, id);
		acc->AuthMgr_->setTokenCredentials (authToken, authSecret);
		return acc;
	}

	QObject* FlickrAccount::GetQObject ()
	{
		return this;
	}

	IService* FlickrAccount::GetService () const
	{
		return Service_;
	}

	QString FlickrAccount::GetName () const
	{
		return Name_;
	}

	QByteArray FlickrAccount::GetID () const
	{
		return ID_;
	}

	QAbstractItemModel* FlickrAccount::GetCollectionsModel () const
	{
		return CollectionsModel_;
	}

	void FlickrAccount::UpdateCollections ()
	{
		CollectionsModel_->clear ();
		CollectionsModel_->setHorizontalHeaderLabels ({ tr ("Name") });

		AllPhotosItem_ = new QStandardItem (tr ("All photos"));
		AllPhotosItem_->setData (ItemType::AllPhotos, CollectionRole::Type);
		AllPhotosItem_->setEditable (false);
		CollectionsModel_->appendRow (AllPhotosItem_);

		UpdateCollectionsPage ({});
	}

	struct AuthGate
	{
		QOAuth1& AuthMgr_;

		Util::RaiiSignalConnection StatusConn_ {};
		Util::RaiiSignalConnection ErrorConn_ {};

		bool await_ready () const
		{
			if (AuthMgr_.status () == QAbstractOAuth::Status::Granted)
				return true;

			const auto& [token, secret] = AuthMgr_.tokenCredentials ();
			if (AuthMgr_.status () == QAbstractOAuth::Status::NotAuthenticated)
				return !token.isEmpty () && !secret.isEmpty ();

			return false;
		}

		void await_suspend (std::coroutine_handle<> handle)
		{
			StatusConn_ = QObject::connect (&AuthMgr_,
					&QOAuth1::statusChanged,
					[this, handle]
					{
						switch (AuthMgr_.status ())
						{
						case QAbstractOAuth::Status::Granted:
						case QAbstractOAuth::Status::NotAuthenticated:
							handle ();
							break;
						case QAbstractOAuth::Status::TemporaryCredentialsReceived:
						case QAbstractOAuth::Status::RefreshingToken:
							break;
						}
					});

			if (AuthMgr_.status () == QAbstractOAuth::Status::NotAuthenticated)
			{
				qDebug () << "requesting authentication";
				AuthMgr_.grant ();
			}
		}

		bool await_resume () const
		{
			return await_ready ();
		}
	};

	Util::ContextTask<> FlickrAccount::UpdateCollectionsPage (std::optional<int> page)
	{
		co_await Util::AddContextObject { *this };

		if (!co_await AuthGate { *AuthMgr_ })
		{
			qWarning () << "not authenticated";
			co_return;
		}

		const auto reply = AuthMgr_->get ({ "https://api.flickr.com/services/rest/"_qs }, {
					{ "method", "flickr.photos.search" },
					{ "user_id", "me" },
					{ "format", "rest" },
					{ "per_page", "500" },
					{ "page", QString::number (page.value_or (0)) },
					{ "extras", "url_o,url_z,url_m" }
				});
		const auto response = co_await *reply;
		const auto data = co_await Util::WithHandler (response.ToEither ("FlickrAccount::UpdateCollections"_qs),
				[] (const QString& error)
				{
					GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeNotification ("Blasq Spegnersi"_qs,
								tr ("Unable to refresh collections: %1.").arg (error),
								Priority::Critical));
				});
		HandleCollectionsReply (data);
	}

	void FlickrAccount::HandleCollectionsReply (const QByteArray& data)
	{
		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << "cannot parse" << data;
			return;
		}

		const auto& photos = doc
				.documentElement ()
				.firstChildElement ("photos"_qs);

		for (const auto& photo : Util::DomChildren (photos, "photo"_qs))
		{
			auto item = new QStandardItem;

			item->setText (photo.attribute ("title"_qs));
			item->setEditable (false);
			item->setData (ItemType::Image, CollectionRole::Type);
			item->setData (photo.attribute ("id"_qs), CollectionRole::ID);
			item->setData (photo.attribute ("title"_qs), CollectionRole::Name);

			auto getSize = [&photo] (char s)
			{
				return QSize
				{
					photo.attribute ("width_"_qs + s).toInt (),
					photo.attribute ("height_"_qs + s).toInt ()
				};
			};

			item->setData (QUrl (photo.attribute ("url_o"_qs)), CollectionRole::Original);
			item->setData (getSize ('o'), CollectionRole::OriginalSize);
			item->setData (QUrl (photo.attribute ("url_z"_qs)), CollectionRole::MediumThumb);
			item->setData (getSize ('z'), CollectionRole::MediumThumbSize);
			item->setData (QUrl (photo.attribute ("url_m"_qs)), CollectionRole::SmallThumb);
			item->setData (getSize ('m'), CollectionRole::SmallThumbSize);

			AllPhotosItem_->appendRow (item);
		}

		const auto thisPage = photos.attribute ("page"_qs).toInt ();
		if (thisPage != photos.attribute ("pages"_qs).toInt ())
			UpdateCollectionsPage (thisPage + 1);
		else
			emit doneUpdating ();
	}
}
}
}
