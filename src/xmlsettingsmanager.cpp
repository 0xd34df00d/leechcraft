/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include <QSettings>
#include <QDynamicPropertyChangeEvent>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	XmlSettingsManager::XmlSettingsManager ()
	{
		Util::BaseSettingsManager::Init ();
	}

	XmlSettingsManager::~XmlSettingsManager ()
	{
	}

	XmlSettingsManager* XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager manager;
		return &manager;
	}

	QSettings* XmlSettingsManager::BeginSettings () const
	{
		QSettings *settings = new QSettings (Util::Proxy::Instance ()->GetOrganizationName (),
				Util::Proxy::Instance ()->GetApplicationName ());
		return settings;
	}

	void XmlSettingsManager::EndSettings (QSettings*) const
	{
	}
};

