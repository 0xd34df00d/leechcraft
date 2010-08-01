/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "versioncomparator.h"
#include <QStringList>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			QString Numerize (QString version)
			{
				static const QStringList mods = QStringList () << "-rc"
						<< "-pre"
						<< "-beta"
						<< "-alpha";
				static QStringList replacements;
				int modsSize = mods.size ();
				if (replacements.isEmpty ())
					for (int i = 0; i < modsSize; ++i)
						replacements << QString (".%1.").arg (-i - 1);

				for (int i = 0; i < modsSize; ++i)
					version.replace (mods.at (i), replacements.at (i));

				return version;
			}

			bool IsVersionLess (const QString& leftVer, const QString& rightVer)
			{
				if (leftVer == rightVer)
					return false;

				QString leftNum = Numerize (leftVer);
				QString rightNum = Numerize (rightVer);

#ifdef VERSIONCOMPARATOR_DEBUG
				qDebug () << leftVer << "->" << leftNum
						<< rightVer << "->" << rightNum;
#endif

				QStringList leftParts = leftNum.split ('.',
						QString::SkipEmptyParts);
				QStringList rightParts = rightNum.split ('.',
						QString::SkipEmptyParts);

				int maxSize = std::max (leftParts.size (), rightParts.size ());
				for (int i = leftParts.size (); i < maxSize; ++i)
					leftParts << "0";
				for (int i = rightParts.size (); i < maxSize; ++i)
					rightParts << "0";

#ifdef VERSIONCOMPARATOR_DEBUG
				qDebug () << leftParts << rightParts;
#endif

				for (int i = 0; i < maxSize; ++i)
				{
					int left = leftParts.at (i).toInt ();
					int right = rightParts.at (i).toInt ();
#ifdef VERSIONCOMPARATOR_DEBUG
					qDebug () << left << right;
#endif
					if (left < right)
						return true;
					else if (left > right)
						return false;
				}

				return false;
			}
		}
	}
}
