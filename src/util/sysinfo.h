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

#ifndef UTIL_SYSINFO_H
#define UTIL_SYSINFO_H
#include "utilconfig.h"
#include <QPair>

class QString;

namespace LeechCraft
{
namespace Util
{
namespace SysInfo
{
	/** @brief Returns a string of OS name and version joined together.
	 *
	 * @return The name and version of OS running LeechCraft.
	 */
	UTIL_API QString GetOSName ();

	/** @brief Returns a pair of OS name and version.
	 *
	 * @return A pair consisting of operating system name and version.
	 */
	UTIL_API QPair<QString, QString> GetOSNameSplit ();
}
}
}

#endif
