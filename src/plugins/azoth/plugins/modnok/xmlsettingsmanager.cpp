/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlsettingsmanager.h"
#include <QCoreApplication>

namespace LC
{
namespace Azoth
{
namespace Modnok
{
	XmlSettingsManager::XmlSettingsManager ()
	: Util::BaseSettingsManager ()
	{
		Util::BaseSettingsManager::Init ();
	}

	XmlSettingsManager& XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager xsm;
		return xsm;
	}

	QSettings* XmlSettingsManager::BeginSettings () const
	{
		return new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () +
					"_Azoth_Modnok");
	}

	void XmlSettingsManager::EndSettings (QSettings*) const
	{
	}
}
}
}
