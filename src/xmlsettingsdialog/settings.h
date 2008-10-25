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
#ifndef SETTINGS_H
#define SETTINGS_H
#include <QObject>
#include <QSettings>

class Settings : public QObject
{
	Q_OBJECT

	QString ConfigDir_;
	QString ThemesConfigDir_;

	Q_PROPERTY (QString ConfigDir READ GetConfigDir);
	Q_PROPERTY (QString ThemesConfigDir READ GetThemesConfigDir);
public:
	Settings ();
	const QString& GetConfigDir () const;
	const QString& GetThemesConfigDir () const;
};

#endif

