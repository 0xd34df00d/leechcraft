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

#include "pls.h"
#include <algorithm>
#include <QSettings>

namespace LeechCraft
{
namespace LMP
{
namespace PLS
{
	QStringList Read (const QString& path)
	{
		QStringList result;

		QSettings settings (path, QSettings::IniFormat);
		settings.beginGroup ("playlist");

		const int numFiles = settings.value ("NumberOfEntries").toInt ();
		for (int i = 1; i <= numFiles; ++i)
		{
			const auto& str = settings.value ("File" + QString::number (i)).toString ();
			if (!str.isEmpty ())
				result << str;
		}

		settings.endGroup ();

		return result;
	}

	QList<Phonon::MediaSource> Read2Sources (const QString& path)
	{
		QList<Phonon::MediaSource> result;
		const auto& paths = Read (path);
		std::copy (paths.begin (), paths.end (), std::back_inserter (result));
		return result;
	}
}
}
}
