/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkaccessmanager.h"
#include <stdexcept>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <util/network/customcookiejar.h>
#include <util/network/networkdiskcache.h>
#include <util/sll/qtutil.h>
#include <util/xpc/defaulthookproxy.h>
#include "cookiesaver.h"
#include "sslerrorshandler.h"
#include "xmlsettingsmanager.h"

namespace LC
{
	namespace
	{
		QList<QRegularExpression> GetRxList (const QStringList& rxStrings)
		{
			QList<QRegularExpression> result;
			result.reserve (rxStrings.size ());
			for (const auto& str : rxStrings)
				result << QRegularExpression { str };
			return result;
		}
	}

	NetworkAccessManager::NetworkAccessManager (QObject *parent)
	: QNetworkAccessManager (parent)
	, CookieJar_ { std::make_unique<Util::CustomCookieJar> () }
	, CookieSaver_ { std::make_unique<CookieSaver> (*CookieJar_)}
	{
		setCookieJar (CookieJar_.get ());

		connect (this,
				&QNetworkAccessManager::sslErrors,
				[] (QNetworkReply *reply, const QList<QSslError>& errors) { new SslErrorsHandler { reply, errors }; });

		XmlSettingsManager::Instance ()->RegisterObject ("FilterTrackingCookies",
				CookieJar_.get (),
				&Util::CustomCookieJar::SetFilterTrackingCookies);
		XmlSettingsManager::Instance ()->RegisterObject ("EnableCookies",
				CookieJar_.get (),
				&Util::CustomCookieJar::SetEnabled);
		XmlSettingsManager::Instance ()->RegisterObject ("MatchDomainExactly",
				CookieJar_.get (),
				&Util::CustomCookieJar::SetExactDomainMatch);
		XmlSettingsManager::Instance ()->RegisterObject ("CookiesWhitelist",
				this,
				[this] (const QStringList& strs) { CookieJar_->SetWhitelist (GetRxList (strs)); });
		XmlSettingsManager::Instance ()->RegisterObject ("CookiesBlacklist",
				this,
				[this] (const QStringList& strs) { CookieJar_->SetBlacklist (GetRxList (strs)); });

		try
		{
			auto cache = new Util::NetworkDiskCache ("core"_qs, this);
			setCache (cache);

			XmlSettingsManager::Instance ()->RegisterObject ("CacheSize",
					this,
					[cache] (int megabytes) { cache->setMaximumCacheSize (megabytes * 1024 * 1024); });
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< "so continuing without cache";
		}
	}

	NetworkAccessManager::~NetworkAccessManager () = default;

	QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
			const QNetworkRequest& req, QIODevice *out)
	{
		QNetworkRequest r = req;

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		proxy->SetValue ("request", QVariant::fromValue<QNetworkRequest> (r));
		emit hookNAMCreateRequest (proxy, this, &op, &out);

		if (proxy->IsCancelled ())
		{
			const auto reply = proxy->GetReturnValue ().value<QNetworkReply*> ();
			emit requestCreated (op, r, reply);
			return reply;
		}

		proxy->FillValue ("request", r);

		if (XmlSettingsManager::Instance ()->property ("SetDNT").toBool ())
		{
			const bool dnt = XmlSettingsManager::Instance ()->property ("DNTValue").toBool ();
			r.setRawHeader ("DNT", dnt ? "1" : "0");
		}

		QNetworkReply *result = QNetworkAccessManager::createRequest (op, r, out);
		emit requestCreated (op, r, result);
		return result;
	}
}
