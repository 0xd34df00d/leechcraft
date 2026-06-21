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
	bool CheckWithDefaultValue (const GlobalStrongestId& id, const QString& group, const QByteArray& propName)
	{
		const auto& idString = id.ToString ();

		QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Azoth" };

		settings.beginGroup (group);
		auto guard = Util::MakeEndGroupScopeGuard (settings);

		if (settings.value ("Enabled").toStringList ().contains (idString))
			return true;
		if (settings.value ("Disabled").toStringList ().contains (idString))
			return false;

		return XmlSettingsManager::Instance ().property (propName).toBool ();
	}

	void UpdateWithDefaultValue (bool value, const GlobalStrongestId& id, const QString& group, const QByteArray& propName)
	{
		const auto& idString = id.ToString ();

		const bool defaultValue = XmlSettingsManager::Instance ().property (propName).toBool ();

		QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup (group);

		auto enabled = settings.value ("Enabled").toStringList ();
		auto disabled = settings.value ("Disabled").toStringList ();

		if (value == defaultValue)
		{
			enabled.removeAll (idString);
			disabled.removeAll (idString);

			settings.setValue ("Enabled", enabled);
			settings.setValue ("Disabled", disabled);
		}
		else if (value)
		{
			if (!enabled.contains (idString))
			{
				enabled << idString;
				settings.setValue ("Enabled", enabled);
			}
		}
		else
		{
			if (!disabled.contains (idString))
			{
				disabled << idString;
				settings.setValue ("Disabled", disabled);
			}
		}

		settings.endGroup ();
	}
}
