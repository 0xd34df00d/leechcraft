/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Aggregator
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"Aggregator">;

	bool ConfirmWithPersistence (const char *propName, const QString& questionMessage);
}
