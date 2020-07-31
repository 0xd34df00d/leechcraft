/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>
#include <QFlags>
#include "lmputilconfig.h"

template<typename, typename>
class QMap;

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	enum SubstitutionFlag
	{
		SFNone,
		SFSafeFilesystem
	};
	Q_DECLARE_FLAGS (SubstitutionFlags, SubstitutionFlag);

	LMP_UTIL_API QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters ();

	LMP_UTIL_API QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters ();

	LMP_UTIL_API QStringList GetSubstGettersKeys ();

	LMP_UTIL_API QString PerformSubstitutions (QString mask,
			const MediaInfo& info, SubstitutionFlags flags = SFNone);

	LMP_UTIL_API QStringList PerformSubstitutions (const QString& mask,
			const QList<MediaInfo>& infos,
			const std::function<void (int, QString)>& setter,
			SubstitutionFlags flags = SFSafeFilesystem);
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::LMP::SubstitutionFlags)
