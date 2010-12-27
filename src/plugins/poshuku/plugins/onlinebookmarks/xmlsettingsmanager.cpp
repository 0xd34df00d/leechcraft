/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#include "xmlsettingsmanager.h"
#include <QCoreApplication>

using namespace LeechCraft::Plugins::Poshuku::Plugins::OnlineBookmarks;

XmlSettingsManager::XmlSettingsManager ()
{
	LeechCraft::Util::BaseSettingsManager::Init ();
}

XmlSettingsManager *XmlSettingsManager::Instance ()
{
	static XmlSettingsManager xsm;
	return &xsm;
}

void XmlSettingsManager::EndSettings (QSettings *settings) const
{
}

QSettings *XmlSettingsManager::BeginSettings () const
{
	return new QSettings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku_OnlineBookmarks");
}

