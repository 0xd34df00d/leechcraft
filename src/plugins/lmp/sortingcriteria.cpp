/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sortingcriteria.h"
#include <QVariant>
#include <QtDebug>
#include <util/sll/unreachable.h>

namespace LC
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
		if (criteria.isEmpty ())
			return false;

		QVariantList result;
		for (const auto crit : criteria)
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
				if (static_cast<quint8> (crit) == val)
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

		Util::Unreachable ();
	}
}
}
