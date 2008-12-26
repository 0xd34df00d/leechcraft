/*
    Copyright (c) 2008 by Rudoy Georg <0xd34df00d@gmail.com>

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************
*/
#include "settings.h"
#include <QCoreApplication>

Settings::Settings ()
{
#ifdef Q_OS_WIN32
	ConfigDir_ = QCoreApplication::applicationDirPath () + "/leechcraft/";
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

