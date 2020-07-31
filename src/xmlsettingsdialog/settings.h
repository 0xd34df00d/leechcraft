/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef XMLSETTINGSDIALOG_SETTINGS_H
#define XMLSETTINGSDIALOG_SETTINGS_H
#include <QObject>
#include <QSettings>

namespace LC
{
	class Settings : public QObject
	{
		Q_OBJECT

		QString ConfigDir_;
		QString ThemesConfigDir_;

		Q_PROPERTY (QString ConfigDir READ GetConfigDir)
		Q_PROPERTY (QString ThemesConfigDir READ GetThemesConfigDir)
	public:
		Settings ();
		const QString& GetConfigDir () const;
		const QString& GetThemesConfigDir () const;
	};
};

#endif

