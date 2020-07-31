/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlsettingsmanager.h"
#include <QCoreApplication>

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	XmlSettingsManager::XmlSettingsManager ()
	: Util::BaseSettingsManager (true)
	{
		Util::BaseSettingsManager::Init ();
	}

	XmlSettingsManager& XmlSettingsManager::Instance ()
	{
		static XmlSettingsManager xsm;
		return xsm;
	}

	void XmlSettingsManager::EndSettings (QSettings*) const
	{
	}

	QSettings *XmlSettingsManager::BeginSettings () const
	{
		return new QSettings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku_DCAC");
	}
}
}
}
