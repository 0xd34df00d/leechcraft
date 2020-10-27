/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarksettingsmanager.h"
#include <QCoreApplication>
#include <QQmlContext>
#include <QFileInfo>
#include <QtDebug>

namespace LC::SB2
{
	QuarkSettingsManager::QuarkSettingsManager (QUrl url, QQmlContext *ctx)
	: QuarkURL_ (std::move (url))
	, Ctx_ (ctx)
	{
		Util::BaseSettingsManager::Init ();

		Ctx_->setContextProperty (QFileInfo (QuarkURL_.path ()).baseName () + "_Settings", this);
	}

	void QuarkSettingsManager::setSettingsValue (const QString& key, const QVariant& value)
	{
		auto s = GetSettings ();
		s->setValue (key, value);

		PropertyChanged (key, value);
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

	namespace
	{
		bool TryDouble (QVariant& val)
		{
			bool ok = false;
			const auto& tempVal = val.toDouble (&ok);
			if (ok)
				val = tempVal;

			return ok;
		}

		bool TryInt (QVariant& val)
		{
			bool ok = false;
			const auto& tempVal = val.toInt (&ok);
			if (ok)
				val = tempVal;

			return ok;
		}
	}

	void QuarkSettingsManager::PropertyChanged (const QString& name, const QVariant& srcVal)
	{
		QVariant val = srcVal;
		if (val.type () == QVariant::String)
		{
			if (val == "true" || val == "false")
				val = val.toBool ();
			else
				TryDouble (val) || TryInt (val);
		}
		Ctx_->setContextProperty (name.toUtf8 ().constData (), val);
	}
}
