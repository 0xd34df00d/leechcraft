/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "settings.h"
#include <QApplication>
#include <QStringList>

using namespace LC;

Settings::Settings ()
{
#ifdef Q_OS_WIN32
	ConfigDir_ = QCoreApplication::applicationDirPath () + "/leechcraft/";
#elif defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
	ConfigDir_ = QCoreApplication::applicationDirPath () + "/../Resources/config";
#else
	ConfigDir_ = "/etc/leechcraft/";
#endif

	ThemesConfigDir_ = ConfigDir_ + "themes/";
}

const QString& Settings::GetConfigDir () const
{
	return ConfigDir_;
}

const QString& Settings::GetThemesConfigDir () const
{
	return ThemesConfigDir_;
}

