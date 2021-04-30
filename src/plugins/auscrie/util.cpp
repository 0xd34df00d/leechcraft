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
	namespace
	{
		const auto FilterGroup = QStringLiteral ("Filter");
		const auto PluginIdName = QStringLiteral ("PluginId");
		const auto VariantName = QStringLiteral ("Variant");
	}

	void SaveFilterState (const FilterState& data)
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Auscrie");
		const auto guard = Util::BeginGroup (settings, FilterGroup);
		settings.setValue (PluginIdName, data.PluginId_);
		settings.setValue (VariantName, data.Variant_);
	}

	FilterState RestoreFilterState ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Auscrie");
		const auto guard = Util::BeginGroup (settings, FilterGroup);
		return
		{
			settings.value (PluginIdName).toByteArray (),
			settings.value (VariantName).toByteArray ()
		};
	}
}
}
