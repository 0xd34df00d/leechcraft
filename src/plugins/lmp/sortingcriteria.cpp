/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "sortingcriteria.h"
#include <QVariant>
#include <QtDebug>

namespace LeechCraft
{
namespace LMP
{
	QList<SortingCriteria> GetAllCriteria ()
	{
		return
		{
			SortingCriteria::Artist,
			SortingCriteria::Year,
			SortingCriteria::Album,
			SortingCriteria::TrackNumber,
			SortingCriteria::TrackTitle,
			SortingCriteria::DirectoryPath,
			SortingCriteria::FileName
		};
	}

	QVariant SaveCriteria (const QList<SortingCriteria>& criteria)
		{
			QVariantList result;
			for (const auto& crit : criteria)
				result << static_cast<quint8> (crit);
			return result;
		}

	QList<SortingCriteria> LoadCriteria (const QVariant& var)
	{
		QList<SortingCriteria> result;
		for (const auto& critVar : var.toList ())
		{
			const auto val = critVar.value<quint8> ();
			for (auto crit : GetAllCriteria ())
				if (static_cast<decltype (val)> (crit) == val)
				{
					result << crit;
					break;
				}
		}
		return result;
	}

	QString GetCriteriaName (SortingCriteria crit)
	{
		switch (crit)
		{
		case SortingCriteria::Artist:
			return QObject::tr ("Artist");
		case SortingCriteria::Year:
			return QObject::tr ("Year");
		case SortingCriteria::Album:
			return QObject::tr ("Album");
		case SortingCriteria::TrackNumber:
			return QObject::tr ("Track number");
		case SortingCriteria::TrackTitle:
			return QObject::tr ("Title");
		case SortingCriteria::DirectoryPath:
			return QObject::tr ("Directory");
		case SortingCriteria::FileName:
			return QObject::tr ("File name");
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown sorting criteria"
				<< static_cast<int> (crit);
		return QString ();
	}
}
}
