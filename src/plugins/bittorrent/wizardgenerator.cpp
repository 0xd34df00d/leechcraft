/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "wizardgenerator.h"
#include "xmlsettingsmanager.h"
#include "startupfirstpage.h"

namespace LC::BitTorrent
{
	QList<QWizardPage*> CreateStartupWizard (SessionSettingsManager *ssm)
	{
		QList<QWizardPage*> result;
		int version = XmlSettingsManager::Instance ()->Property ("StartupVersion", 0).toInt ();
		if (version < 1)
		{
			result << new StartupFirstPage (ssm);
			++version;
		}
		XmlSettingsManager::Instance ()->setProperty ("StartupVersion", version);
		return result;
	}
}
