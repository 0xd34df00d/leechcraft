/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlsettingsmanager.h"
#include <QSettings>
#include <QCoreApplication>

namespace LC::BitTorrent
{
	XmlSettingsManager::XmlSettingsManager ()
	{
		BaseSettingsManager::Init ();
	}

	XmlSettingsManager* XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager manager;
		return &manager;
	}

	QSettings* XmlSettingsManager::BeginSettings () const
	{
		return new QSettings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Torrent");
	}

	void XmlSettingsManager::EndSettings (QSettings*) const
	{
	}
}
