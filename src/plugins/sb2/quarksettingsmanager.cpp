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
#include <QtDebug>

namespace LeechCraft
{
namespace SB2
{
	QuarkSettingsManager::QuarkSettingsManager (const QUrl& url, QDeclarativeContext *ctx)
	: QuarkURL_ (url)
	, Ctx_ (ctx)
	{
		Util::BaseSettingsManager::Init ();
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

	void QuarkSettingsManager::PropertyChanged (const QString& name, const QVariant& srcVal)
	{
		QVariant val = srcVal;
		if (val.type () == QVariant::String)
		{
			if (val.canConvert<bool> ())
				val = val.toBool ();
			else if (val.canConvert<double> ())
				val = val.toDouble ();
			else if (val.canConvert<int> ())
				val = val.toInt ();
		}
		Ctx_->setContextProperty (name.toUtf8 ().constData (), val);
	}
}
}
