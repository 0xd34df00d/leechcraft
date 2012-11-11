/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "quarksettingsmanager.h"
#include <QCoreApplication>
#include <QDeclarativeContext>
#include <QFileInfo>

namespace LeechCraft
{
namespace SB2
{
	QuarkSettingsManager::QuarkSettingsManager (const QUrl& url, QDeclarativeContext *ctx)
	: QuarkURL_ (url)
	, Ctx_ (ctx)
	{
	}

	QSettings* QuarkSettingsManager::BeginSettings () const
	{
		auto settings = new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2_Quarks");
		settings->beginGroup (QFileInfo (QuarkURL_.path ()).fileName ());
		return settings;
	}

	void QuarkSettingsManager::EndSettings (QSettings *settings) const
	{
		settings->endGroup ();
	}
}
}
