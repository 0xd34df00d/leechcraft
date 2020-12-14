/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QStringList>
#include <QFileInfo>
#include "sysconfig.h"

class QDir;
class QUrl;

namespace LC::Util
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
	UTIL_SYS_API QStringList GetPathCandidates (SysPath path, QString subfolder);

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
	 * @sa GetSysPathUrl()
	 */
	UTIL_SYS_API QString GetSysPath (SysPath path, const QString& subfolder, const QString& filename);

	/** @brief Returns path to the file in the given root path and subfolder.
	 *
	 * This function behaves exactly like GetSysPath(), but it returns
	 * the path as QUrl via the <code>QUrl::fromLocalFile</code>, so it
	 * is suitable, for example, for QML views.
	 *
	 * @param[in] path The identifier of the root path.
	 * @param[in] subfolder The subfolder inside the \em path.
	 * @param[in] filename The filename inside the \em path +
	 * \em subfolder.
	 * @return Path to the \em filename located in \em path +
	 * \em subfolder, or an empty URL if there is no such file.
	 *
	 * @sa GetSysPath()
	 */
	UTIL_SYS_API QUrl GetSysPathUrl (SysPath path, const QString& subfolder, const QString& filename);

	/** @brief Returns the components of the system PATH variable.
	 *
	 * This function gets the PATH variable from the environment, parses
	 * it and returns the list of the components to the caller.
	 *
	 * @return System PATH components.
	 *
	 * @sa FindInSystemPath()
	 */
	UTIL_SYS_API QStringList GetSystemPaths ();

	/** @brief Searches for a file in system paths according to a filter.
	 *
	 * This function searches for a file named \em name in system paths
	 * passed in \em paths and returns the full path for the first file
	 * that matches \em filter, or an empty string if nothing is found.
	 *
	 * \em paths are most possibly obtained via GetSystemPaths(), but
	 * an arbitrary set of paths is fine too.
	 *
	 * @param[in] name The name of the file to search for.
	 * @param[in] paths The paths to search in.
	 * @param[in] filter The filter function for the candidates.
	 * @return The full path to the first found file or an empty string.
	 */
	UTIL_SYS_API QString FindInSystemPath (const QString& name, const QStringList& paths,
			const std::function<bool (QFileInfo)>& filter = {});

	/** @brief Describes various user-specific paths.
	 */
	enum class UserDir
	{
		/** @brief Cache for volatile data.
		 */
		Cache,

		/** @brief Root LeechCraft directory (something like ~/.leechcraft).
		 */
		LC
	};

	UTIL_SYS_API QDir GetUserDir (UserDir dir, const QString& subpath);

	/** @brief Creates a path if it doesn't exist.
	 *
	 * Creates a relative path ~/.leechcraft/path and throws an
	 * exception if this could not be done or if such path already
	 * exists and it is not readable.
	 *
	 * @param[in] path The path to create.
	 * @return The newly created dir.
	 * @exception std::runtime_error Throws if the path could not be
	 * created.
	 */
	UTIL_SYS_API QDir CreateIfNotExists (QString path);

	/** @brief Returns a temporary filename.
	 *
	 * This function returns a name of a temporary file that could
	 * be created, not creating the file itself.
	 *
	 * @param[in] pattern Pattern of the filename.
	 * @return The filename.
	 */
	UTIL_SYS_API QString GetTemporaryName (const QString& pattern = {});

	/** @brief Contains information about a partition's disk space.
	 */
	struct SpaceInfo
	{
		/** @brief The capacity of the partition.
		 */
		quint64 Capacity_;

		/** @brief How much free space there is.
		 *
		 * This is equal or less than Capacity_.
		 */
		quint64 Free_;

		/** @brief How much space is available to the current user.
		 *
		 * This is equal or less than Free_ and includes all the quotas
		 * and other limitations the user is subject to.
		 */
		quint64 Available_;
	};

	/** @brief Returns the disk space info of the partition containing
	 * \em path.
	 *
	 * @param[in] path The path on the partition for which the space
	 * information should be returned.
	 * @return The information about the disk space on the partition
	 * containing \em path.
	 */
	UTIL_SYS_API SpaceInfo GetSpaceInfo (const QString& path);
}
