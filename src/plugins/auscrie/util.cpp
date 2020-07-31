/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QSettings>
#include <QCoreApplication>
#include <util/sll/scopeguards.h>

namespace LC
{
namespace Auscrie
{
	void SaveFilterState (const FilterState& data)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Auscrie");
		settings.beginGroup ("Filter");
		settings.setValue ("PluginId", data.PluginId_);
		settings.setValue ("Variant", data.Variant_);
		settings.endGroup ();
	}

	FilterState RestoreFilterState ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Auscrie");
		const auto guard = Util::BeginGroup (settings, "Filter");

		return
		{
			settings.value ("PluginId").toByteArray (),
			settings.value ("Variant").toByteArray ()
		};
	}
}
}
