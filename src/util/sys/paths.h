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

#pragma once

#include <QStringList>
#include <util/utilconfig.h>

namespace LeechCraft
{
namespace Util
{
	enum class SysPath
	{
		QML,
		Share
	};

	UTIL_API QStringList GetPathCandidates (SysPath path, QString suffix);
	UTIL_API QString GetSysPath (SysPath path, const QString& suffix, const QString& filename);
}
}
