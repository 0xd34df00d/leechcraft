/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>

class QVariant;
class QString;

namespace LC
{
namespace LMP
{
	enum class SortingCriteria
	{
		Artist,
		Year,
		Album,
		TrackNumber,
		TrackTitle,
		DirectoryPath,
		FileName
	};

	QList<SortingCriteria> GetAllCriteria ();

	QVariant SaveCriteria (const QList<SortingCriteria>&);
	QList<SortingCriteria> LoadCriteria (const QVariant&);

	QString GetCriteriaName (SortingCriteria);
}
}
