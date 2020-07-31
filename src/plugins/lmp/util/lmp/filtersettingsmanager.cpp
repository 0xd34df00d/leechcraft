/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "filtersettingsmanager.h"
#include <QCoreApplication>

namespace LC
{
namespace LMP
{
	FilterSettingsManager::FilterSettingsManager (const QString& filterId, QObject *parent)
	: BaseSettingsManager { false, parent }
	, FilterId_ { filterId }
	{
		BaseSettingsManager::Init ();
	}

	QSettings* FilterSettingsManager::BeginSettings () const
	{
		auto settings = new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_LMP_Effects");
		settings->beginGroup (FilterId_);
		return settings;
	}

	void FilterSettingsManager::EndSettings (QSettings *settings) const
	{
		settings->endGroup ();
	}
}
}
