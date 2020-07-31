/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingsinstancehandler.h"
#include <QFile>
#include <QUrl>
#include <QtDebug>
#include <qwebsettings.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	SettingsInstanceHandler::SettingsInstanceHandler (QWebSettings *settings, QObject *parent)
	: QObject { parent }
	, Settings_ { settings }
	{
		XmlSettingsManager::Instance ().RegisterObject ("DNSPrefetchEnabled",
				this, "cacheSettingsChanged");
		cacheSettingsChanged ();

		XmlSettingsManager::Instance ().RegisterObject ({
					"AllowJava",
					"OfflineStorageDB",
					"OfflineWebApplicationCache",
					"EnableNotifications",
					"DeveloperExtrasEnabled"
				},
				this, "generalSettingsChanged");
		generalSettingsChanged ();

		XmlSettingsManager::Instance ().RegisterObject ("UserStyleSheet",
				this, "setUserStyleSheet");
		setUserStyleSheet ();
	}

	void SettingsInstanceHandler::generalSettingsChanged ()
	{
		auto& xsm = XmlSettingsManager::Instance ();

		auto boolOpt = [this, &xsm] (QWebSettings::WebAttribute attr, const char *name)
		{
			Settings_->setAttribute (attr, xsm.property (name).toBool ());
		};
		boolOpt (QWebSettings::JavaEnabled, "AllowJava");
		boolOpt (QWebSettings::OfflineStorageDatabaseEnabled, "OfflineStorageDB");
		boolOpt (QWebSettings::OfflineWebApplicationCacheEnabled, "OfflineWebApplicationCache");
		boolOpt (QWebSettings::DeveloperExtrasEnabled, "DeveloperExtrasEnabled");
		boolOpt (QWebSettings::NotificationsEnabled, "EnableNotifications");
	}

	void SettingsInstanceHandler::cacheSettingsChanged ()
	{
		Settings_->setAttribute (QWebSettings::DnsPrefetchEnabled,
				XmlSettingsManager::Instance ().property ("DNSPrefetchEnabled").toBool ());
	}

	void SettingsInstanceHandler::setUserStyleSheet ()
	{
		const auto& pathStr = XmlSettingsManager::Instance ()
				.property ("UserStyleSheet").toString ();
		if (pathStr.isEmpty ())
		{
			Settings_->setUserStyleSheetUrl ({});
			return;
		}

		QFile file { pathStr };
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot open"
					<< pathStr
					<< file.errorString ();
			Settings_->setUserStyleSheetUrl ({});
			return;
		}

		const auto& contents = file.readAll ();

		const auto& uriContents = "data:text/css;charset=utf-8;base64," + contents.toBase64 ();
		Settings_->setUserStyleSheetUrl (QUrl::fromEncoded (uriContents));
	}
}
}
}
