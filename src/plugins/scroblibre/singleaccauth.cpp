/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "singleaccauth.h"
#include <QDateTime>
#include <QCryptographicHash>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtDebug>
#include <QTimer>
#include <QSettings>
#include <QCoreApplication>
#include <util/xpc/passutils.h>
#include <util/xpc/util.h>
#include <util/sll/urloperator.h>
#include <interfaces/core/ientitymanager.h>
#include "util.h"

namespace LC
{
namespace Scroblibre
{
	SingleAccAuth::SingleAccAuth (const QUrl& url,
			const QString& login, ICoreProxy_ptr proxy, QObject *parent)
	: QObject { parent }
	, Proxy_ { proxy }
	, BaseURL_ { url }
	, Login_ { login }
	{
		LoadQueue ();
		reauth ();
	}

	namespace
	{
		QByteArray GetPostBody (const QString& sid, const SubmitInfo& info, int idx)
		{
			const auto& idStr = idx < 0 ?
					QByteArray {} :
					("[" + QByteArray::number (idx) + "]");

			QByteArray data;
			if (!sid.isEmpty ())
				data = "s=" + QUrl::toPercentEncoding (sid);

			auto append = [&data, &idStr] (const QByteArray& param, const QString& value) -> void
			{
				if (!data.isEmpty ())
					data += '&';
				data += param + idStr + '=' + QUrl::toPercentEncoding (value);
			};

			append ("a", info.Info_.Artist_);
			append ("b", info.Info_.Album_);
			append ("t", info.Info_.Title_);
			append ("i", QString::number (info.TS_.toSecsSinceEpoch ()));
			append ("l", QString::number (info.Info_.Length_));
			if (info.Info_.TrackNumber_)
				append ("n", QString::number (info.Info_.TrackNumber_));

			return data;
		}
	}

	void SingleAccAuth::SetNP (const SubmitInfo& info)
	{
		if (SID_.isEmpty ())
			return;

		const auto& data = GetPostBody (SID_, info, -1);

		QNetworkRequest req { NowPlayingUrl_ };
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		const auto reply = Proxy_->GetNetworkAccessManager ()->post (req, data);
		connect (reply,
				SIGNAL (finished ()),
				reply,
				SLOT (deleteLater ()));
	}

	void SingleAccAuth::Submit (const SubmitInfo& info)
	{
		if (SID_.isEmpty () || LastSubmit_.IsValid ())
		{
			Queue_ << info;
			SaveQueue ();
			return;
		}

		LastSubmit_ = info;

		const auto& data = GetPostBody (SID_, info, 0);

		QNetworkRequest req { SubmissionsUrl_ };
		req.setHeader (QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
		const auto reply = Proxy_->GetNetworkAccessManager ()->post (req, data);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleSubmission ()));
	}

	void SingleAccAuth::LoadQueue ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Scroblibre");
		settings.beginGroup ("Queues");
		settings.beginGroup (BaseURL_.toString ());
		settings.beginGroup (Login_);

		auto restore = [&settings] () -> SubmitInfo
		{
			const auto& artist = settings.value ("Artist").toString ();
			const auto& album = settings.value ("Album").toString ();
			const auto& title = settings.value ("Title").toString ();
			const auto& ts = settings.value ("TS").toDateTime ();
			const auto& length = settings.value ("Length").toInt ();
			const auto& track = settings.value ("Track").toInt ();

			return {
					{
						artist,
						album,
						title,
						{},
						length,
						0,
						track,
						{}
					},
					ts
				};
		};

		settings.beginGroup ("LastSubmit");
		const auto& last = restore ();
		if (last.IsValid ())
			Queue_ << last;
		settings.endGroup ();

		const auto size = settings.beginReadArray ("Items");
		for (auto i = 0; i < size; ++i)
		{
			settings.setArrayIndex (i);
			Queue_.append (restore ());
		}
		settings.endArray ();

		settings.endGroup ();
		settings.endGroup ();
		settings.endGroup ();
	}

	void SingleAccAuth::SaveQueue () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Scroblibre");
		settings.beginGroup ("Queues");
		settings.beginGroup (BaseURL_.toString ());
		settings.beginGroup (Login_);

		auto save = [&settings] (const SubmitInfo& info) -> void
		{
			settings.setValue ("Artist", info.Info_.Artist_);
			settings.setValue ("Album", info.Info_.Album_);
			settings.setValue ("Title", info.Info_.Title_);
			settings.setValue ("TS", info.TS_);
			settings.setValue ("Length", info.Info_.Length_);
			settings.setValue ("Track", info.Info_.TrackNumber_);
		};

		settings.remove ("LastSubmit");
		settings.beginGroup ("LastSubmit");
		if (LastSubmit_.IsValid ())
			save (LastSubmit_);
		settings.endGroup ();

		settings.beginWriteArray ("Items");
		for (auto i = 0; i < Queue_.size (); ++i)
		{
			settings.setArrayIndex (i);
			save (Queue_.at (i));
		}
		settings.endArray ();

		settings.endGroup ();
		settings.endGroup ();
		settings.endGroup ();
	}

	void SingleAccAuth::reauth (bool failed)
	{
		ReauthScheduled_ = false;

		SID_.clear ();

		const auto nowTime_t = QDateTime::currentSecsSinceEpoch ();
		const auto& nowStr = QString::number (nowTime_t);

		QUrl reqUrl { BaseURL_ };
		Util::UrlOperator { reqUrl }
				("hs", "true")
				("p", "1.2")
				("c", "tst")
				("v", "1.0")
				("u", Login_)
				("t", nowStr);

		const auto& service = UrlToService (BaseURL_);
		const auto& pass = Util::GetPassword ("org.LeechCraft.Scroblibre/" + service + '/' + Login_,
				tr ("Please enter password for account %1 on %2:")
					.arg ("<em>" + Login_ + "</em>")
					.arg (service),
				Proxy_,
				!failed);
		if (pass.isEmpty ())
			return;

		const auto& md5pass = QCryptographicHash::hash (pass.toUtf8 (),
				QCryptographicHash::Md5).toHex ();
		const auto& token = QCryptographicHash::hash (md5pass + nowStr.toUtf8 (),
				QCryptographicHash::Md5).toHex ();
		Util::UrlOperator { reqUrl } ("a", token);

		const auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (reqUrl));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleHSFinished ()));
	}

	void SingleAccAuth::rotateSubmitQueue ()
	{
		if (!Queue_.isEmpty ())
			Submit (Queue_.takeFirst ());
	}

	void SingleAccAuth::handleHSFinished ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = QString::fromLatin1 (reply->readAll ());
		auto split = data.split ('\n', Qt::SkipEmptyParts);
		for (auto& part : split)
			part = part.trimmed ();

		const auto& status = split.value (0);
		if (status == "OK")
		{
			SID_ = split.value (1);
			NowPlayingUrl_ = QUrl::fromEncoded (split.value (2).toLatin1 ());
			SubmissionsUrl_ = QUrl::fromEncoded (split.value (3).toLatin1 ());

			if (!Queue_.isEmpty ())
				QTimer::singleShot (0,
						this,
						SLOT (rotateSubmitQueue ()));

			return;
		}

		if (status == "BADTIME")
		{
			const auto& e = Util::MakeNotification ("Scroblibre",
					tr ("Your system clock is too incorrect."),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
		}
		else if (status.startsWith ("FAILED"))
		{
			const auto& e = Util::MakeNotification ("Scroblibre",
					tr ("Temporary server failure for %1, please try again later.")
						.arg (UrlToService (BaseURL_)),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
		}
		else if (status == "BANNED")
		{
			const auto& e = Util::MakeNotification ("Scroblibre",
					tr ("Sorry, the client is banned on %1.")
						.arg (UrlToService (BaseURL_)),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
		}
		else if (status == "BADAUTH")
		{
			const auto& e = Util::MakeNotification ("Scroblibre",
					tr ("Invalid authentication on %1.")
						.arg (UrlToService (BaseURL_)),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
			reauth (true);
			return;
		}
		else
		{
			const auto& e = Util::MakeNotification ("Scroblibre",
					tr ("General server error for %1.")
						.arg (UrlToService (BaseURL_)),
					Priority::Critical);
			Proxy_->GetEntityManager ()->HandleEntity (e);
		}

		if (ReauthScheduled_)
			return;

		QTimer::singleShot (120 * 1000,
				this,
				SLOT (reauth ()));
		ReauthScheduled_ = true;
	}

	void SingleAccAuth::handleSubmission ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		const auto& data = reply->readAll ();
		const auto& split = data.split ('\n');

		int timeout = 10 * 1000;
		if (split.value (0).trimmed () != "OK")
		{
			Queue_ << LastSubmit_;
			timeout *= 6;
		}
		else
			qDebug () << Q_FUNC_INFO
					<< "submitted to"
					<< BaseURL_
					<< Login_;

		LastSubmit_.Clear ();
		SaveQueue ();
		QTimer::singleShot (timeout,
				this,
				SLOT (rotateSubmitQueue ()));
	}
}
}
