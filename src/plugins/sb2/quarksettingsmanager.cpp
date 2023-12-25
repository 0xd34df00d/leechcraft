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
#include <util/sll/qtutil.h>
#include <util/xsd/util.h>

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
		auto s = MakeSettings ();
		s->setValue (key, value);

		PropertyChanged (key, value);
	}

	auto QuarkSettingsManager::MakeSettings () const -> QSettings_ptr
	{
		return Util::MakeGroupSettings ("SB2_Quarks"_qs, QFileInfo (QuarkURL_.path ()).fileName ());
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
