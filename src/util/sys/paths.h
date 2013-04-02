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
	/** @brief Describes various root paths recognized by GetSysPath().
	 *
	 * @sa GetPathCandidates(), GetSysPath()
	 */
	enum class SysPath
	{
		/** @brief Root path for QML files.
		 *
		 * Plugins are expected to install their files into a
		 * subdirectory of this directory.
		 */
		QML,

		/** @brief Directory with shared data files.
		 *
		 * Corresponds to <code>/usr/[local/]share/leechcraft</code> on
		 * Linux, for example.
		 */
		Share
	};

	/** @brief Returns possible full paths for the path and subfolder.
	 *
	 * This function returns a list of paths formed as the given root
	 * path identified by \em path plus the \em subfolder in it. The
	 * paths in the returned list aren't checked for existence.
	 *
	 * For example, on Linux
	 * \code
	 * const auto& paths = GetPathCandidates (SysPath::Share, "flags");
	 * \endcode
	 * will return <code>{ "/usr/share/leechcraft/flags",
	 * "/usr/local/share/leechcraft/flags" }</code>.
	 *
	 * The \em subfolder can actually be a relative path, like
	 * <code>flags/countries</code>, not just a single subfolder name.
	 *
	 * This function hardly needs to be called from user code, consider
	 * using GetSysPath() instead.
	 *
	 * @param[in] path The root path.
	 * @param[in] subfolder The subfolder inside the root path.
	 * @return The list of possible paths to the subfolder in the
	 * root path identified by \em path.
	 *
	 * @sa GetSysPath()
	 */
	UTIL_API QStringList GetPathCandidates (SysPath path, QString subfolder);

	/** @brief Returns path to the file in the given root path and subfolder.
	 *
	 * This function returns path to a file named \em filename and
	 * located in the path specified by \em path + \em subfolder. It
	 * actually checks whether the file exists and if it doesn't, returns
	 * a null string instead.
	 *
	 * If LeechCraft is running on a system where multiple paths
	 * correspond to a single \em path, they are checked from more
	 * user-accessible to less user-accessible. For example,
	 * SysPath::Share corresponds to <code>/usr/local/share/leechcraft</code>
	 * and then <code>/usr/share/leechcraft</code>.
	 *
	 * The \em subfolder can actually be a relative path, like
	 * <code>flags/countries</code>, not just a single subfolder name.
	 *
	 * Refer to GetPathCandidates() for more information.
	 *
	 * @param[in] path The identifier of the root path.
	 * @param[in] subfolder The subfolder inside the \em path.
	 * @param[in] filename The filename inside the \em path +
	 * \em subfolder.
	 * @return Path to the \em filename located in \em path +
	 * \em subfolder, or an empty string if there is no such file.
	 *
	 * @sa GetPathCandidates()
	 */
	UTIL_API QString GetSysPath (SysPath path, const QString& subfolder, const QString& filename);
}
}
