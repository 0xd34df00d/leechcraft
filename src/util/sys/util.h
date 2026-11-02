/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "sysconfig.h"

class QString;

namespace LC::Util
{
	/** @brief Checks if the given filename has a known image extension.
	 *
	 * This function checks if the \em filename has an extension matching
	 * any image format known to QImageWriter.
	 *
	 * @param[in] filename The name of the file, either relative or absolute.
	 * @return Whether the \em filename has a known image suffix.
	 */
	UTIL_SYS_API bool HasSupportedImageExtension (const QString& filename);

	UTIL_SYS_API bool IsOSXLoadFromBundle ();

	/** @brief Makes the system send a signal to the calling process once
	 * its parent exits.
	 *
	 * On Linux, the parent is the thread that started the calling
	 * process, so the signal also arrives once just that thread exits.
	 * The processes the calling process starts don't inherit the
	 * setting, and executing a set-user-ID or set-group-ID program
	 * clears it, as does executing a program with file capabilities on
	 * Linux.
	 *
	 * This function is async-signal-safe, so a QProcess child process
	 * modifier may call it.
	 *
	 * @param[in] signum The signal to send, like SIGTERM.
	 * @return Whether the signal is set up, which only Linux and FreeBSD
	 * support.
	 */
	UTIL_SYS_API bool SetParentDeathSignal (int signum);
}
