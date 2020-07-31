/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settingsglobalhandler.h"
#include <QtDebug>
#include <qwebsettings.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	SettingsGlobalHandler::SettingsGlobalHandler (QObject *parent)
	: QObject { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ({
					"MaximumPagesInCache",
					"MinDeadCapacity",
					"MaxDeadCapacity",
					"TotalCapacity",
					"OfflineStorageQuota"
				},
				this, "cacheSettingsChanged");
		cacheSettingsChanged ();
	}

	void SettingsGlobalHandler::cacheSettingsChanged ()
	{
		auto& xsm = XmlSettingsManager::Instance ();
		QWebSettings::setMaximumPagesInCache (xsm.property ("MaximumPagesInCache").toInt ());

		auto megs = [&xsm] (const char *prop) { return xsm.property (prop).toDouble () * 1024 * 1024; };
		QWebSettings::setObjectCacheCapacities (megs ("MinDeadCapacity"),
				megs ("MaxDeadCapacity"),
				megs ("TotalCapacity"));

		QWebSettings::setOfflineStorageDefaultQuota (xsm.property ("OfflineStorageQuota").toInt () * 1024);
	}

	void SettingsGlobalHandler::handleSettingsClicked (const QString& name)
	{
		if (name == "ClearIconDatabase")
			QWebSettings::clearIconDatabase ();
		else if (name == "ClearMemoryCaches")
			QWebSettings::clearMemoryCaches ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown button"
					<< name;
	}
}
}
}
