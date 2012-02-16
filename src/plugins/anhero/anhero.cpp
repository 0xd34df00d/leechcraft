/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "anhero.h"
#include <QIcon>
#include <QApplication>
#include <KDE/KGlobal>
#include <KDE/KCrash>
#include <kdeversion.h>
#include <util/util.h>

namespace LeechCraft
{
namespace AnHero
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("anhero");

		if (!QApplication::arguments ().contains ("-noanhero"))
		{
#if KDE_VERSION_MINOR < 6
			KCrash::setApplicationPath (qApp->applicationDirPath ());
#endif
			KCrash::setApplicationName ("LeechCraft");
#if KDE_VERSION_MINOR < 5
			KCrash::setCrashHandler (KCrash::defaultCrashHandler);
#else
			KCrash::setDrKonqiEnabled (true);;
#endif
		}
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AnHero";
	}

	QString Plugin::GetName () const
	{
		return "AnHero";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Crash handler");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/anhero.svg");
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_anhero, LeechCraft::AnHero::Plugin);
