/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "networkaccessmanager.h"
#include <stdexcept>
#include <algorithm>
#include <QNetworkRequest>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QTimer>
#include <util/network/customcookiejar.h>
#include <util/network/networkdiskcache.h>
#include <util/xpc/defaulthookproxy.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "mainwindow.h"
#include "sslerrorshandler.h"

Q_DECLARE_METATYPE (QNetworkReply*);

using namespace LC;
using namespace LC::Util;

NetworkAccessManager::NetworkAccessManager (QObject *parent)
: QNetworkAccessManager (parent)
, CookieSaveTimer_ (new QTimer (this))
{
	connect (this,
			SIGNAL (sslErrors (QNetworkReply*, QList<QSslError>)),
			this,
			SLOT (handleSslErrors (QNetworkReply*, QList<QSslError>)));

	CookieJar_ = new CustomCookieJar (this);
	setCookieJar (CookieJar_);

	XmlSettingsManager::Instance ()->RegisterObject ("FilterTrackingCookies",
			this,
			"handleFilterTrackingCookies");
	XmlSettingsManager::Instance ()->RegisterObject ("DeleteCookiesOnExit",
			this,
			"saveCookies");
	XmlSettingsManager::Instance ()->RegisterObject ("EnableCookies",
			this,
			"setCookiesEnabled");
	XmlSettingsManager::Instance ()->RegisterObject ("MatchDomainExactly",
			this,
			"setMatchDomainExactly");
	XmlSettingsManager::Instance ()->RegisterObject ({ "CookiesWhitelist", "CookiesBlacklist" },
			this,
			"setCookiesLists");

	handleFilterTrackingCookies ();
	setCookiesEnabled ();
	setMatchDomainExactly ();
	setCookiesLists ();

	try
	{
		auto cache = new Util::NetworkDiskCache ("core", this);
		setCache (cache);

		XmlSettingsManager::Instance ()->RegisterObject ("CacheSize",
				this, "handleCacheSize");
		handleCacheSize ();
	}
	catch (const std::runtime_error& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< "so continuing without cache";
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (file.open (QIODevice::ReadOnly))
		CookieJar_->Load (file.readAll ());
	else
		qWarning () << Q_FUNC_INFO
			<< "could not open file"
			<< file.fileName ()
			<< file.errorString ();

	connect (CookieSaveTimer_,
			SIGNAL (timeout ()),
			this,
			SLOT (saveCookies ()));
	CookieSaveTimer_->start (10000);
}

NetworkAccessManager::~NetworkAccessManager ()
{
	saveCookies ();
}

QNetworkReply* NetworkAccessManager::createRequest (QNetworkAccessManager::Operation op,
		const QNetworkRequest& req, QIODevice *out)
{
	QNetworkRequest r = req;

	DefaultHookProxy_ptr proxy (new DefaultHookProxy);
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

void LC::NetworkAccessManager::handleSslErrors (QNetworkReply *replyObj,
		const QList<QSslError>& errors)
{
	new SslErrorsHandler { replyObj, errors };
}

void LC::NetworkAccessManager::saveCookies () const
{
	QDir dir = QDir::home ();
	dir.cd (".leechcraft");
	if (!dir.exists ("core") &&
			!dir.mkdir ("core"))
	{
		emit error (tr ("Could not create Core directory."));
		return;
	}

	QFile file (QDir::homePath () +
			"/.leechcraft/core/cookies.txt");
	if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
	{
		emit error (tr ("Could not save cookies, error opening cookie file."));
		qWarning () << Q_FUNC_INFO
			<< file.errorString ();
		return;
	}

	const bool saveEnabled = !XmlSettingsManager::Instance ()->
			property ("DeleteCookiesOnExit").toBool ();
	file.write (saveEnabled ? CookieJar_->Save () : QByteArray ());
}

void LC::NetworkAccessManager::handleFilterTrackingCookies ()
{
	CookieJar_->SetFilterTrackingCookies (XmlSettingsManager::Instance ()->
				property ("FilterTrackingCookies").toBool ());
}

void NetworkAccessManager::setCookiesEnabled ()
{
	CookieJar_->SetEnabled (XmlSettingsManager::Instance ()->
			property ("EnableCookies").toBool ());
}

void NetworkAccessManager::setMatchDomainExactly ()
{
	CookieJar_->SetExactDomainMatch (XmlSettingsManager::Instance ()->
			property ("MatchDomainExactly").toBool ());
}

namespace
{
	QList<QRegExp> GetList (const QByteArray& setting)
	{
		const auto& stringList = XmlSettingsManager::Instance ()->
				property (setting).toStringList ();
		QList<QRegExp> result;
		for (const auto& str : stringList)
			result << QRegExp (str);
		return result;
	}
}

void NetworkAccessManager::setCookiesLists ()
{
	CookieJar_->SetWhitelist (GetList ("CookiesWhitelist"));
	CookieJar_->SetBlacklist (GetList ("CookiesBlacklist"));
}

void NetworkAccessManager::handleCacheSize ()
{
	auto ourCache = qobject_cast<Util::NetworkDiskCache*> (cache ());
	ourCache->setMaximumCacheSize (XmlSettingsManager::Instance ()->
			property ("CacheSize").toInt () * 1048576);
}
