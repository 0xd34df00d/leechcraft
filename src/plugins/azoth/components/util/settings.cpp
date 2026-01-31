/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settings.h"
#include <QCoreApplication>
#include <util/sll/scopeguards.h>
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	bool CheckWithDefaultValue (const QString& entryId, const QString& group, const QByteArray& propName)
	{
		QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Azoth" };

		settings.beginGroup (group);
		auto guard = Util::MakeEndGroupScopeGuard (settings);

		if (settings.value ("Enabled").toStringList ().contains (entryId))
			return true;
		if (settings.value ("Disabled").toStringList ().contains (entryId))
			return false;

		return XmlSettingsManager::Instance ().property (propName).toBool ();
	}

	void UpdateWithDefaultValue (bool value, const QString& entryId, const QString& group, const QByteArray& propName)
	{
		const bool defaultValue = XmlSettingsManager::Instance ().property (propName).toBool ();

		QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup (group);

		auto enabled = settings.value ("Enabled").toStringList ();
		auto disabled = settings.value ("Disabled").toStringList ();

		if (value == defaultValue)
		{
			enabled.removeAll (entryId);
			disabled.removeAll (entryId);

			settings.setValue ("Enabled", enabled);
			settings.setValue ("Disabled", disabled);
		}
		else if (value)
		{
			if (!enabled.contains (entryId))
			{
				enabled << entryId;
				settings.setValue ("Enabled", enabled);
			}
		}
		else
		{
			if (!disabled.contains (entryId))
			{
				disabled << entryId;
				settings.setValue ("Disabled", disabled);
			}
		}

		settings.endGroup ();
	}
}
